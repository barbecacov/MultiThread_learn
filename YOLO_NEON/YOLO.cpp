//
// Created by zd on 12/4/23.
//

#include "YOLO.h"
#include <chrono>
#include "arm_neon.h"

inline float sigmoid(float x) { return 1.0f / (1.0f + exp(-x)); }

inline void softmax_(const float* x, float* y, int length)
{
    float sum = 0;
    int i = 0;
    for (i = 0; i < length; i++)
    {
        y[i] = exp(x[i]);
        sum += y[i];
    }
    for (i = 0; i < length; i++)
    {
        y[i] /= sum;
    }
}

static inline float intersection_area(const Object& a, const Object& b)
{
    cv::Rect_<float> inter = a.rect & b.rect;
    return inter.area();
}

static void qsort_descent_inplace(std::vector<Object>& faceobjects, int left, int right)
{
    int i = left;
    int j = right;
    float p = faceobjects[(left + right) / 2].prob;

    while (i <= j)
    {
        while (faceobjects[i].prob > p)
            i++;

        while (faceobjects[j].prob < p)
            j--;

        if (i <= j)
        {
            // swap
            std::swap(faceobjects[i], faceobjects[j]);

            i++;
            j--;
        }
    }

#pragma omp parallel sections
    {
#pragma omp section
        {
            if (left < j) qsort_descent_inplace(faceobjects, left, j);
        }
#pragma omp section
        {
            if (i < right) qsort_descent_inplace(faceobjects, i, right);
        }
    }
}

static void qsort_descent_inplace(std::vector<Object>& objects)
{
    if (objects.empty())
        return;

    qsort_descent_inplace(objects, 0, objects.size() - 1);
}


static void nms_sorted_bboxes(const std::vector<Object>& faceobjects, std::vector<int>& picked, float nms_threshold)
{
    picked.clear();

    const int n = faceobjects.size();

    std::vector<float> areas(n);
    for (int i = 0; i < n; i++)
    {
        areas[i] = faceobjects[i].rect.area();
    }

    for (int i = 0; i < n; i++)
    {
        const Object& a = faceobjects[i];

        int keep = 1;
        for (int j = 0; j < (int)picked.size(); j++)
        {
            const Object& b = faceobjects[picked[j]];

            // intersection over union
            float inter_area = intersection_area(a, b);
            float union_area = areas[i] + areas[picked[j]] - inter_area;
            // float IoU = inter_area / union_area
            if (inter_area / union_area > nms_threshold)
                keep = 0;
        }

        if (keep)
            picked.push_back(i);
    }
}


static void generate_yolox_proposals(float stride, Mat& outs, float prob_threshold, std::vector<Object>& objects)
{

    const int num_class = outs.size[1] - 4;
    int feat_h = outs.size[2];
    int feat_w = outs.size[3];

    auto* feat_ptr = (float*)outs.data;

    for (int h = 0; h < feat_h; ++h) {
        for (int w = 0; w < feat_w; ++w) {

            int label = 0;
            float prob = feat_ptr[feat_h * feat_w * 4 + feat_w * h + w];

            for (int class_idx = 1; class_idx < num_class; ++class_idx) {
                float class_score = feat_ptr[feat_h * feat_w * (4 + class_idx) + feat_w * h + w];
                if (class_score > prob)
                {
                    prob = class_score;
                    label = class_idx;
                }
            }

            prob = sigmoid(prob);

            if ((prob > prob_threshold))
            {
                float left = ((float)w + 0.5f - feat_ptr[feat_w * h + w]) * stride;
                float top = ((float)h + 0.5f - feat_ptr[feat_w * feat_h + feat_w * h + w]) * stride;
                float right = ((float)w + 0.5f + feat_ptr[feat_w * feat_h * 2 + feat_w * h + w]) * stride;
                float bottom = ((float)h + 0.5f + feat_ptr[feat_w * feat_h * 3 + feat_w * h + w]) * stride;

                Object obj;
                obj.rect.x = left;
                obj.rect.y = top;
                obj.rect.width = right - left;
                obj.rect.height = bottom - top;
                obj.label = label;
                obj.prob = prob;
                objects.push_back(obj);
            }
        }
    }
}


static void generate_yolox_proposals2(float stride, float* out_reg, float* out_cls, float prob_threshold, std::vector<Object>& objects)
{

    const int num_class = 10;
    int feat_h = 512.f/stride;
    int feat_w = 640.f/stride;

    int length = feat_w / 4;

    unsigned int* vclsId;
    float32x4_t v0, v1, sv0;
    uint32x4_t vidx0, vmask, vidx1;
    float max_value = -100000.0f;
    float *vconf;
    int row_step = feat_w;
    int map_size = feat_h * row_step;
    float *cls_addr, *reg_addr;

    for (int h = 0; h < feat_h; ++h) {

        cls_addr = out_cls + h * row_step;
        reg_addr = out_reg + h * row_step;

        for (int l = 0; l < length; ++l) {

            v0 = vmovq_n_f32(max_value);
            vidx0 = vmovq_n_u32(0);

            for (int c = 0; c < num_class; ++c) {

                v1 = vld1q_f32(cls_addr + c * map_size);
                vmask = vcgtq_f32(v1, v0);
                vidx1 = vmovq_n_u32(c);
                vidx0 = vbslq_u32(vmask, vidx1, vidx0);
                v0 = vmaxq_f32(v1, v0);
            }

            vclsId = (unsigned int*)(&vidx0);

            sv0 = vsetq_lane_f32((cls_addr + 0)[vclsId[0] * map_size], sv0, 0);
            sv0 = vsetq_lane_f32((cls_addr + 1)[vclsId[1] * map_size], sv0, 1);
            sv0 = vsetq_lane_f32((cls_addr + 2)[vclsId[2] * map_size], sv0, 2);
            sv0 = vsetq_lane_f32((cls_addr + 3)[vclsId[3] * map_size], sv0, 3);

            vconf = (float*)(&sv0);

            for (int i = 0; i < 4; ++i) {

                float score_ = sigmoid(vconf[i]);

                if (score_ >= prob_threshold)
                {
                    float grid0 = (float)(l*4 + i) + 0.5f;
                    float grid1 = (float)h + 0.5f;

                    Object obj;

                    obj.prob = score_;
                    obj.label = vclsId[i];

                    float pred_ltrb[4];
                    float dfl_value[16];
                    float dfl_softmax[16];

                    for (int j = 0; j < 4; ++j) {

                        for (int k = 0; k < 16; ++k) {
                            dfl_value[k] = (reg_addr + i)[(j*16+k)*map_size];
                        }
                        softmax_(dfl_value, dfl_softmax, 16);
                        float dis = 0.f;
                        for (int k = 0; k < 16; ++k) {
                            dis += k * dfl_softmax[k];
                        }
                        pred_ltrb[j] = dis;

                    }

                    float left = (grid0 - pred_ltrb[0]) * stride;
                    float top =  (grid1 - pred_ltrb[1]) * stride;
                    float right = (grid0 + pred_ltrb[2]) * stride;
                    float bottom = (grid1 + pred_ltrb[3]) * stride;

                    obj.rect.x = left;
                    obj.rect.y = top;
                    obj.rect.height = bottom - top;
                    obj.rect.width = right - left;
                    objects.push_back(obj);
                }
            }
            cls_addr += 4;
            reg_addr += 4;
        }
    }

//    for (int h = 0; h < feat_h; ++h) {
//        for (int w = 0; w < feat_w; ++w) {
//
//            int label = 0;
//            float prob = out_cls[feat_w * h + w];
//
//            for (int class_idx = 1; class_idx < num_class; ++class_idx) {
//                float class_score = out_cls[feat_h * feat_w * class_idx + feat_w * h + w];
//                if (class_score > prob)
//                {
//                    prob = class_score;
//                    label = class_idx;
//                }
//            }
//            prob = sigmoid(prob);
//            if ((prob > prob_threshold))
//            {
//
//                float pred_ltrb[4];
//                float dfl_value[16];
//                float dfl_softmax[16];
//
//                for (int i = 0; i < 4; ++i) {
//
//                    for (int j = 0; j < 16; ++j) {
//
//                        dfl_value[j] = out_reg[(i*16+j)*feat_h*feat_w + h*feat_w + w];
//
//                    }
//                    softmax_(dfl_value, dfl_softmax, 16);
//
//                    float dis = 0.f;
//                    for (int j = 0; j < 16; ++j) {
//                        dis += j * dfl_softmax[j];
//                    }
//
//                    pred_ltrb[i] = dis;
//
//                }
//
//
//                float left = ((float)w + 0.5f - pred_ltrb[0]) * stride;
//                float top = ((float)h + 0.5f - pred_ltrb[1]) * stride;
//                float right = ((float)w + 0.5f + pred_ltrb[2]) * stride;
//                float bottom = ((float)h + 0.5f + pred_ltrb[3]) * stride;
//
//                Object obj;
//                obj.rect.x = left;
//                obj.rect.y = top;
//                obj.rect.width = right - left;
//                obj.rect.height = bottom - top;
//                obj.label = label;
//                obj.prob = prob;
//                objects.push_back(obj);
//            }
//        }
//    }
}


YOLO::YOLO(int img_w, int img_h, int net_w, int net_h, float confThreshold, float nmsThreshold) {

    this->confThreshold_ = confThreshold;
    this->nmsThreshold_ = nmsThreshold;

    this->net_w_ = net_w;
    this->net_h_ = net_h;
    this->img_w_ = img_w;
    this->img_h_ = img_h;

    this->keep_ratio = true;

    if (this->img_h_ != this->net_h_ || this->img_w_ != this->net_w_) {

        if (this->keep_ratio) {

            float h_ratio = (float) this->img_h_ / (float) this->net_h_;
            float w_ratio = (float) this->img_w_ / (float) this->net_w_;

            if (w_ratio < h_ratio) {
                this->ratio_w_ = h_ratio;
                this->ratio_h_ = h_ratio;

                this->pad_h = 0;
                this->pad_w = int(abs(this->net_w_ - (float) this->img_w_ / h_ratio) / 2);

            } else {
                this->ratio_w_ = w_ratio;
                this->ratio_h_ = w_ratio;
                this->pad_w = 0;
                this->pad_h = int(abs(this->net_h_ - (float) this->img_h_ / w_ratio) / 2);

            }
        } else {
            this->pad_h = 0;
            this->pad_w = 0;
            this->ratio_w_ = (float) img_w / (float) net_w;
            this->ratio_h_ = (float) img_h / (float) net_h;
        }
    }

    else{

        this->pad_h = 0;
        this->pad_w = 0;
        this->ratio_h_ = 1.0f;
        this->ratio_w_ = 1.0f;

    }

}

YOLO::~YOLO() {

}

void YOLO::postprecess(vector<float *> &out_ptr) {

    this->objects.clear();

    std::vector<Object> proposals;
    std::vector<Object> objects8;
    std::vector<Object> objects16;
    std::vector<Object> objects32;

    generate_yolox_proposals2(8.f, out_ptr[0], out_ptr[1], this->confThreshold_, objects8);

    generate_yolox_proposals2(16.f, out_ptr[2], out_ptr[3], this->confThreshold_, objects8);

    generate_yolox_proposals2(32.f, out_ptr[4], out_ptr[5], this->confThreshold_, objects8);

    proposals.insert(proposals.end(), objects8.begin(), objects8.end());
    proposals.insert(proposals.end(), objects16.begin(), objects16.end());
    proposals.insert(proposals.end(), objects32.begin(), objects32.end());

    qsort_descent_inplace(proposals);
    std::vector<int> picked;
    nms_sorted_bboxes(proposals, picked, this->nmsThreshold_);

    int count = picked.size();

    objects.resize(count);

    for (int i = 0; i < count; i++)
    {
        this->objects[i] = proposals[picked[i]];

        // adjust offset to original unpadded
        float x0 = objects[i].rect.x;
        float y0 = objects[i].rect.y;
        float w = objects[i].rect.width;   ///w
        float h = objects[i].rect.height;  ///h

        float x1 = x0 + w;
        float y1 = y0 + h;

        // clip
        x0 = std::max(std::min(x0, (float)(this->img_w_ - 1)), 0.f);
        y0 = std::max(std::min(y0, (float)(this->img_h_ - 1)), 0.f);
        x1 = std::max(std::min(x1, (float)(this->img_w_ - 1)), 0.f);
        y1 = std::max(std::min(y1, (float)(this->img_h_ - 1)), 0.f);

        this->objects[i].rect.x = x0;
        this->objects[i].rect.y = y0;
        this->objects[i].rect.width = x1 - x0;
        this->objects[i].rect.height = y1 - y0;
    }

}

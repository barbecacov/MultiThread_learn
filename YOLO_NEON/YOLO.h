//
// Created by zd on 12/4/23.
//

#ifndef AMBA_DEMO_YOLO_H
#define AMBA_DEMO_YOLO_H
#include "opencv2/opencv.hpp"
#include "common.h"
#include <vector>

using namespace cv;
using namespace std;



class YOLO {

public:
    YOLO(int img_w, int img_h, int net_w, int net_h, float confThreshold, float nmsThreshold);
    ~YOLO();

    void postprecess(vector<float*>& out_ptr);

    vector<Object> objects;

private:

    float confThreshold_;
    float nmsThreshold_;

    int net_w_;
    int net_h_;
    int img_w_;
    int img_h_;
    float ratio_h_;
    float ratio_w_;
    int pad_h;
    int pad_w;

    int top_k;

    bool keep_ratio;

};


#endif //AMBA_DEMO_YOLO_H

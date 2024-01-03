//
// Created by zd on 12/21/23.
//

#ifndef MY_CC_YOLO_HPP
#define MY_CC_YOLO_HPP

#include <vector>
#include <memory>
#include <iostream>
namespace yolo {


    struct Box {

        float left, top, right, bottom, confidence;
        int class_label;

        Box() = default;

        Box(float left, float top, float right, float bottom, float confidence, int class_label)
                : left(left),
                  top(top),
                  right(right),
                  bottom(bottom),
                  confidence(confidence),
                  class_label(class_label) {}

    };

    struct Image {

        const void *bgr_ptr = nullptr;
        int width = 0, height = 0;

        Image() = default;

        Image(const void *bgr_ptr, int width, int height) : bgr_ptr(bgr_ptr), width(width), height(height) {}

    };

    typedef std::vector<Box> BoxArray;

    class Infer{

    public:
        BoxArray forward(const Image &image)
        {
            std::vector<Box> objs;
            std::cout << "1111111111111111" << std::endl;
            return objs;
        }
    };

    std::shared_ptr<Infer> load(float confidence_threshold = 0.25f, float nms_threshold = 0.5f);

}

#endif //MY_CC_YOLO_HPP

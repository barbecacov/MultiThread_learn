#include <iostream>
#include <opencv2/opencv.hpp>
#include "cpm.hpp"
#include "yolo.hpp"

#include "ProducerConsumer.hpp"

using namespace std;
using namespace cv;

yolo::Image cvimg(const cv::Mat &image) { return yolo::Image(image.data, image.cols, image.rows); }



int main()
{

//    ProducerConsumer<int> producerConsumer(100, 6, 6);



    std::vector<cv::Mat> images{
            imread("/home/zd/CLionProjects/my_cc/image1.jpg"),
            imread("/home/zd/CLionProjects/my_cc/image2.jpg"),
            imread("/home/zd/CLionProjects/my_cc/image3.jpg")
    };

    std::vector<yolo::Image> yoloimages(images.size());

    std::transform(images.begin(), images.end(), yoloimages.begin(), cvimg);

    cpm::Instance<yolo::BoxArray, yolo::Image> cpmi;

    bool ok = cpmi.start(5);

    for (int i = 0; i < 3; ++i) {

        cpmi.commit(yoloimages[i]).get();

    }

    return 0;





}
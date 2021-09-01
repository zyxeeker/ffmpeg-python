//
// Created by zyx on 2021/8/23.
//
#include "library.h"

int main() {
    CaptureSDK *sdk;
    sdk = new CaptureSDK(1280, 480);
    sdk->InitDevice();
    sdk->CaptureFrame2Video();
    cv::waitKey(2);
    while (1) {
        cv::Mat dst = sdk->GetFrame();
        cv::imshow("test", dst);
        cv::waitKey(2);
    }
}
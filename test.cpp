//
// Created by zyx on 2021/8/17.
//

#include <gtest/gtest.h>
#include "library.h"

TEST(VIDEO_CAPTURE, MEMORY) {
    CaptureSDK *sdk = new CaptureSDK(640, 480);
    sdk->InitDevice();

    sdk->CaptureFrame2Video();
    cv::waitKey(20);
    while (1) {
        cv::Mat dst = sdk->GetFrame();
        cv::imshow("test", dst);
        cv::waitKey(1);
    }
}



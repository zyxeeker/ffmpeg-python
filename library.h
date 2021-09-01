#ifndef CAPTURESDK_LIBRARY_H
#define CAPTURESDK_LIBRARY_H

#include <opencv2/opencv.hpp>
#include <ctime>
#include <cstdlib>
#include <deque>
#include <stack>
#include <thread>
#include <mutex>
#include <iostream>

#ifdef __cplusplus
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavformat/version.h>
#include <libavutil/time.h>
#include <libavutil/mathematics.h>
}
#else
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavformat/version.h>
#include <libavutil/time.h>
#include <libavutil/mathematics.h>
#endif

class CaptureSDK {
public:
    CaptureSDK(int w, int h) {
        m_setting = std::to_string(w) + "x" + std::to_string(h);
    }

    ~CaptureSDK();

    bool InitDevice();

    bool SetExposure(int val);

    bool CaptureFrame2PNG(int filename);

    bool CaptureFrame2Video();

    cv::Mat GetFrame();

private:
    bool ReadFrame();

    bool EncodeFrame(AVPacket *p);

    void onDecodeRawPhase(cv::Mat &raw);

    void TransData(cv::Mat &m);

    void SplitDepthIRFrame(cv::Mat &merged, cv::Mat &depth);

private:
    int m_mk = 1;
    std::string m_setting;
    AVFrame m_frameDst;
    cv::Mat m_dst;
    cv::Mat m_depth;
    std::deque<AVPacket *> m_dq;
//    std::stack<AVPacket*> m_dq;
    std::thread *m_thread;
    std::mutex m_mutex;

    AVInputFormat *m_InputFormat;
    AVFrame *m_Frame;
    AVFormatContext *m_FormatCtx;
    AVPixelFormat m_Format;
    AVCodecContext *m_CaptureContext;
    AVCodec *m_Codec;
    AVPacket *m_Pkt;
    struct SwsContext *m_convert_ctx;
    int m_VideoStreamIndex;

//    UVCCameraLibrary *m_Lib;

};

#endif //CAPTURESDK_LIBRARY_H

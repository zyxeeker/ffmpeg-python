//
// Created by zyx on 2021/8/2.
//

#include "library.h"

#define LOG(x) std::cout<<x<<std::endl;
#define DLLEXPORT extern "C" __declspec(dllexport)


static void cvt(unsigned short *src, unsigned char *dst, int len) {
    for (int i = 0; i < len; ++i) {
        dst[i] = (src[i] & 0x3fc) >> 2;
    }
}

static void cvt2(unsigned short *src, unsigned char *dst, int len) {
    for (int i = 0; i < len; ++i) {
        dst[i] = (src[i] & 0x3f0) >> 4;
    }
}

CaptureSDK::~CaptureSDK() {
    if (m_FormatCtx != nullptr)
        avformat_free_context(m_FormatCtx);
}

void CaptureSDK::onDecodeRawPhase(cv::Mat &raw) {
    uint8_t emb[1280 * 2];
    uint8_t *p = (uint8_t *) raw.data;
    uint16_t *p2 = (uint16_t *) raw.data;
    for (int i = 0; i < 1280 * 962; i++) {
        p2[i] = (p2[i] & 0xfff) << 2 | (p2[i] >> 14);
    }

    if (p[0] == 0x29) {
        cvt((unsigned short *) p, emb, 1280 * 2);
    } else if (p[0] == 0xa5) {
        cvt2((unsigned short *) p, emb, 1280 * 2);
    } else {
        memcpy(emb, p, 1280 * 2);
    }
}

bool CaptureSDK::InitDevice() {
    avdevice_register_all();
    m_FormatCtx = avformat_alloc_context();

    m_InputFormat = av_find_input_format("dshow");
    AVDictionary *format_opts = nullptr;

    av_dict_set(&format_opts, "video_size", m_setting.c_str(), 0);

    m_FormatCtx = avformat_alloc_context();
    std::string str = "video=UVC DEPTH";
    int result = avformat_open_input(&m_FormatCtx, str.c_str(), m_InputFormat, &format_opts);
    if (result < 0) {
        std::cout << "Device[UVC DEPTH] Not Found!";
        return false;
    }
    result = avformat_find_stream_info(m_FormatCtx, nullptr);
    if (result < 0) {
        return false;
    }
    int count = m_FormatCtx->nb_streams;
    for (int i = 0; i < count; ++i) {
        if (m_FormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_VideoStreamIndex = i;
            break;
        }
    }
    if (m_VideoStreamIndex < 0)
        return false;

    m_CaptureContext = m_FormatCtx->streams[m_VideoStreamIndex]->codec;
    m_Codec = avcodec_find_decoder(m_CaptureContext->codec_id);
    if (m_Codec == nullptr)
        return false;

    if (avcodec_open2(m_CaptureContext, m_Codec, nullptr) != 0)
        return false;

    m_Format = m_CaptureContext->pix_fmt;
    return true;
}

bool CaptureSDK::ReadFrame() {
    m_Pkt = (AVPacket *) av_malloc(sizeof(AVPacket));
    if (av_read_frame(m_FormatCtx, m_Pkt)) {
        return false;
    }
    m_mutex.lock();
    m_dq.push_back(m_Pkt);
    m_mutex.unlock();
//    av_packet_unref(m_Pkt);
//    av_packet_free()
    return true;
}

bool CaptureSDK::EncodeFrame(AVPacket *p) {
    m_Frame = av_frame_alloc();
    if (p->stream_index != m_VideoStreamIndex) {
        av_packet_unref(p);
        return false;
    }
    if (avcodec_send_packet(m_CaptureContext, p)) {
        av_packet_unref(p);
        return false;
    }
    if (avcodec_receive_frame(m_CaptureContext, m_Frame)) {
        av_packet_unref(p);
        return false;
    }
//    av_packet_free(&p);
    return true;
}

void CaptureSDK::TransData(cv::Mat &m) {
    int w = m_Frame->width, h = m_Frame->height;
    memset(&m_frameDst, 0, sizeof(m_frameDst));
    m = cv::Mat(h, w, CV_16UC1);
    m_frameDst.data[0] = (uint8_t *) m.data;
    AVPixelFormat dstPixel = AV_PIX_FMT_YUYV422;
    avpicture_fill((AVPicture *) &m_frameDst, m_frameDst.data[0], dstPixel, w, h);

    m_convert_ctx = sws_getContext(w, h, m_Format, w, h, dstPixel,
                                   SWS_FAST_BILINEAR, NULL, NULL, NULL);
    sws_scale(m_convert_ctx, m_Frame->data, m_Frame->linesize, 0, h,
              m_frameDst.data, m_frameDst.linesize);
    av_frame_free(&m_Frame);
}

bool CaptureSDK::CaptureFrame2PNG(int filename) {
    if (!ReadFrame()) return false;

    TransData(m_dst);

    onDecodeRawPhase(m_dst);

    cv::imwrite("./" + std::to_string(filename) + ".png", m_dst);
    return true;
}

bool CaptureSDK::CaptureFrame2Video() {
    m_thread = new std::thread([=]() {
        while (1)
            if (!ReadFrame()) return false;
    });
    m_thread->detach();
    return true;
}

bool CaptureSDK::SetExposure(int val) {
//    m_Lib = new UVCCameraLibrary;
//    m_Lib->connectDevice("UVC DEPTH");
//    m_Lib->setGain(val);
    return true;
}

void CaptureSDK::SplitDepthIRFrame(cv::Mat &merged, cv::Mat &depth) {
    depth = cv::Mat(480, 640, CV_16UC1, merged.ptr(0));
}

cv::Mat CaptureSDK::GetFrame() {
    AVPacket *tmp = nullptr;
    if (!m_dq.empty()) {
        tmp = m_dq.front();
        EncodeFrame(tmp);
        av_packet_unref(tmp);
        av_packet_free(&tmp);
        m_dq.pop_front();
//        m_dq.shrink_to_fit();
        TransData(m_dst);
        SplitDepthIRFrame(m_dst, m_depth);
    }
    return m_depth;
}

#if DEBUG

#else
CaptureSDK *sdk;

DLLEXPORT void init(int w, int h) {
    sdk = new CaptureSDK(w, h);
    sdk->InitDevice();
}

DLLEXPORT void raw2PNG(int filename) {
    sdk->CaptureFrame2PNG(filename);
}

DLLEXPORT void setExposure(int val) {
    sdk->SetExposure(val);
}

DLLEXPORT void runCapture() {
    sdk->CaptureFrame2Video();
    cv::waitKey(1);
}

DLLEXPORT uchar *getVideoFrame() {
    return sdk->GetFrame().data;
}

DLLEXPORT void release(uchar *data) {
    free(data);
}

#endif

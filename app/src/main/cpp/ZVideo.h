//
// Created by 张立 on 5/8/21.
//

#ifndef FFMPEGMUSICPLAYER_ZVIDEO_H
#define FFMPEGMUSICPLAYER_ZVIDEO_H


#include "ZCallJava.h"
#include "ZPlaystatus.h"
#include "ZQueen.h"
#include "ZAudio.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/time.h"
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
};

class ZVideo {

public:
    int streamIndex;
    ZCallJava *callJava = NULL;
    ZPlaystatus *playstatus = NULL;
    ZQueen *queen = NULL;
    pthread_mutex_t codecMutex;
    pthread_t threadPlay;

    AVCodecContext *codecContext = NULL;
    AVCodecParameters *parameters = NULL;
    //-------------------同步代码-------------------
    ZAudio *audio = NULL;
    double delayTime = 0;//实时计算 音视频差值
    double default_time = 0.04;
    AVRational time_base;
    double clock = 0;//视频时间
public:
    ZVideo(ZCallJava *pJava, ZPlaystatus *pPlaystatus);

    ~ZVideo();

    void play();

    double getDelayTime(double diff);
    double getFrameDiffTime(AVFrame *avFrame);
};


#endif //FFMPEGMUSICPLAYER_ZVIDEO_H

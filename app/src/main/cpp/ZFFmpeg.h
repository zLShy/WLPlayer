//
// Created by 张立 on 4/29/21.
//

#ifndef FFMPEGMUSICPLAYER_ZFFMPEG_H
#define FFMPEGMUSICPLAYER_ZFFMPEG_H


#include <sys/types.h>
#include "ZCallJava.h"
#include "ZPlaystatus.h"
#include "ZAudio.h"
#include "ZVideo.h"
#include <pthread.h>

extern "C" {
#include <libavformat/avformat.h>
};

class ZFFmpeg {
public:
    ZCallJava *callJava = NULL;
    ZPlaystatus *playstatus = NULL;
    const char *url = NULL;
    pthread_t decodeThread;
    ZAudio *audio = NULL;
    ZVideo *video = NULL;
    AVFormatContext *pFormatContext = NULL;

    int duration = 0;
    pthread_mutex_t seek_mutex;
    pthread_mutex_t init_mutex;
    bool exit = false;

public:
    ZFFmpeg(ZCallJava *pJava, ZPlaystatus *pPlaystatus, const char *string);

    ~ZFFmpeg();

    void parpared();

    void decodeFFmpegThread();

    void start();

    void selectChannel(int type);

    void seekTo(int position);

    void setVolume(int percent);

    void setSpeed(int type, float speed);

    int getCodecContext(AVCodecParameters *codecpar, AVCodecContext **avCodecContext);

    void release();
};


#endif //FFMPEGMUSICPLAYER_ZFFMPEG_H

//
// Created by 张立 on 4/29/21.
//

#ifndef FFMPEGMUSICPLAYER_ZQUEEN_H
#define FFMPEGMUSICPLAYER_ZQUEEN_H

#include "queue"
#include "pthread.h"
#include "ZLog.h"
#include "ZPlaystatus.h"

extern "C"
{
#include "libavcodec/avcodec.h"
};

class ZQueen {
public:
    std::queue<AVPacket *> queuePacket;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;
    ZPlaystatus *playstatus = NULL;

public:
    ZQueen(ZPlaystatus *playstatus);
    ~ZQueen();

    int putAvpacket(AVPacket *packet);
    int getAvpacket(AVPacket *packet);

    int getQueueSize();

    void clearAvpacket();
};


#endif //FFMPEGMUSICPLAYER_ZQUEEN_H

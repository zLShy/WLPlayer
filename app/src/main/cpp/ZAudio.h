//
// Created by 张立 on 4/29/21.
//

#ifndef FFMPEGMUSICPLAYER_ZAUDIO_H
#define FFMPEGMUSICPLAYER_ZAUDIO_H

#include "ZCallJava.h"
#include "ZPlaystatus.h"
#include "ZQueen.h"
#include "SoundTouch.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include <libswresample/swresample.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
};
using namespace soundtouch;

class ZAudio {

public:
    int streamIndex = -1;
    ZPlaystatus *playstatus = NULL;
    ZCallJava *callJava = NULL;
    AVCodecContext *codecContext = NULL;
    AVFrame *avFrame = NULL;
    AVPacket *avPacket = NULL;
    AVCodecParameters *parameters = NULL;
    AVPacket *packet = NULL;
    ZQueen *queen = NULL;
    int ret = 0;

    int duration = 0;
    int data_size = 0;
    int sampleRate = 0;
    uint8_t *buffer = NULL;
    float speed = 1.0f;
    float pitch = 1.0f;

    double now_time;//当前frame时间
    double clock;//当前播放的时间    准确时间
    double last_tiem; //上一次调用时间


    //时间单位         总时间/帧数   单位时间     *   时间戳= pts  * 总时间/帧数
    AVRational time_base;
    pthread_t thread_play;

    //SLES环境 相关信息
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pcmPlayerPlay = NULL;//播放器接口
    SLMuteSoloItf pcmMutePlay = NULL;//声道接口
    SLVolumeItf pcmVolumePlay = NULL;//声音接口
    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;

    //混音器
    SLObjectItf outputMixObject = NULL;//用SLObjectItf创建混音器接口对象
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;//创建具体的混音器对象实例
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;


    SoundTouch *soundTouch = NULL;
    uint8_t *out_buffer = NULL;//改变声音入参出参
//    波处理完了没
    bool finished = true;
    SAMPLETYPE *sampleBuffer = NULL;//新的缓冲区
//    新波的实际个数
    int nb = 0;
    int num = 0;

public:
    ZAudio(ZCallJava *callback, ZPlaystatus *playstatus, int samplerate);

    ~ZAudio();

    void play();

    void initOpenSLES();

    int getCurrentSampleRateForOpensles(int sample_rate);

    int resampleAudio(void **pcmbuf);

    void selectChannel(int type);

    void setVolume(int percent);

    void setSpeed(int type, float speed);

    int getSoundTouchData();

};


#endif //FFMPEGMUSICPLAYER_ZAUDIO_H

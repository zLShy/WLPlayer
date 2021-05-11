//
// Created by 张立 on 4/29/21.
//

#ifndef FFMPEGMUSICPLAYER_ZCALLJAVA_H
#define FFMPEGMUSICPLAYER_ZCALLJAVA_H

#include "jni.h"
#include <linux/stddef.h>
#include "ZLog.h"

#define MAIN_THREAD 0
#define CHILD_THREAD 1

class ZCallJava {
public:
    _JavaVM *javaVM = NULL;
    JNIEnv *jniEnv = NULL;
    jobject jobj;

    jmethodID jmid_parpared;
    jmethodID jmid_timeinfo;

    jmethodID jmid_load;
    jmethodID jmid_renderyuv;

public:
    ZCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj);

    ~ZCallJava();

    void onCallParpared(int type);

    void onCallTimeInfo(int type, int curr, int total);

    void onLoad(int i, bool b);

    void onCallRenderYUV(int width, int height, uint8_t *fy, uint8_t *fu, uint8_t *fv);

};


#endif //FFMPEGMUSICPLAYER_ZCALLJAVA_H

#include <jni.h>
#include <string>
#include "ZLog.h"
#include "ZCallJava.h"
#include "ZFFmpeg.h"
#include "ZPlaystatus.h"

extern "C"
{
#include <libavformat/avformat.h>
}

_JavaVM *javaVM = NULL;
ZCallJava *callJava = NULL;
ZFFmpeg *fFmpeg = NULL;
ZPlaystatus *playstatus = NULL;


extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    jint result = -1;
    javaVM = vm;
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {

        return result;
    }
    return JNI_VERSION_1_4;

}

extern "C"
JNIEXPORT void JNICALL
Java_com_zl_ffmpegmusicplayer_player_ZPlayer_native_1parpared(JNIEnv *env, jobject thiz,
                                                              jstring source_) {

    const char *source = env->GetStringUTFChars(source_, 0);
    if (fFmpeg == NULL) {
        if (callJava == NULL) {
            callJava = new ZCallJava(javaVM, env, &thiz);
        }
        playstatus = new ZPlaystatus();

        fFmpeg = new ZFFmpeg(callJava, playstatus, source);
        fFmpeg->parpared();
    }

    env->ReleaseStringUTFChars(source_, source);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_zl_ffmpegmusicplayer_player_ZPlayer_native_1start(JNIEnv *env, jobject thiz) {

    if (fFmpeg != NULL) {
        fFmpeg->start();
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_zl_ffmpegmusicplayer_player_ZPlayer_native_1channel_1select(JNIEnv *env, jobject thiz,
                                                                     jint type) {
    if (fFmpeg != NULL) {
        fFmpeg->selectChannel(type);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_zl_ffmpegmusicplayer_player_ZPlayer_native_1seek(JNIEnv *env, jobject thiz, jint seconds) {
    if (fFmpeg != NULL) {
        fFmpeg->seekTo(seconds);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_zl_ffmpegmusicplayer_player_ZPlayer_native_1changeVol(JNIEnv *env, jobject thiz,
                                                               jint percent) {

    if (fFmpeg != NULL) {
        fFmpeg->setVolume(percent);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_zl_ffmpegmusicplayer_player_ZPlayer_native_1changeTone(JNIEnv *env, jobject thiz,
                                                                jint type, jfloat speed) {

    if (fFmpeg != NULL) {
        fFmpeg->setSpeed(type, speed);
    }
}
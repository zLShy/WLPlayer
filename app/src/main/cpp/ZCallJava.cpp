//
// Created by 张立 on 4/29/21.
//

#include "ZCallJava.h"

ZCallJava::ZCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj) {

    this->javaVM = javaVM;
    this->jniEnv = env;
    this->jobj = *obj;
    this->jobj = env->NewGlobalRef(jobj);

    jclass jclzz = env->GetObjectClass(jobj);
    if (!jclzz) {
        LOGE("get class error");
        return;
    }

    jmid_parpared = env->GetMethodID(jclzz, "onCallParpared", "()V");
    jmid_timeinfo = env->GetMethodID(jclzz, "onCallTimeInfo", "(II)V");
}

ZCallJava::~ZCallJava() {

}

void ZCallJava::onCallParpared(int type) {
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_parpared);
    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {

            LOGE("get child thread jnienv worng");

            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_parpared);
        javaVM->DetachCurrentThread();
    }
}

void ZCallJava::onCallTimeInfo(int type, int curr, int total) {
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, curr, total);
    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            LOGE("call onCallTimeInfo worng");
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, curr, total);
        javaVM->DetachCurrentThread();
    }
}

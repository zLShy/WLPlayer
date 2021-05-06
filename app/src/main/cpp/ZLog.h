//
// Created by 张立 on 4/29/21.
//

#ifndef FFMPEGMUSICPLAYER_ZLOG_H
#define FFMPEGMUSICPLAYER_ZLOG_H
#include <android/log.h>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"FFmpeg",__VA_ARGS__)
#endif //FFMPEGMUSICPLAYER_ZLOG_H

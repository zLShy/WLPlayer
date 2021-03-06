//
// Created by 张立 on 4/29/21.
//

#include "ZFFmpeg.h"

ZFFmpeg::ZFFmpeg(ZCallJava *pJava, ZPlaystatus *pPlaystatus, const char *source) {

    this->callJava = pJava;
    this->playstatus = pPlaystatus;
    this->url = source;
    pthread_mutex_init(&seek_mutex, NULL);
    pthread_mutex_init(&init_mutex, NULL);

}

ZFFmpeg::~ZFFmpeg() {

    pthread_mutex_destroy(&seek_mutex);
    pthread_mutex_destroy(&init_mutex);
}

void *decodeFFmpeg(void *data) {
    //将对象转为ZFFmpeg 然后点用C++函数 将C环境切换到c++环境
    ZFFmpeg *zfFmpeg = (ZFFmpeg *) data;
    zfFmpeg->decodeFFmpegThread();
    pthread_exit(&zfFmpeg->decodeThread);
}

void ZFFmpeg::parpared() {
    //创建子线程
    pthread_create(&decodeThread, NULL, decodeFFmpeg, this);
}

//初始化FFmpeg函数 线程运行
void ZFFmpeg::decodeFFmpegThread() {
    pthread_mutex_lock(&init_mutex);
    av_register_all();
    avformat_network_init();

    pFormatContext = avformat_alloc_context();//初始化上下文 ---解码器最开始的上下文
    if (avformat_open_input(&pFormatContext, url, NULL, NULL) != 0) {
        LOGE("打开链接失败");
        return;
    }

    if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
        LOGE("寻找流失败");
        return;
    }

    for (int i = 0; i < pFormatContext->nb_streams; ++i) {
        if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio == NULL) {
                audio = new ZAudio(callJava, playstatus,
                                   pFormatContext->streams[i]->codecpar->sample_rate);
                audio->streamIndex = i;
                audio->duration = pFormatContext->streams[i]->duration / AV_TIME_BASE;
                audio->parameters = pFormatContext->streams[i]->codecpar;
                audio->time_base = pFormatContext->streams[i]->time_base;
                duration = audio->duration;
            }
        } else if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (video == NULL) {
                video = new ZVideo(callJava, playstatus);
                video->streamIndex = i;
                video->parameters = pFormatContext->streams[i]->codecpar;

                video->time_base = pFormatContext->streams[i]->time_base;
                int num = pFormatContext->streams[i]->r_frame_rate.num;//分子
                int den = pFormatContext->streams[i]->r_frame_rate.den;//分母
                if (num != 0 && den != 0) {
                    int fps = num / den;
                    video->default_time = 1.0 / fps;//秒
                }
            }
        }
    }

    if (audio != NULL) {
        getCodecContext(audio->parameters, &audio->codecContext);
    }

    if (video != NULL) {
        getCodecContext(video->parameters, &video->codecContext);
    }
    callJava->onCallParpared(CHILD_THREAD);
    pthread_mutex_unlock(&init_mutex);
}

//开始解码
void ZFFmpeg::start() {
    if (audio == NULL) {
        LOGE("audio is null");
        return;
    }
    LOGE("开始解码");
    audio->play();
    video->audio = audio;
    video->play();
    int count = 0;
    while (playstatus != NULL && !playstatus->exit) {
        if (playstatus->seek) {
            continue;
        }
        if (audio->queen->getQueueSize() > 40) {
            continue;
        }

        AVPacket *packet = av_packet_alloc();
        if (av_read_frame(pFormatContext, packet) == 0) {
            if (packet->stream_index == audio->streamIndex) {
                count++;
                audio->queen->putAvpacket(packet);//加入音频播放列表
            } else if (packet->stream_index == video->streamIndex) {
                video->queen->putAvpacket(packet);//加入视频播放列表
            } else {
                av_packet_free(&packet);
                av_free(packet);
                packet = NULL;
            }
        } else {
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
            while (playstatus != NULL && !playstatus->exit) {
                if (audio->queen->getQueueSize() > 0) {
                    continue;
                } else {
                    playstatus->exit = true;
                    break;
                }
            }
        }
    }
    LOGE("解压完成");
}

void ZFFmpeg::selectChannel(int type) {

    if (audio != NULL) {
        audio->selectChannel(type);
    }
}

void ZFFmpeg::seekTo(int position) {
    if (duration <= 0) {
        return;
    }
    if (position >= 0 && position <= duration) {
        if (audio != NULL) {
            playstatus->seek = true;
            audio->queen->clearAvpacket();
            audio->clock = 0;
            audio->last_tiem = 0;
            pthread_mutex_lock(&seek_mutex);
            int64_t rel = position * AV_TIME_BASE;
            avformat_seek_file(pFormatContext, -1, INT64_MIN, rel, INT8_MAX, 0);
            pthread_mutex_unlock(&seek_mutex);
            playstatus->seek = false;
            LOGE("seek finish %d", rel);
        }
    }
}

void ZFFmpeg::setVolume(int percent) {
    if (audio != NULL) {
        audio->setVolume(percent);
    }
}

void ZFFmpeg::setSpeed(int type, float speed) {

    if (audio != NULL) {
        audio->setSpeed(type, speed);
    }
}

//公共方法初始化video audio 参数
int ZFFmpeg::getCodecContext(AVCodecParameters *codecpar, AVCodecContext **avCodecContext) {

    //AVCodec是存储编解码器信息的结构体
    AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        LOGE("获取编解码信息失败");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
//初始化解码器上下文
    *avCodecContext = avcodec_alloc_context3(codec);

    if (!audio->codecContext) {
        LOGE("初始化解码器上下文失败");
        return -1;
    }

    if (avcodec_parameters_to_context(*avCodecContext, codecpar) < 0) {
        LOGE("填充编码器上下文失败");
        exit = true;
        pthread_mutex_unlock(&init_mutex);

        return -1;
    }
    if (avcodec_open2(*avCodecContext, codec, NULL) != 0) {
        LOGE("打开解码器失败");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    return 0;
}

void ZFFmpeg::release() {

}

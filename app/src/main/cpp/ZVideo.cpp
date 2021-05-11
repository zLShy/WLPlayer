//
// Created by 张立 on 5/8/21.
//

#include "ZVideo.h"

ZVideo::ZVideo(ZCallJava *pJava, ZPlaystatus *pPlaystatus) {

    this->callJava = pJava;
    this->playstatus = pPlaystatus;
    this->queen = new ZQueen(pPlaystatus);
    pthread_mutex_init(&codecMutex, NULL);
}

void *playVideo(void *data) {
    ZVideo *video = static_cast<ZVideo *>(data);
    while (video->playstatus != NULL && !video->playstatus->exit) {

        if (video->playstatus->seek) {
            av_usleep(1000 * 100);
            continue;
        }
        if (video->playstatus->pause) {
            av_usleep(100 * 1000);
            continue;
        }
        if (video->queen->getQueueSize() == 0) {
            if (!video->playstatus->load) {
                video->playstatus->load = true;
                video->callJava->onLoad(CHILD_THREAD, true);
            }
            continue;
        }

        AVPacket *packet = av_packet_alloc();
        if (video->queen->getAvpacket(packet) != 0) {
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
            continue;
        }
        //        视频解码 比较耗时  多线程环境
        pthread_mutex_lock(&video->codecMutex);
        if (avcodec_send_packet(video->codecContext, packet) != 0) {
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
            pthread_mutex_unlock(&video->codecMutex);
            LOGE("packet null");
            continue;
        }
        AVFrame *avFrame = av_frame_alloc();
        if (avcodec_receive_frame(video->codecContext, avFrame) != 0) {
            //          括号就失败了
            LOGE("frame null ");
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
            pthread_mutex_unlock(&video->codecMutex);
            continue;
        }
//        此时解码成功了  如果 之前是yuv420  ----》   opengl
//        av_usleep(33 * 1000);
//      获取音视频差值
        double diff = video->getFrameDiffTime(avFrame);
        av_usleep(video->getDelayTime(diff) * 1000000);
        if (avFrame->format == AV_PIX_FMT_YUV420P) {
            av_usleep(33 * 1000);
            LOGE("yuv420  ");
            video->callJava->onCallRenderYUV(
                    avFrame->width,
                    avFrame->height,
                    avFrame->data[0],
                    avFrame->data[1],
                    avFrame->data[2]
            );

        } else {
            LOGE("other   ");

            AVFrame *pChangeFrame = av_frame_alloc();
            int num = av_image_get_buffer_size(
                    AV_PIX_FMT_YUV420P,
                    video->codecContext->width,
                    video->codecContext->height,
                    1
            );
            uint8_t *buffer = static_cast<uint8_t *>(av_malloc(num * sizeof(uint8_t)));//数据容器

            av_image_fill_arrays(
                    pChangeFrame->data,
                    pChangeFrame->linesize,
                    buffer,
                    AV_PIX_FMT_YUV420P,
                    video->codecContext->width,
                    video->codecContext->height,
                    1
            );
            SwsContext *swsContext = sws_getContext(
                    video->codecContext->width,
                    video->codecContext->height,
                    video->codecContext->pix_fmt,
                    video->codecContext->width,
                    video->codecContext->height,
                    AV_PIX_FMT_YUV420P,
                    SWS_BICUBIC, NULL, NULL, NULL
            );
            if (!swsContext) {
                av_frame_free(&pChangeFrame);
                av_free(pChangeFrame);
                av_free(buffer);
                pthread_mutex_unlock(&video->codecMutex);
                continue;
            }
            sws_scale(
                    swsContext,
                    reinterpret_cast<const uint8_t *const *>(avFrame->data),
                    avFrame->linesize,
                    0,
                    avFrame->height,
                    pChangeFrame->data,
                    pChangeFrame->linesize
            );

            video->callJava->onCallRenderYUV(video->codecContext->width,
                                             video->codecContext->height,
                                             pChangeFrame->data[0], pChangeFrame->data[1],
                                             pChangeFrame->data[2]);
            av_frame_free(&pChangeFrame);
            av_free(pChangeFrame);
            av_free(buffer);
            sws_freeContext(swsContext);
        }


        av_frame_free(&avFrame);
        av_free(avFrame);
        avFrame = NULL;
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
        LOGE("video start ");

        pthread_mutex_unlock(&video->codecMutex);

    }
    pthread_exit(&video->threadPlay);//退出线程
}

void ZVideo::play() {
    pthread_create(&threadPlay, NULL, playVideo, this);
}

double ZVideo::getDelayTime(double diff) {
    if (diff >= 10) {
        //音频超前太多
        queen->clearAvpacket();
        delayTime = default_time;
    } else if (diff <= -10) {
        //视频超前太多
        audio->queen->clearAvpacket();
        delayTime = default_time;
    } else if (diff >= 0.5) {
        delayTime = 0;
    } else if (diff <= -0.5) {
        delayTime = default_time * 2;
    } else if (diff > 0.003) {
        delayTime = delayTime * 2 / 3;
        if (delayTime < default_time / 2) {
            delayTime = default_time * 2 / 3;
        } else if (delayTime > default_time * 2) {
            delayTime = default_time * 2;
        }
    } else if (diff < -0.003) {
        delayTime = delayTime * 3 / 2;
        if (delayTime < default_time / 2) {
            delayTime = default_time * 2 / 3;
        } else if (delayTime > default_time * 2) {
            delayTime = default_time * 2;
        }
    }
    return delayTime;
}

double ZVideo::getFrameDiffTime(AVFrame *avFrame) {
    double pts = av_frame_get_best_effort_timestamp(avFrame);
    if (pts == AV_NOPTS_VALUE) {
        pts = 0;
    }
    pts *= av_q2d(time_base);
    if (pts > 0) {
        clock = pts;
    }
    double diff = audio->clock - clock;
    return diff;
}

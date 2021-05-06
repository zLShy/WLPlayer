//
// Created by 张立 on 4/29/21.
//

#include "ZAudio.h"

ZAudio::ZAudio(ZCallJava *callback, ZPlaystatus *playstatus, int samplerate) {

    this->callJava = callback;
    this->playstatus = playstatus;
    this->sampleRate = samplerate;
    this->queen = new ZQueen(playstatus);
    this->buffer = (uint8_t *) av_malloc(samplerate * 2 * 2);
    sampleBuffer = static_cast<SAMPLETYPE *>(malloc(samplerate * 2 * 2));

    soundTouch = new SoundTouch();
    soundTouch->setSampleRate(samplerate);
    soundTouch->setChannels(2);
    soundTouch->setTempo(speed);
    soundTouch->setPitch(pitch);
}

ZAudio::~ZAudio() {

}

void *decodePlay(void *data) {
    ZAudio *audio = (ZAudio *) data;

    audio->initOpenSLES();

    pthread_exit(&audio->thread_play);
}

void ZAudio::play() {
    pthread_create(&thread_play, NULL, decodePlay, this);
}

//播放   喇叭需要取出数据   ---》播放
void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {

    ZAudio *wlAudio = (ZAudio *) context;
    if (wlAudio != NULL) {
//        int buffersize = wlAudio->resampleAudio();
//        LOGE("audio --------");
        int buffersize = wlAudio->getSoundTouchData();
//        LOGE("size======%d", buffersize);
        if (buffersize > 0) {
            wlAudio->clock += buffersize / ((double) (wlAudio->sampleRate * 2 * 2));
            if (wlAudio->clock - wlAudio->last_tiem >= 0.1) {
                wlAudio->last_tiem = wlAudio->clock;
                wlAudio->callJava->onCallTimeInfo(CHILD_THREAD, wlAudio->clock, wlAudio->duration);
            }
        }

        (*wlAudio->pcmBufferQueue)->Enqueue(wlAudio->pcmBufferQueue, (char *) wlAudio->sampleBuffer,
                                            buffersize * 2 * 2);
    }
}

int ZAudio::resampleAudio(void **pcmbuf) {
    while (playstatus != NULL && !playstatus->exit) {
        avPacket = av_packet_alloc();
        if (queen->getAvpacket(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        ret = avcodec_send_packet(codecContext, avPacket);
        if (ret != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, avFrame);
        if (ret != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        } else {
            if (avFrame->channels && avFrame->channel_layout == 0) {
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0) {
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }
            SwrContext *swrContext;

            swrContext = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,
                    AV_SAMPLE_FMT_S16,
                    avFrame->sample_rate,
                    avFrame->channel_layout,
                    (AVSampleFormat) avFrame->format,
                    avFrame->sample_rate,
                    NULL,
                    NULL
            );

            if (!swrContext || swr_init(swrContext) < 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                swr_free(&swrContext);
                continue;
            }

            nb = swr_convert(swrContext,
                             &buffer,
                             avFrame->nb_samples,
                             (const uint8_t **) avFrame->data,
                             avFrame->nb_samples
            );
            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            now_time = avFrame->pts * av_q2d(time_base);
            if (now_time < clock) {
                now_time = clock;
            }
            clock = now_time;
            *pcmbuf = buffer;
//            LOGE("data_size is %d", data_size);

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swrContext);
            break;
        }

    }
    return data_size;
}

void ZAudio::initOpenSLES() {

    SLresult result;
    //  初始化引擎
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);

    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);

    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    //创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void) result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void) result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings
        );
        (void) result;
    }

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink dataSink = {&outputMix, 0};

    // 第三步，配置PCM格式信息
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};

    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,
            2,
            static_cast<SLuint32>(getCurrentSampleRateForOpensles(sampleRate)),
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };

    SLDataSource dataSource = {&android_queue, &pcm};
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_MUTESOLO};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &dataSource, &dataSink, 3,
                                       ids, req);
    //初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);
    //    得到接口后调用  获取Player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);
//   拿控制  播放暂停恢复的句柄
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmVolumePlay);
    //    获取声道操作接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_MUTESOLO, &pcmMutePlay);
//    注册回调缓冲区 获取缓冲队列接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);
    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);
//    获取播放状态接口
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    pcmBufferCallBack(pcmBufferQueue, this);
}

int ZAudio::getCurrentSampleRateForOpensles(int sample_rate) {
    int rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

void ZAudio::selectChannel(int type) {
    LOGE("切换声道");
    if (pcmMutePlay != NULL) {
        switch (type) {
            case 0://left
                (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, true);
                (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, false);
                break;
            case 1://right
                (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, false);
                (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, true);
                break;
            case 2://立体
                (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, false);
                (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, false);
                break;
            default:
                break;
        }
    }
}

void ZAudio::setVolume(int percent) {
    if (pcmVolumePlay != NULL) {
        if (percent > 30) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -20);
        } else if (percent > 25) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -22);
        } else if (percent > 20) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -25);
        } else if (percent > 15) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -28);
        } else if (percent > 10) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -30);
        } else if (percent > 5) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -34);
        } else if (percent > 3) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -37);
        } else if (percent > 0) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -40);
        } else {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -100);
        }
    }
}

void ZAudio::setSpeed(int type, float speed) {

    this->speed = speed;
    if (soundTouch != NULL) {
        LOGE("speed %f",speed);
        soundTouch->setTempo(speed);
    }

}

int ZAudio::getSoundTouchData() {
    while (playstatus != NULL && !playstatus->exit) {
        out_buffer = NULL;
        if (finished) {
            finished = false;
            data_size = this->resampleAudio(reinterpret_cast<void **>(&out_buffer));
            if (data_size > 0) {
                for (int i = 0; i < data_size / 2 + 1; i++) {
                    sampleBuffer[i] = (out_buffer[i * 2] | (out_buffer[i * 2 + 1] << 8));
                }
                soundTouch->putSamples(sampleBuffer, nb);
                num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);

            } else {
                soundTouch->flush();
            }
        }

        if (num == 0) {
            finished = true;
            continue;
        } else {
            if (out_buffer == NULL) {
                num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
                if (num == 0) {
                    finished = true;
                    continue;
                }
            }
            return num;
        }
    }
    return 0;
}


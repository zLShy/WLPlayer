//
// Created by 张立 on 4/29/21.
//

#ifndef FFMPEGMUSICPLAYER_ZPLAYSTATUS_H
#define FFMPEGMUSICPLAYER_ZPLAYSTATUS_H


class ZPlaystatus {
public:
    bool exit;
    bool seek = false;
    bool pause = false;
//    正在努力加载
    bool load = true;
public:
    ZPlaystatus();
};


#endif //FFMPEGMUSICPLAYER_ZPLAYSTATUS_H

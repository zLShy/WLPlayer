package com.zl.ffmpegmusicplayer.listener

open abstract class PlayListener : IPlayListener {
    override fun onLoad(load: Boolean) {
    }

    override fun onCurrentTime(curr: Int, totlal: Int) {
    }

    override fun onError(code: Int, msg: String) {
    }

    override fun onPause(pause: Boolean) {
    }

    override fun onDbValue(db: Int) {
    }

    override fun onComplete() {
    }

    override fun onNext() {
    }
}
package com.zl.ffmpegmusicplayer.listener

interface IPlayListener {
    fun onLoad(load:Boolean)
    fun onCurrentTime(curr:Int,totlal:Int)
    fun onError(code:Int,msg:String)
    fun onPause(pause:Boolean)
    fun onDbValue(db:Int)
    fun onComplete()
    fun onNext()
}
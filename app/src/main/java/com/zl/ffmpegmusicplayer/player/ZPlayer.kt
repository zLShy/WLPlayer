package com.zl.ffmpegmusicplayer.player

import android.util.Log
import com.zl.ffmpegmusicplayer.listener.IPlayListener
import com.zl.ffmpegmusicplayer.listener.WlOnParparedListener
import java.lang.Exception
import kotlin.concurrent.thread

class ZPlayer {
    private var vol = 1
    var mSource: String? = null;
    var mOnParparedListener: WlOnParparedListener? = null
    var mPlayListener: IPlayListener? = null

    companion object {
        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }

    fun onCallParpared() {
        Log.e("ZPlayer", "onCallParpared")
        thread {
            native_start()
        }
    }


    fun onCallTimeInfo(current: Int, total: Int) {
        mPlayListener?.onCurrentTime(current, total)
    }

    fun onParpared() {
        if (mSource == null) {
            return
        }

        thread {
            native_parpared(mSource!!)
        }
    }

    external fun native_parpared(source: String)
    external fun native_start()
    external fun native_channel_select(type: Int)
    external fun native_seek(seconds: Int)
    external fun native_changeVol(percent: Int)
    external fun native_changeTone(type: Int, speed: Float)
    fun leftPlay() {
        native_channel_select(0)
    }

    fun stereoPlay() {
        native_channel_select(2)
    }

    fun rightPlay() {
        native_channel_select(1)
    }

    fun seekto(position: Int) {
        native_seek(position)
    }

    //    @Throws(Exception::class)
    fun changeVol() {
        vol++
        native_changeVol(vol)
    }

    fun changeTone(type: Int,speed: Float) {
        native_changeTone(type, speed)
    }

}
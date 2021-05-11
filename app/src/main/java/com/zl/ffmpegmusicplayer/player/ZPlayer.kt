package com.zl.ffmpegmusicplayer.player

import android.util.Log
import com.zl.ffmpegmusicplayer.listener.IPlayListener
import com.zl.ffmpegmusicplayer.listener.WlOnParparedListener
import com.zl.ffmpegmusicplayer.opengl.HGLSurfaceView
import com.zl.ffmpegmusicplayer.opengl.ZGLSurfaceView
import kotlin.concurrent.thread

class ZPlayer {
    private var vol = 1
    var mSource: String? = null;
    var mOnParparedListener: WlOnParparedListener? = null
    var mPlayListener: IPlayListener? = null
    private var mSurfaceView: HGLSurfaceView? = null

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

    fun onCallLoad(load: Boolean) {

//        native   网络不行     再       肯定有
    }

    fun onCallTimeInfo(current: Int, total: Int) {
        mPlayListener?.onCurrentTime(current, total)
    }

    fun onParpared() {
        if (mSource == null) {
            return
        }
        Log.e("ZPlayer", "start")
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
    external fun native_release()
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

    fun changeTone(type: Int, speed: Float) {
        native_changeTone(type, speed)
    }

    fun setsurfaceView(zsurface: HGLSurfaceView) {
        mSurfaceView = zsurface
    }

    fun onCallRenderYUV(
        width: Int,
        height: Int,
        y: ByteArray,
        u: ByteArray,
        v: ByteArray
    ) {
//        opengl  的java版本
        Log.e("ZP", "video call back")
        if (this.mSurfaceView != null) {
            this.mSurfaceView?.setYuvData(width, height, y, u, v)
        }
    }
}
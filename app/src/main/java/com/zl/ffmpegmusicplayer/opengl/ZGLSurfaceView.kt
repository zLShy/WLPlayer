package com.zl.ffmpegmusicplayer.opengl

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet
import java.nio.ByteBuffer

class ZGLSurfaceView @JvmOverloads constructor(
    context: Context,
    attributeSet: AttributeSet
) :
    GLSurfaceView(context, attributeSet) {

    private val mRender = ZRender(context)

    init {
        setEGLContextClientVersion(2)
        setRenderer(mRender)
        renderMode = GLSurfaceView.RENDERMODE_WHEN_DIRTY
    }

    fun setYuvData(width: Int, heigth: Int, y: ByteArray, u: ByteArray, v: ByteArray) {
        if (mRender != null) {
            mRender.setYUVRenderData(width, height, y, u, v)
            requestLayout()
        }
    }

}
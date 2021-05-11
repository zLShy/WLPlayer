package com.zl.ffmpegmusicplayer.opengl

import android.content.Context
import android.opengl.GLES20
import android.opengl.GLSurfaceView
import android.util.Log
import com.zl.ffmpegmusicplayer.R
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.FloatBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class ZRender(context: Context) : GLSurfaceView.Renderer {
    private var v: ByteBuffer? = null
    private var u: ByteBuffer? = null
    private var y: ByteBuffer? = null
    private var heigth: Int = 0
    private var width: Int = 0
    private var sampler_v: Int = 0
    private var sampler_u: Int = 0
    private var sampler_y: Int = 0
    private var af_Position: Int = 0
    private var av_Position: Int = 0
    private var mProgram: Int = 0
    private val mContext = context
    private val textureId_yuv = IntArray(3)
    private val vertexData = floatArrayOf(
        -1f, -1f,
        1f, -1f,
        -1f, 1f,
        1f, 1f
    )

    private val textureData = floatArrayOf(
        0f, 1f,
        1f, 1f,
        0f, 0f,
        1f, 0f
    )

    private var vertexBuffer: FloatBuffer? = null
    private var textureBuffer: FloatBuffer? = null

    init {
        //顶点坐标传入
        vertexBuffer = ByteBuffer.allocateDirect(vertexData.size * 4)
            .order(ByteOrder.nativeOrder())
            .asFloatBuffer()
        vertexBuffer?.clear()
        vertexBuffer?.put(vertexData)
        //片源坐标传入
        textureBuffer = ByteBuffer.allocateDirect(textureData.size * 4)
            .order(ByteOrder.nativeOrder())
            .asFloatBuffer()
        textureBuffer?.clear()
        textureBuffer?.put(textureData)
        //获取program gup索引
        val sharderVert = ShaderUtils.readRawTxt(context, R.raw.base_vert)
        val sharderFrag = ShaderUtils.readRawTxt(context, R.raw.base_frag)
        mProgram = ShaderUtils.createProgram(sharderVert, sharderFrag)
        //获取索引
        av_Position = GLES20.glGetAttribLocation(mProgram, "av_Position")
        af_Position = GLES20.glGetAttribLocation(mProgram, "af_Position")
        //获取采样点
        sampler_y = GLES20.glGetUniformLocation(mProgram, "sampler_y")
        sampler_u = GLES20.glGetUniformLocation(mProgram, "sampler_u")
        sampler_v = GLES20.glGetUniformLocation(mProgram, "sampler_v")
//        textureId_yuv = intArrayOf(3)

        GLES20.glGenTextures(3, textureId_yuv, 0)
        for (index in 0 until 3) {
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[index])
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT)
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT)
            GLES20.glTexParameteri(
                GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_MIN_FILTER,
                GLES20.GL_LINEAR
            )
            GLES20.glTexParameteri(
                GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_MAG_FILTER,
                GLES20.GL_LINEAR
            )
        }
    }

    override fun onDrawFrame(gl: GL10?) {

        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT)
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f)
        renderYUV()
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4)
        Log.e("RD","===========")

    }

    fun setYUVRenderData(width: Int, height: Int, y: ByteArray, u: ByteArray, v: ByteArray) {
        this.width = width
        this.heigth = height
        this.y = ByteBuffer.wrap(y)
        this.u = ByteBuffer.wrap(u)
        this.v = ByteBuffer.wrap(v)
    }

    private fun renderYUV() {
        if (width > 0 && heigth > 0 && y != null && u != null && v != null) {
            GLES20.glUseProgram(mProgram)
            GLES20.glEnableVertexAttribArray(av_Position)
            GLES20.glVertexAttribPointer(av_Position, 2, GLES20.GL_FLOAT, false, 8, vertexBuffer)
            GLES20.glEnableVertexAttribArray(af_Position)
            GLES20.glVertexAttribPointer(af_Position, 2, GLES20.GL_FLOAT, false, 8, textureBuffer)

            GLES20.glActiveTexture(GLES20.GL_TEXTURE0)
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[0])
            GLES20.glTexImage2D(
                GLES20.GL_TEXTURE_2D,
                0,
                GLES20.GL_LUMINANCE,
                width,
                heigth,
                0,
                GLES20.GL_LUMINANCE,
                GLES20.GL_UNSIGNED_BYTE,
                this.y
            )

            GLES20.glActiveTexture(GLES20.GL_TEXTURE1)
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[1])
            GLES20.glTexImage2D(
                GLES20.GL_TEXTURE_2D,
                0,
                GLES20.GL_LUMINANCE,
                width / 2,
                heigth / 2,
                0,
                GLES20.GL_LUMINANCE,
                GLES20.GL_UNSIGNED_BYTE,
                this.u
            )
            GLES20.glActiveTexture(GLES20.GL_TEXTURE2)
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId_yuv[2])
            GLES20.glTexImage2D(
                GLES20.GL_TEXTURE_2D,
                0,
                GLES20.GL_LUMINANCE,
                width / 2,
                heigth / 2,
                0,
                GLES20.GL_LUMINANCE,
                GLES20.GL_UNSIGNED_BYTE,
                this.v
            )

            GLES20.glUniform1i(sampler_y, 0)
            GLES20.glUniform1i(sampler_u, 1)
            GLES20.glUniform1i(sampler_v, 2)
            y?.clear()
            u?.clear()
            v?.clear()
            y = null
            u = null
            v = null
        }
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {

    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
    }

}
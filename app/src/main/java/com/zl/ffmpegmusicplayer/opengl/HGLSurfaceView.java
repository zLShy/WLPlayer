package com.zl.ffmpegmusicplayer.opengl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;

public class HGLSurfaceView extends GLSurfaceView {
    private HRender mRender;

    public HGLSurfaceView(Context context) {
        this(context, null);
    }

    public HGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        mRender = new HRender(context);
        setRenderer(mRender);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    public void setYuvData(int width, int height, byte[] y, byte[] u, byte[] v) {
//        invildate
        if (mRender != null) {
            mRender.setYUVRenderData(width, height, y, u, v);
            requestRender();
        }

    }
}

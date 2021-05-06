package com.zl.ffmpegmusicplayer

import android.Manifest
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.SeekBar
import com.tbruyelle.rxpermissions3.RxPermissions
import com.zl.ffmpegmusicplayer.listener.PlayListener
import com.zl.ffmpegmusicplayer.musicui.utils.DisplayUtil
import com.zl.ffmpegmusicplayer.player.ZPlayer
import kotlinx.android.synthetic.main.activity_main.*
import kotlin.concurrent.thread

class MainActivity : AppCompatActivity() {
    private var position = 0
    val player = ZPlayer()
    var totalTime = 0
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        RxPermissions(this).request(
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.CAMERA
        ).subscribe { aBoolean ->
            player.mSource = "http://mn.maliuedu.com/music/dengniguilai.mp3"
            player.onParpared()
        }
        player.mPlayListener = object : PlayListener() {

            override fun onCurrentTime(curr: Int, totlal: Int) {
                Log.e("Time", "${curr} / ${totlal}")
                tvTotalTime.post {
                    musicSeekBar.setProgress(curr * 100 / totlal);
                    tvCurrentTime.setText(DisplayUtil.secdsToDateFormat(curr, totlal))
                    tvTotalTime.setText(DisplayUtil.secdsToDateFormat(totlal, totlal))
                    totalTime = totlal
                }
            }
        }

        musicSeekBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                position = totalTime * progress / 100
                tvCurrentTime.setText(DisplayUtil().duration2Time(progress))
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {

            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                player.seekto(position)
            }

        })
        tvCurrentTime.setText(DisplayUtil().duration2Time(0))
        tvTotalTime.setText(DisplayUtil().duration2Time(0))

        thread {
            while (true) {
                Thread.sleep(100)
                player.changeVol()
            }
        }
    }

    fun left(view: View) {
        player.leftPlay()
    }

    fun center(view: View) {
        player.stereoPlay()
    }

    fun right(view: View) {
        player.rightPlay()

    }

    fun changeTone(view: View) {
        player.changeTone(1,1.2F)
    }

    fun changeSpeed(view: View) {
        player.changeTone(2,1.5F)
    }

    fun changeAll(view: View) {
        player.changeTone(3,2.0F)
    }

    fun normalPlay(view: View) {
        player.changeTone(0,1.0F)
    }


}

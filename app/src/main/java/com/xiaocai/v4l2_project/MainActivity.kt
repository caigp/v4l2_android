package com.xiaocai.v4l2_project

import android.graphics.Bitmap
import android.os.Bundle
import android.view.SurfaceHolder
import android.view.SurfaceHolder.Callback
import androidx.appcompat.app.AppCompatActivity
import com.xiaocai.v4l2.V4L2
import com.xiaocai.v4l2_project.databinding.ActivityMainBinding
import kotlin.concurrent.thread

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    private var v4L2: V4L2 = V4L2()

    private var draw: Boolean = true

    private val lock: Any = Any()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.surface.holder.addCallback(object : Callback {
            override fun surfaceCreated(holder: SurfaceHolder) {
                v4L2.open("/dev/video1", 1920, 1080)
                thread {
                    while (draw) {
                        var bitmap: Bitmap?
                        synchronized(lock) {
                            bitmap = v4L2.nextFrame()
                        }
                        if (bitmap != null) {
                            try {
                                val canvas = holder.lockCanvas()
                                canvas.drawBitmap(bitmap!!, 0f, 0f, null)
                                holder.unlockCanvasAndPost(canvas)
                            } catch (e: Exception) {
                            }
                        }
                    }
                }
            }

            override fun surfaceChanged(
                holder: SurfaceHolder,
                format: Int,
                width: Int,
                height: Int
            ) {

            }

            override fun surfaceDestroyed(holder: SurfaceHolder) {
                draw = false
                synchronized(lock) {
                    v4L2.close()
                }

            }
        })
    }

}
package com.xiaocai.v4l2_project

import android.graphics.Bitmap
import android.os.Bundle
import android.os.SystemClock
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceHolder.Callback
import androidx.appcompat.app.AppCompatActivity
import com.xiaocai.v4l2.V4L2
import com.xiaocai.v4l2_project.databinding.ActivityMainBinding
import java.io.File
import java.nio.charset.Charset
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
                getVideoPath(arrayListOf("EP28WD"))?.let {
                    v4L2.open(it, 1920, 1080)
                }

                thread {
                    var fps = 0
                    var start = SystemClock.elapsedRealtime()
                    while (draw) {
                        var bitmap: Bitmap?
                        synchronized(lock) {
                            bitmap = v4L2.nextFrame()
                        }
                        if (bitmap == null) {
                            break
                        }

                        try {
                            val canvas = holder.lockCanvas()
                            canvas.drawBitmap(bitmap!!, 0f, 0f, null)
                            holder.unlockCanvasAndPost(canvas)
                        } catch (_: Exception) {
                        }
                        fps++
                        val end = SystemClock.elapsedRealtime()
                        if (end - start > 1000) {
                            start = end
                            val text = fps
                            fps = 0
                            runOnUiThread {
                                binding.textView.text = text.toString()
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
    fun getVideoPath(names: List<String?>): String? {
        val file = File("/sys/class/video4linux/")
        file.listFiles()?.forEach {
            val name = File(it, "name").readText(Charset.defaultCharset())
            val index = File(it, "index").readText(Charset.defaultCharset())
            Log.d("xxx", "video name: $name")
            if (names.contains(name.trim()) && "0" == index.trim()) {
                return String.format("/dev/%s", it.name)
            }
        }
        return null
    }
}
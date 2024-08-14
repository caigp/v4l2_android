package com.xiaocai.v4l2_project

import android.graphics.Bitmap
import android.graphics.Matrix
import android.os.Bundle
import android.os.SystemClock
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceHolder.Callback
import android.view.View
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

    private var thread: Thread? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.surface.holder.addCallback(object : Callback {
            override fun surfaceCreated(holder: SurfaceHolder) {
                getVideoPath(arrayListOf("USB2.0_CAM1"))?.let {
                    v4L2.open(it)
                }
                val supportSize = v4L2.getSupportSize()
                Log.d("xxx", "surfaceCreated: $supportSize")
                v4L2.startCapture(1280, 960)

                thread = thread {
                    var fps = 0
                    var start = SystemClock.elapsedRealtime()
                    while (draw) {
                        val bitmap: Bitmap = v4L2.nextFrame() ?: break

                        try {
                            val canvas = holder.lockCanvas()
                            val matrix = Matrix()

                            val s = (canvas.width / bitmap.width.toFloat()).coerceAtMost(canvas.height / bitmap.height.toFloat())

                            matrix.preTranslate((canvas.width - bitmap.width) / 2f, (canvas.height - bitmap.height) / 2f)
                            matrix.preScale(-s, s, bitmap.width / 2f, bitmap.height / 2f)

                            canvas.drawBitmap(bitmap, matrix, null)
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
                thread?.join()
                v4L2.close()
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
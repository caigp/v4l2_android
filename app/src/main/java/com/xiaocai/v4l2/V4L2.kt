package com.xiaocai.v4l2

import android.graphics.Bitmap
import java.io.DataOutputStream
import java.io.File
import java.nio.ByteBuffer

class V4L2 {

    init {
        System.loadLibrary("v4l2")
    }

    private var bitmap: Bitmap? = null
    private var byteArray: ByteArray? = null

    private var nativePtr: Long = 0

    fun open(video: String, width: Int, height: Int): Int {
        val f = File(video)
        if (!f.canRead() || !f.canWrite()) {
            sudo(f)
        }
        val ret = nativeOpen(video, nativePtr)
        if (ret == -1) {
            return -1
        }
        bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
        byteArray = ByteArray(width * height * 4)
        return startCapture(nativePtr, width, height)
    }

    fun nextFrame(): Bitmap? {
        val ret = loadNext(nativePtr)
        if (ret != -1) {
            getRgba(nativePtr, byteArray)
            bitmap?.copyPixelsFromBuffer(ByteBuffer.wrap(byteArray!!))
        }
        return bitmap
    }

    fun close(): Int {
        stopCapture(nativePtr)
        return nativeClose(nativePtr)
    }

    private fun sudo(device: File): Int {
        val cmd = "chmod 666 " + device.absolutePath + "\nexit\n";
        var process: Process? = null
        try {
            process = Runtime.getRuntime().exec("su")
            val os = DataOutputStream(process.outputStream)
            os.write(cmd.toByteArray())
            os.flush()
            if (process.waitFor() != 0 || !device.canRead() || !device.canWrite()) {
                return -1
            }
        } catch (_: Exception) {

        } finally {
            process?.destroy()
        }
        return 0
    }

    private external fun nativeOpen(video: String, ptr: Long): Int
    private external fun nativeClose(ptr: Long): Int
    private external fun startCapture(ptr: Long, width: Int, height: Int): Int
    private external fun stopCapture(ptr: Long)
    private external fun loadNext(ptr: Long): Int
    private external fun getRgba(ptr: Long, byteArray: ByteArray?)
}
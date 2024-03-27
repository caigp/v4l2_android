package com.xiaocai.v4l2

import android.graphics.Bitmap
import java.io.DataOutputStream
import java.io.File
import java.nio.ByteBuffer

class V4L2 {

    init {
        System.loadLibrary("v4l2")
    }

    var deviceId: Int = -1

    var bitmap: Bitmap? = null

    var byteArray: ByteArray? = null

    fun open(video: String, width: Int, height: Int): Int {
        val f = File(video)
        if (!f.exists()) {
            return -1
        }
        if (!f.canRead() || !f.canWrite()) {
            val ret = sudo(f)
            if (ret == -1) {
                return ret
            }
        }
        deviceId = nativeOpen(video)
        if (deviceId == -1) {
            return -1
        }
        bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
        byteArray = ByteArray(width * height * 4)
        return startCapture(deviceId, width, height)
    }

    fun nextFrame(): Bitmap? {
        loadNext(deviceId, byteArray)
        bitmap?.copyPixelsFromBuffer(ByteBuffer.wrap(byteArray!!))
        return bitmap
    }

    fun close(): Int {
        if (deviceId != -1) {
            stopCapture(deviceId)
        }
        val ret = nativeClose(deviceId)
        deviceId = -1
        return ret
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

    private external fun nativeOpen(video: String): Int
    private external fun nativeClose(deviceId: Int): Int
    private external fun startCapture(deviceId: Int, width: Int, height: Int): Int
    private external fun stopCapture(deviceId: Int)
    private external fun loadNext(deviceId: Int, byteArray: ByteArray?)
}
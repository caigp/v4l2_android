#include <jni.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <android/log.h>
#include <cstdlib>
#include <sys/mman.h>
#include <string>
#include <android/bitmap.h>
#include <map>
#include <unistd.h>
#include "turbojpeg.h"

#define TAG "v4l2_camera"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

#define CAP_BUF_NUM 4

struct buffer {
    void * start;
    unsigned int length;
};

struct dev {
    int width;
    int height;
    struct buffer *buffers;
};

std::map<int, struct dev *> dev_map;

extern "C"
JNIEXPORT jint JNICALL
Java_com_xiaocai_v4l2_V4L2_nativeClose(JNIEnv *env, jobject thiz, jint device_id) {
    return close(device_id);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_xiaocai_v4l2_V4L2_nativeOpen(JNIEnv *env, jobject thiz, jstring video) {
    const char *path = env->GetStringUTFChars(video, JNI_FALSE);
    int fd = open(path, O_RDWR);
    return fd;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_xiaocai_v4l2_V4L2_startCapture(JNIEnv *env, jobject thiz, jint device_id, jint width,
                                        jint height) {
    struct dev * video = (struct dev *) malloc(sizeof(struct dev));
    video->width = width;
    video->height = height;
    struct buffer *buffers;
    buffers = (struct buffer *) malloc(CAP_BUF_NUM * sizeof(struct buffer));
    video->buffers = buffers;

    dev_map[device_id] = video;

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if (ioctl(device_id, VIDIOC_S_FMT, &fmt) == -1) {
        return -1;
    }

    struct v4l2_streamparm stream_para;
    //设置帧率30
    memset(&stream_para, 0, sizeof(struct v4l2_streamparm));
    stream_para.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    stream_para.parm.capture.timeperframe.denominator = 30;
    stream_para.parm.capture.timeperframe.numerator = 1;

    if (ioctl(device_id, VIDIOC_S_PARM, &stream_para) == -1) {
        return -1;
    }

    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = CAP_BUF_NUM;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(device_id, VIDIOC_REQBUFS, &req) == -1) {
        return -1;
    }

    for (int i = 0; i < CAP_BUF_NUM; ++i) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(device_id, VIDIOC_QUERYBUF, &buf) == -1) {
            return -1;
        }

        buffers[i].length = buf.length;
        buffers[i].start = mmap(NULL, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, device_id, buf.m.offset);

        if (buffers[i].start == MAP_FAILED) {
            return -1;
        }
    }

    for (int i = 0; i < CAP_BUF_NUM; ++i) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(device_id, VIDIOC_QBUF, &buf) == -1) {
            return -1;
        }
    }

    //开启视频流
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(device_id, VIDIOC_STREAMON, &type) == -1) {
        return -1;
    }

    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xiaocai_v4l2_V4L2_loadNext(JNIEnv *env, jobject thiz, jint device_id,
                                    jbyteArray byte_array) {
    if (device_id != -1)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        ioctl(device_id, VIDIOC_DQBUF, &buf);
        struct buffer *buffers = dev_map[device_id]->buffers;
        jbyte * dstBuf = env->GetByteArrayElements(byte_array, JNI_FALSE);

        tjhandle tjh = tjInitDecompress();
        tjDecompress2(tjh, (const unsigned char *) buffers[buf.index].start, buffers[buf.index].length, (unsigned char *) dstBuf,
                      dev_map[device_id]->width, 0, dev_map[device_id]->height, TJPF_RGBA, 0);
        tjDestroy(tjh);

        env->ReleaseByteArrayElements(byte_array, dstBuf, JNI_OK);

        ioctl(device_id, VIDIOC_QBUF, &buf);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_xiaocai_v4l2_V4L2_stopCapture(JNIEnv *env, jobject thiz, jint device_id) {
    //关闭视频流
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(device_id, VIDIOC_STREAMOFF, &type);

    struct dev *video = dev_map[device_id];
    struct buffer *buffers = video->buffers;

    for (int i = 0; i < CAP_BUF_NUM; ++i) {
        munmap(buffers[i].start, buffers[i].length);
    }

    //释放内存
    free(buffers);
    free(video);

    dev_map.erase(device_id);
}
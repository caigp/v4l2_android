//
// Created by android_01 on 2024/4/12.
//

#ifndef V4L2_PROJECT_V4L2CAMERA_H
#define V4L2_PROJECT_V4L2CAMERA_H

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

class MyVideo {

private:
    void *rgba = NULL;
    int fd;
    struct buffer *buffers = NULL;

public:
    MyVideo();
    int openVideo(const char *pathname);
    int closeVideo();
    int startCapture(int width, int height);
    void stopCapture();
    int loadNext();
    void * getRgba();
    int width;
    int height;

};

#endif //V4L2_PROJECT_V4L2CAMERA_H

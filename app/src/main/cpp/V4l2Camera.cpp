//
// Created by android_01 on 2024/4/12.
//

#include "V4l2Camera.h"

int MyVideo::startCapture(int w, int h) {
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = w;
    fmt.fmt.pix.height = h;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        return -1;
    }

    MyVideo::width = fmt.fmt.pix.width;
    MyVideo::height = fmt.fmt.pix.height;
    buffers = (struct buffer *) malloc(CAP_BUF_NUM * sizeof(struct buffer));
    rgba = malloc(width * height * 4);
    LOGD("set width=%d, height=%d", width, height);

    struct v4l2_streamparm stream_para;
    //设置帧率30
    memset(&stream_para, 0, sizeof(struct v4l2_streamparm));
    stream_para.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    stream_para.parm.capture.timeperframe.denominator = 30;
    stream_para.parm.capture.timeperframe.numerator = 1;

    if (ioctl(fd, VIDIOC_S_PARM, &stream_para) == -1) {
        return -1;
    }

    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = CAP_BUF_NUM;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        return -1;
    }

    for (int i = 0; i < CAP_BUF_NUM; ++i) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
            return -1;
        }

        buffers[i].length = buf.length;
        buffers[i].start = mmap(NULL, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

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

        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
            return -1;
        }
    }

    //开启视频流
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret = 0;
    ret = ioctl(fd, VIDIOC_STREAMON, &type);
    LOGD("VIDIOC_STREAMON = %d", ret);
    if (ret == -1) {
        return -1;
    }

    return 0;
}

void MyVideo::stopCapture() {
    //关闭视频流
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMOFF, &type);

    if (buffers) {
        for (int i = 0; i < CAP_BUF_NUM; ++i) {
            munmap(buffers[i].start, buffers[i].length);
        }
        free(buffers);
        buffers = NULL;
    }

    if (rgba) {
        free(rgba);
        rgba = NULL;
    }
}

MyVideo::MyVideo() {

}

int MyVideo::openVideo(const char *pathname) {
    fd = open(pathname, O_RDWR);
    LOGD("device_id = %d", fd);

    struct v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
        return fd;
    }

    LOGD("Driver name: %s\n", cap.driver);
    LOGD("Card name: %s\n", cap.card);
    LOGD("Bus info: %s\n", cap.bus_info);
    LOGD("Version: %u.%u.%u\n",
           (cap.version >> 16) & 0xFF,
           (cap.version >> 8) & 0xFF,
           cap.version & 0xFF);
    LOGD("Capabilities: 0x%X\n", cap.capabilities);

    return fd;
}

int MyVideo::closeVideo() {
    return close(fd);
}

int MyVideo::loadNext() {
    int ret = -1;
    if (fd != -1)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        ret = ioctl(fd, VIDIOC_DQBUF, &buf);

        tjhandle tjh = tjInitDecompress();
        tjDecompress2(tjh, (const unsigned char *) buffers[buf.index].start, buffers[buf.index].length, (unsigned char *) rgba,
                      width, 0, height, TJPF_RGBA, 0);
        tjDestroy(tjh);

        ret = ioctl(fd, VIDIOC_QBUF, &buf);
    }
    return ret;
}

void *MyVideo::getRgba() {
    return rgba;
}

std::vector<Size> MyVideo::supportSize() {
    std::vector<Size> supportSize;
    if (fd != -1) {
        struct v4l2_frmsizeenum frmsize;
        memset(&frmsize, 0, sizeof(frmsize));
        frmsize.pixel_format = V4L2_PIX_FMT_MJPEG;
        frmsize.index = 0;

        while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
            LOGD("frame_size<%d*%d>", frmsize.discrete.width, frmsize.discrete.height);
            struct Size size;
            size.width = frmsize.discrete.width;
            size.height = frmsize.discrete.height;
            supportSize.push_back(size);

            frmsize.index++;
        }
    }
    return supportSize;
}

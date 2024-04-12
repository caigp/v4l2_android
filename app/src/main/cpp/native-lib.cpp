#include <jni.h>
#include "turbojpeg.h"
#include "V4l2Camera.h"

void setJavaPtr(JNIEnv *env, jobject thiz, jlong ptr) {
    jclass _jclass = env->GetObjectClass(thiz);
    jfieldID _jfieldID = env->GetFieldID(_jclass, "nativePtr", "J");
    env->SetLongField(thiz, _jfieldID, (jlong) ptr);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_xiaocai_v4l2_V4L2_nativeClose(JNIEnv *env, jobject thiz, jlong ptr) {
    int ret = -1;
    if (ptr != 0) {
        auto *myVideo = (MyVideo *) ptr;
        ret = myVideo->closeVideo();

        delete myVideo;
        setJavaPtr(env, thiz, 0);
    }
    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_xiaocai_v4l2_V4L2_nativeOpen(JNIEnv *env, jobject thiz, jstring video, jlong ptr) {

    if (ptr != 0) {
        auto *myVideo = (MyVideo *) ptr;
        delete myVideo;
    }

    const char *pathname = env->GetStringUTFChars(video, JNI_FALSE);

    auto *myVideo = new MyVideo();
    int ret = myVideo->openVideo(pathname);

    setJavaPtr(env, thiz, (jlong) myVideo);

    return ret;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_xiaocai_v4l2_V4L2_startCapture(JNIEnv *env, jobject thiz, jlong ptr, jint width,
                                        jint height) {
    if (ptr != 0) {
        auto *myVideo = (MyVideo *) ptr;
        return myVideo->startCapture(width, height);
    }
    return -1;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_xiaocai_v4l2_V4L2_loadNext(JNIEnv *env, jobject thiz, jlong ptr) {
    if (ptr != 0) {
        auto * myVideo = (MyVideo *) ptr;
        return myVideo->loadNext();
    }
    return -1;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xiaocai_v4l2_V4L2_stopCapture(JNIEnv *env, jobject thiz, jlong ptr) {
    if (ptr != 0) {
        auto * myVideo = (MyVideo *) ptr;
        myVideo->stopCapture();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xiaocai_v4l2_V4L2_getRgba(JNIEnv *env, jobject thiz, jlong ptr, jbyteArray byte_array) {
    if (ptr != 0) {
        auto *myVideo = (MyVideo *) ptr;
        env->SetByteArrayRegion(byte_array, 0, myVideo->width * myVideo->height * 4, (const jbyte *) myVideo->getRgba());
    }
}
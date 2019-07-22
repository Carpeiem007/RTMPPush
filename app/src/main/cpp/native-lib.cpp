#include <jni.h>
#include <string>
#include "x264.h"
#include "librtmp/rtmp.h"
#include "VideoChannel.h"
#include "safe_queue.h"
#include "AudioChannel.h"
#include "Marco.h"

uint32_t start_time;
int readyPushing;
SafeQueue<RTMPPacket *> packets;

void release(RTMPPacket *&packet);

void callback(RTMPPacket *packet) {
    if (packet) {
        packet->m_nTimeStamp = RTMP_GetTime() - start_time;
        packets.put(packet);
    }
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_gg_demo_rtmppush_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    x264_picture_t *p = new x264_picture_t;
    RTMP *rmp = RTMP_Alloc();
    LOGV("native success");

    return env->NewStringUTF(hello.c_str());
}
VideoChannel *videoChannel;
bool isStart;
pthread_t thread;
AudioChannel *audioChannel;
extern "C"
JNIEXPORT void JNICALL
Java_com_gg_demo_rtmppush_LivePusher_init(JNIEnv *env, jobject instance) {
    LOGV("start init");
    videoChannel = new VideoChannel;
    videoChannel->setVideoCallback(callback);
    audioChannel = new AudioChannel;
    audioChannel->setCallback(callback);

}extern "C"
JNIEXPORT void JNICALL
Java_com_gg_demo_rtmppush_LivePusher_native_1setVideoEncInfo(JNIEnv *env, jobject instance,
                                                             jint width, jint height, jint fps,
                                                             jint bitrate) {
    LOGV("start setVideoEncInfo");
    if (!videoChannel) {
        return;
    }
    videoChannel->setVideoEncInfo(width, height, fps, bitrate);

}


void *start(void *f) {
    char *url = static_cast<char *>(f);
    RTMP *rtmp = 0;
    rtmp = RTMP_Alloc();
    if (rtmp == 0) {
        LOGV("RTMP INIT FAILED");
        return NULL;
    }
    RTMP_Init(rtmp);
    int result = RTMP_SetupURL(rtmp, url);
    if (!result) {
        LOGV("RTMP connect FAILED");
        return NULL;
    }
    rtmp->Link.timeout = 5;
    RTMP_EnableWrite(rtmp);
    result = RTMP_Connect(rtmp, 0);
    if (!result) {
        LOGV("RTMP stream failed");
        return NULL;
    }
    result = RTMP_ConnectStream(rtmp,0);
    LOGV("connectStream result = %d",result);
    start_time = RTMP_GetTime();
    readyPushing = 1;
    packets.setWork(1);
    RTMPPacket *packet = 0;
    while (readyPushing) {
        packets.get(packet);
        LOGV("取出一帧数据");
        if (!readyPushing) {
            break;
        }
        if (!packet) {
            continue;
        }
        packet->m_nInfoField2 = rtmp->m_stream_id;
        int send = RTMP_SendPacket(rtmp, packet, 1);
        LOGV("send result = %d", send);
        //释放packet
        release(packet);

    }
    LOGV("release");
    isStart = false;
    readyPushing = 0;
    packets.setWork(0);
    packets.clear();
    if (rtmp) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
    }

}

void release(RTMPPacket *&packet) {
    if (packet) {
        RTMPPacket_Free(packet);
        delete (packet);
        packet = 0;

    }
}


extern "C"
JNIEXPORT void JNICALL
Java_com_gg_demo_rtmppush_LivePusher_native_1start(JNIEnv *env, jobject instance, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    if (isStart)
        return;
    isStart = true;
    char *url = new char[strlen(path) + 1];
    strcpy(url, path);
    pthread_create(&thread, 0, start, url);

    env->ReleaseStringUTFChars(path_, path);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_gg_demo_rtmppush_LivePusher_native_1pushVideo(JNIEnv *env, jobject instance,
        jbyteArray data_) {
    if (!videoChannel || !readyPushing) {
        return;
    }
    jbyte *data = env->GetByteArrayElements(data_, NULL);
    // TODO
    videoChannel->encodeData(data);

    env->ReleaseByteArrayElements(data_, data, 0);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_gg_demo_rtmppush_LivePusher_native_1pushAudio(JNIEnv *env, jobject instance,
                                                       jbyteArray bytes_, jint len) {
    jbyte *bytes = env->GetByteArrayElements(bytes_, NULL);
    if (!audioChannel || !readyPushing) {
        return;
    }
    audioChannel->encodeData(bytes);

    // TODO

    env->ReleaseByteArrayElements(bytes_, bytes, 0);
}extern "C"
JNIEXPORT void JNICALL
Java_com_gg_demo_rtmppush_LivePusher_native_1setAudioEncInfo(JNIEnv *env, jobject instance, jint i,
                                                             jint channels) {
    if (audioChannel) {
        audioChannel->setAudioEncInfo(i, channels);
        // TODO
    }

}extern "C"
JNIEXPORT jint JNICALL
Java_com_gg_demo_rtmppush_LivePusher_getInputSamples(JNIEnv *env, jobject instance) {

    // TODO
    return audioChannel->getInputSamples();

}extern "C"
JNIEXPORT void JNICALL
Java_com_gg_demo_rtmppush_LivePusher_native_1release(JNIEnv *env, jobject instance) {
    delete (videoChannel);
    delete (audioChannel);
    // TODO
    readyPushing = -1;

}
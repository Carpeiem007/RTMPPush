//
// Created by guo on 2019-06-13.
//

#ifndef RTMPPUSH_VIDEOCHANNEL_H
#define RTMPPUSH_VIDEOCHANNEL_H


#include "librtmp/rtmp.h"

class VideoChannel {
    typedef void (*VideoCallback)(RTMPPacket* packet);
private:
    int mWidth,mHeight,mFps,mBitrate,ySize,uvSize;
    x264_t *videoCodec;
    x264_picture_t * pic_in;
    VideoCallback videoCallback;
public:
    void setVideoEncInfo(int width, int height, int fps, int bitrate);
    void encodeData(int8_t *data);

    void sendSpsPps(uint8_t sps[100], uint8_t pps[100], int len, int pps_len);


    void sendFrame(int type, uint8_t *payload, int i_payload);

    void setVideoCallback(VideoCallback videoCallback);

};


#endif //RTMPPUSH_VIDEOCHANNEL_H

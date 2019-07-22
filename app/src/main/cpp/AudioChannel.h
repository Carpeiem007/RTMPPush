//
// Created by 郭强 on 2019-06-27.
//

#ifndef RTMPPUSH_AUDIOCHANNEL_H
#define RTMPPUSH_AUDIOCHANNEL_H


#include <endian.h>
#include <faac.h>
#include <sys/types.h>
#include "librtmp/rtmp.h"

class AudioChannel {
    typedef void (*AudioCallback) (RTMPPacket *packet);
private:
    int mCahnnels;
    u_char* buffer ;
    faacEncHandle  audioCodec;
    u_long inputSamples;
    u_long maxOutputBytes;
    AudioCallback audioCallback;
public:
    void encodeData(int8_t* i);
    void setAudioEncInfo(int sampleHz,int channels);
    int getInputSamples();
    void setCallback(AudioCallback audioCallback1){
     this->audioCallback = audioCallback1;
    }

};


#endif //RTMPPUSH_AUDIOCHANNEL_H

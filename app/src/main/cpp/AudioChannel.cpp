//
// Created by guo on 2019-06-27.
//

#include "AudioChannel.h"
#include "librtmp/rtmp.h"
#include <string.h>
void AudioChannel::encodeData(int8_t* i) {
   int bytelen =  faacEncEncode(audioCodec, reinterpret_cast<int32_t *>(i), inputSamples, buffer, maxOutputBytes);
    if(bytelen){
        int bodySize = 2+bytelen;
        RTMPPacket *packet = new RTMPPacket;
        RTMPPacket_Alloc(packet,bodySize);
        packet->m_body[0] = 0xAF;
        packet->m_body[1]= 0x01;
        memcpy(&(packet->m_body[2]),buffer,bytelen);
        packet->m_hasAbsTimestamp=0;
        packet->m_nBodySize = bodySize;
        packet->m_nChannel = 0x11;
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
        packet->m_packetType=RTMP_PACKET_TYPE_AUDIO;
        if(audioCallback){
            audioCallback(packet);
        }
    }
}

void AudioChannel::setAudioEncInfo(int sampleHz, int channels) {
    audioCodec = faacEncOpen(sampleHz, channels, &inputSamples, &maxOutputBytes);
    faacEncConfigurationPtr config =  faacEncGetCurrentConfiguration(audioCodec);
    config->mpegVersion = MPEG4;
    config->aacObjectType = LOW;
    config->inputFormat = FAAC_INPUT_16BIT;
    config->outputFormat = 0;
    faacEncSetConfiguration(audioCodec,config);
    buffer = new u_char[maxOutputBytes];
}

int AudioChannel::getInputSamples() {
    return inputSamples;
}

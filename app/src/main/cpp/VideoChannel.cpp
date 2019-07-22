//
// Created by guo on 2019-06-13.
//
#include <stdint.h>
#include "x264.h"
#include "VideoChannel.h"
#include "librtmp/rtmp.h"
#include <string.h>
#include "Marco.h"

void VideoChannel::setVideoEncInfo(int width, int height, int fps, int bitrate) {
    mWidth = width;
    mHeight = height;
    mFps = fps;
    mBitrate = bitrate;
    ySize = width * height;
    uvSize = ySize /4;
    //初始化x264的编码器
    //编码
    LOGV("video init  fps = %d",mFps);
    x264_param_t param;
    x264_param_default_preset(&param, "ultrafast", "zerolatency");
    param.i_level_idc = 32;
    param.i_csp = X264_CSP_I420;
    param.i_width = width;
    param.i_height = height;
    //无B帧
    param.i_bframe = 0;
    param.rc.i_rc_method = X264_RC_ABR;
    param.rc.i_bitrate = bitrate / 1000;
    param.rc.i_vbv_max_bitrate = bitrate / 1000 * 1.2;
    param.rc.i_vbv_buffer_size = bitrate / 1000;

    param.i_fps_num = fps;
    param.i_fps_den = 1;
    param.i_timebase_den = param.i_fps_num;
    param.i_timebase_num = param.i_fps_den;

    param.b_vfr_input = 0;

    param.i_keyint_max = fps * 2;
    param.b_repeat_headers = 1;

    param.i_threads = 1;
    x264_param_apply_profile(&param, "baseline");
    videoCodec = x264_encoder_open(&param);
    pic_in = new x264_picture_t;
    x264_picture_alloc(pic_in, X264_CSP_I420, width, height);
}

void VideoChannel::setVideoCallback(VideoCallback  videoCallback) {
    this->videoCallback = videoCallback;
}

void VideoChannel::encodeData(int8_t *data) {
    memcpy(pic_in->img.plane[0], data, ySize);
    for (int i = 0; i < uvSize; ++i) {
        *(pic_in->img.plane[1] + i) = *(data + ySize + i * 2 + 1);
        *(pic_in->img.plane[2] + i) = *(data + ySize + i * 2);
    }
    x264_nal_t *pp_nal;
    int pi_nal;
    int sps_len;
    int pps_len;
    uint8_t  sps[100];
    uint8_t  pps[100];
    x264_picture_t pic_out ;
    x264_encoder_encode(videoCodec,&pp_nal,&pi_nal,pic_in,&pic_out);
    for (int i = 0; i <pi_nal ; ++i) {
       if(pp_nal[i].i_type == NAL_SPS){
          sps_len= pp_nal[i].i_payload-4;
          LOGV("sps_len = %d",sps_len);
          memcpy(sps,pp_nal[i].p_payload+4,sps_len);
       } else if(pp_nal[i].i_type==NAL_PPS){
           pps_len = pp_nal[i].i_payload-4;
           LOGV("pps_len = %d",pps_len);
           memcpy(pps,pp_nal[i].p_payload+4,pps_len);
           sendSpsPps(sps,pps,sps_len,pps_len);
       } else{
           sendFrame(pp_nal[i].i_type, pp_nal[i].p_payload, pp_nal[i].i_payload);
       }
    }
}

void VideoChannel::sendFrame(int type, uint8_t *payload, int i_payload) {
    if (payload[2] == 0x00) {
        i_payload -= 4;
        payload += 4;
    } else {
        i_payload -= 3;
        payload += 3;
    }
    //看表
    int bodySize = 9 + i_payload;
    RTMPPacket *packet = new RTMPPacket;
    //
    RTMPPacket_Alloc(packet, bodySize);

    packet->m_body[0] = 0x27;
    if(type == NAL_SLICE_IDR){
        packet->m_body[0] = 0x17;
        LOGV("关键帧");
    }
    //类型
    packet->m_body[1] = 0x01;
    //时间戳
    packet->m_body[2] = 0x00;
    packet->m_body[3] = 0x00;
    packet->m_body[4] = 0x00;
    //数据长度 int 4个字节
    packet->m_body[5] = (i_payload >> 24) & 0xff;
    packet->m_body[6] = (i_payload >> 16) & 0xff;
    packet->m_body[7] = (i_payload >> 8) & 0xff;
    packet->m_body[8] = (i_payload) & 0xff;

    //图片数据
    memcpy(&packet->m_body[9], payload, i_payload);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = bodySize;
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nChannel = 0x10;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    videoCallback(packet);
}

// 格式    RTMP
void VideoChannel::sendSpsPps(uint8_t *sps, uint8_t *pps, int sps_len, int pps_len) {
//    sps, pps  --->packet
    int bodySize = 13 + sps_len + 3 + pps_len;
    RTMPPacket *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, bodySize);

    int i = 0;
    //固定头
    packet->m_body[i++] = 0x17;
    //类型
    packet->m_body[i++] = 0x00;
    //composition time 0x000000
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;

    //版本
    packet->m_body[i++] = 0x01;
    //编码规格
    packet->m_body[i++] = sps[1];
    packet->m_body[i++] = sps[2];
    packet->m_body[i++] = sps[3];
    packet->m_body[i++] = 0xFF;

    //整个sps
    packet->m_body[i++] = 0xE1;
    //sps长度
    packet->m_body[i++] = (sps_len >> 8) & 0xff;
    packet->m_body[i++] = sps_len & 0xff;
    memcpy(&packet->m_body[i], sps, sps_len);
    i += sps_len;

    //pps
    packet->m_body[i++] = 0x01;
    packet->m_body[i++] = (pps_len >> 8) & 0xff;
    packet->m_body[i++] = (pps_len) & 0xff;
    memcpy(&packet->m_body[i], pps, pps_len);

    //视频
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = bodySize;
    //随意分配一个管道（尽量避开rtmp.c中使用的）
    packet->m_nChannel = 10;
    //sps pps没有时间戳
    packet->m_nTimeStamp = 0;
    //不使用绝对时间
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;

    videoCallback(packet);
}

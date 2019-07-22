package com.gg.demo.rtmppush;

import android.app.Activity;
import android.view.SurfaceHolder;

import com.gg.demo.rtmppush.meida.AudioChannel;
import com.gg.demo.rtmppush.meida.VideoChannel;


public class LivePusher {
    private AudioChannel audioChannel;
    private VideoChannel videoChannel;
    static {
        System.loadLibrary("native-lib");
    }

    public LivePusher(Activity activity, int width, int height, int bitrate,
                      int fps, int cameraId) {
        init();
        videoChannel = new VideoChannel(this, activity, width, height, bitrate, fps, cameraId);
        audioChannel = new AudioChannel(this);


    }
    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {
        videoChannel.setPreviewDisplay(surfaceHolder);
    }
    public void switchCamera() {
        videoChannel.switchCamera();
    }


    public native void init();

    public native void native_setVideoEncInfo(int width ,int height ,int fps,int bitrate);

    public void startLive(String s) {
        native_start(s);
        videoChannel.startVideo();
//        audioChannel.start();
    }

    public native void native_start(String path);

    public native void native_pushVideo(byte[] data);

    public native void native_pushAudio(byte[] bytes, int len);

    public native void native_setAudioEncInfo(int i,int channels);

    public native int getInputSamples();

    public native void native_release();

    public void release() {
    }
}

package com.gg.demo.rtmppush.meida;


import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

import com.gg.demo.rtmppush.LivePusher;

import java.nio.ByteBuffer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AudioChannel {
    private LivePusher mLivePusher;
    private AudioRecord audioRecord ;
    private int channelConfig;
    private ExecutorService service = Executors.newSingleThreadExecutor();
    private int channels =2 ;
    private int minBufferSize;
    private boolean isLiving;
    private int inputSample ;

    public AudioChannel(LivePusher livePusher) {
        mLivePusher = livePusher;
        channelConfig= channels==2?AudioFormat.CHANNEL_IN_STEREO:AudioFormat.CHANNEL_IN_MONO;
        inputSample = mLivePusher.getInputSamples()*2;
        minBufferSize = AudioRecord.getMinBufferSize(4410,channelConfig,AudioFormat.ENCODING_PCM_16BIT)*2;
        this.audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC,44100, channelConfig,AudioFormat.ENCODING_PCM_16BIT,minBufferSize<inputSample?inputSample:minBufferSize);

    }

    public void setChannels(int channels) {
        this.channels = channels;
    }

    public void start(){
        isLiving = true;
        mLivePusher.native_setAudioEncInfo(44100,channels);
        service.execute(new AudioTask());
    }


    class AudioTask implements Runnable{
        @Override
        public void run() {
            audioRecord.startRecording();
            byte[]bytes =ByteBuffer.allocateDirect(inputSample).array();
            while (isLiving){
                int len = audioRecord.read(bytes,0,bytes.length);
                mLivePusher.native_pushAudio(bytes,len);
            }
        }
    }

}

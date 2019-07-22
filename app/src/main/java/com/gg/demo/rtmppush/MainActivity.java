package com.gg.demo.rtmppush;

import android.Manifest;
import android.content.Context;
import android.hardware.Camera;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {


    private SurfaceView surfaceView;
    private LivePusher livePusher;

    @RequiresApi(api = Build.VERSION_CODES.M)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        requestPermissions(new String[]{Manifest.permission.CAMERA,Manifest.permission.RECORD_AUDIO}, 101);
        livePusher = new LivePusher(this,800,480,800_000,10, Camera.CameraInfo.CAMERA_FACING_BACK);
        // Example of a call to a native method
        surfaceView = findViewById(R.id.sample_text);
        livePusher.setPreviewDisplay(surfaceView.getHolder());
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        livePusher.release();
    }


    public void start(View view) {
        livePusher.startLive("rtmp://47.240.29.24/myapp");
    }

    public void stop(View view){
            livePusher.release();
    }
    public void switchSource(View view){

    }
}

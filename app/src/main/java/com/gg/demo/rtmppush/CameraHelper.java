package com.gg.demo.rtmppush;

import android.content.Context;
import android.hardware.Camera;
import android.view.SurfaceHolder;

import java.io.IOException;

public class CameraHelper {

    private Camera camera;


    public CameraHelper(Context context) {


    }

    public void open() {
        int defaultId = 0;

        Camera.CameraInfo info = new Camera.CameraInfo();
        for (int i = 0; i < Camera.getNumberOfCameras(); i++) {
            Camera.getCameraInfo(1, info);
            if (info.facing == Camera.CameraInfo.CAMERA_FACING_BACK) {
                defaultId = i;
            }
        }
        try {
            camera = Camera.open(defaultId);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void setCameraDisplay(SurfaceHolder surfaceHolder) {
        if (camera != null) {
            try {
                camera.setPreviewDisplay(surfaceHolder);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        camera.setDisplayOrientation(90);
        camera.startPreview();
        camera.setPreviewCallback(new Camera.PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] data, Camera camera) {
                
            }
        });
    }

    public void stop() {
        if (camera != null)
            camera.stopPreview();
    }

    public void release() {
        if (camera != null)
            camera.release();

    }

}

#pragma once
#include "Camera.h"
#include <OpenNI.h>

class ONICamera : public Camera {
public:
    ONICamera() {
    }
    bool Start() override {
        rc = openni::OpenNI::initialize();
        if (rc != openni::STATUS_OK) {
            printf("Initialize failed\n%s\n", openni::OpenNI::getExtendedError());
            return false;
        }

        rc = device.open(openni::ANY_DEVICE);
        if (rc != openni::STATUS_OK) {
            printf("Couldn't open device\n%s\n", openni::OpenNI::getExtendedError());
            return false;
        }

        auto sensorInfo = device.getSensorInfo(openni::SENSOR_DEPTH);
        auto desiredConfig = sensorInfo->getSupportedVideoModes()[0];
        width = desiredConfig.getResolutionX();
        height = desiredConfig.getResolutionY();
        if (sensorInfo != NULL) {

            rc = depth.create(device, openni::SENSOR_DEPTH);
            rc = depth.setVideoMode(desiredConfig);
            if (rc != openni::STATUS_OK) {
                printf("Couldn't create depth stream\n%s\n", openni::OpenNI::getExtendedError());
                return false;
            }
        }

        rc = depth.start();
        if (rc != openni::STATUS_OK) {
            printf("Couldn't start the depth stream\n%s\n", openni::OpenNI::getExtendedError());
            return false;
        }
        auto hfov = depth.getHorizontalFieldOfView();
        auto vfov = depth.getVerticalFieldOfView();

        fx = (width / 2.0f) / tan(hfov / 2.0f);
        fy = (height / 2.0f) / tan(vfov / 2.0f);
		px = width / 2.0f;
		py = height / 2.0f;
        return true;
    }
    bool syncNext() override {
        int changedStreamDummy;
        openni::VideoStream *pStream = &depth;
        rc = openni::OpenNI::waitForAnyStream(&pStream, 1, &changedStreamDummy, 2000);
        if (rc != openni::STATUS_OK) {
            printf("Wait failed! (timeout is %d ms)\n%s\n", 2000, openni::OpenNI::getExtendedError());
            return false;
        }

        rc = depth.readFrame(&frame);
        if (rc != openni::STATUS_OK) {
            printf("Read failed!\n%s\n", openni::OpenNI::getExtendedError());
            return false;
        }

        if (frame.getVideoMode().getPixelFormat() != openni::PIXEL_FORMAT_DEPTH_1_MM) {
            printf("Unexpected frame format\n");
            return false;
        }
        return true;
    }
    uint16_t *getDepth() override {
        return (uint16_t *)frame.getData();
    }
    uint16_t *getRGB() override {
        return nullptr;
    }
    ~ONICamera() {
        depth.stop();
        depth.destroy();
        device.close();
        openni::OpenNI::shutdown();
    }

  private:
    openni::Status rc;
    openni::Device device;
    openni::VideoStream depth;
    openni::VideoFrameRef frame;
};

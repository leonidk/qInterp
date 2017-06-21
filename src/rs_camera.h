#pragma once
#include "Camera.h"
#include "../librealsense/include/librealsense/rs.hpp"
#include "../librealsense/include/librealsense/rsutil.h"

class RSCamera : public Camera {
public:
    RSCamera() {

    }
    bool Start() override {
        if (ctx.get_device_count() == 0) { printf("No device detected. Is it plugged in?\n"); return false; }
        dev = ctx.get_device(0);

        try {
            dev->enable_stream(rs::stream::depth, 480, 360, rs::format::z16, 30);
            //dev->enable_stream(rs::stream::color, 640, 480, rs::format::rgb8, 30);
            dev->enable_stream(rs::stream::infrared, 492, 372, rs::format::y16, 30);
            dev->enable_stream(rs::stream::infrared2, 492, 372, rs::format::y16, 30);

            // Start our device
            dev->start();

            dev->set_option(rs::option::r200_emitter_enabled, true);
            dev->set_option(rs::option::r200_lr_auto_exposure_enabled, true);
        }
        catch (...) {
            return false;
        }
        auto intrin = dev->get_stream_intrinsics(rs::stream::depth);
        auto extrin = dev->get_extrinsics(rs::stream::infrared2, rs::stream::infrared);
        this->width = intrin.width;
        this->height = intrin.height;
        this->fx = intrin.fx;
        this->fy = intrin.fy;
        this->px = intrin.ppx;
        this->py = intrin.ppy;
        disp_factor = intrin.fx*abs(1000.0f*extrin.translation[0]);
        rs_apply_depth_control_preset((rs_device*)dev, 4);
        auto et = dev->get_extrinsics(rs::stream::infrared, rs::stream::infrared2);
        // just because
        for (int i = 0; i < 10; i++)
            dev->wait_for_frames();


        return true;
    }
    bool syncNext() override {
        dev->wait_for_frames();
        return true;
    }
    uint16_t *getDepth() override {
        return (uint16_t *)dev->get_frame_data(rs::stream::depth);
    }
    uint16_t *getRGB() override {
        return nullptr;
    }
    ~RSCamera() {
        if (dev != nullptr)
            dev->stop();
    }
    float disp_factor = 8000.f*35;
  private:
      rs::context ctx;
      rs::device *dev = nullptr;
};

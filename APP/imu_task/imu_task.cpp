#include "imu_task.hpp"

extern "C" {
#include "cmsis_os.h"
}

#include "debug_task.hpp"
#include "bmi088_yaw_imu.hpp"

using yuntai::imu::Bmi088YawImu;

namespace {

constexpr uint32_t kRetryDelayMs = 100U;
constexpr uint32_t kLoopDelayMs = 2U;
Bmi088YawImu g_imu;

}  // namespace

extern "C" void appImuTask(void *argument) {
    (void)argument;

    while (!g_imu.init()) {
        osDelay(kRetryDelayMs);
    }

    while (!g_imu.calibrate()) {
        osDelay(kRetryDelayMs);
    }

    for (;;) {
        g_imu.update();
        g_ozone_imu_ready = g_imu.state().initialized;
        g_ozone_imu_bias_ready = g_imu.state().bias_ready;
        g_ozone_yaw_deg = g_imu.yawDeg();
        g_ozone_gyro_z_dps = g_imu.gyroZDps();
        g_ozone_accel_z_mps2 = g_imu.accelZMps2();
        g_ozone_accel_z_g = g_imu.accelZG();
        osDelay(kLoopDelayMs);
    }
}

extern "C" bool appImuReady(void) {
    return g_imu.state().initialized && g_imu.state().bias_ready;
}

extern "C" float appImuYawDeg(void) {
    return g_imu.state().yaw_deg;
}

extern "C" float appImuGyroZDps(void) {
    return g_imu.state().gyro_z_dps;
}

extern "C" float appImuAccelZMps2(void) {
    return g_imu.state().accel_z_mps2;
}

extern "C" float appImuAccelZG(void) {
    return g_imu.state().accel_z_g;
}

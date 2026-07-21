#include "debug_task.hpp"

#include "cmsis_os.h"

extern "C" {
volatile bool g_ozone_imu_ready = false;
volatile bool g_ozone_imu_bias_ready = false;
volatile float g_ozone_yaw_deg = 0.0f;
volatile float g_ozone_gyro_z_dps = 0.0f;
volatile float g_ozone_accel_z_mps2 = 0.0f;
volatile float g_ozone_accel_z_g = 0.0f;
}

extern "C" void appDebugTask(void *argument) {
    (void)argument;

    for (;;) {
        osDelay(1000U);
    }
}

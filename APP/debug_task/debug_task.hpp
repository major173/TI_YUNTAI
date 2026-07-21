#ifndef APP_DEBUG_TASK_HPP
#define APP_DEBUG_TASK_HPP

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

extern volatile bool g_ozone_imu_ready;
extern volatile bool g_ozone_imu_bias_ready;
extern volatile float g_ozone_yaw_deg;
extern volatile float g_ozone_gyro_z_dps;
extern volatile float g_ozone_accel_z_mps2;
extern volatile float g_ozone_accel_z_g;

void appDebugTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif

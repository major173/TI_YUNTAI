#ifndef APP_IMU_TASK_HPP
#define APP_IMU_TASK_HPP

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void appImuTask(void *argument);
bool appImuReady(void);
float appImuYawDeg(void);
float appImuGyroZDps(void);
float appImuAccelZMps2(void);
float appImuAccelZG(void);

#ifdef __cplusplus
}
#endif

#endif

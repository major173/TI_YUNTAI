#include "bmi088_yaw_imu.hpp"

extern "C" {
#include "BMI088driver.h"
#include "cmsis_os.h"
#include "main.h"
}

namespace yuntai::imu {

namespace {

constexpr float kRadToDeg = 57.29577951308232f;
constexpr float kGravity = 9.80665f;
constexpr float kMinDtSec = 0.001f;
constexpr float kMaxDtSec = 0.02f;

}  // namespace

bool Bmi088YawImu::init() {
    if (BMI088_init() != BMI088_NO_ERROR) {
        state_.initialized = false;
        return false;
    }

    state_ = {};
    state_.initialized = true;
    state_.last_tick_ms = HAL_GetTick();
    return true;
}

bool Bmi088YawImu::calibrate(uint16_t sample_count, uint32_t sample_delay_ms) {
    float gyro[3] = {0.0f};
    float accel[3] = {0.0f};
    float temperature = 0.0f;
    float gyro_z_sum_dps = 0.0f;

    if (!state_.initialized || sample_count == 0U) {
        return false;
    }

    for (uint16_t i = 0U; i < sample_count; ++i) {
        BMI088_read(gyro, accel, &temperature);
        gyro_z_sum_dps += gyro[2] * kRadToDeg;
        osDelay(sample_delay_ms);
    }

    state_.gyro_z_bias_dps = gyro_z_sum_dps / static_cast<float>(sample_count);
    state_.gyro_z_dps = 0.0f;
    state_.yaw_deg = 0.0f;
    state_.accel_z_mps2 = 0.0f;
    state_.accel_z_g = 0.0f;
    state_.bias_ready = true;
    state_.last_tick_ms = HAL_GetTick();
    return true;
}

bool Bmi088YawImu::update() {
    float gyro[3] = {0.0f};
    float accel[3] = {0.0f};
    float temperature = 0.0f;
    uint32_t now_tick_ms;
    float dt_sec;

    if (!state_.initialized || !state_.bias_ready) {
        return false;
    }

    BMI088_read(gyro, accel, &temperature);

    now_tick_ms = HAL_GetTick();
    dt_sec = static_cast<float>(now_tick_ms - state_.last_tick_ms) * 0.001f;
    state_.last_tick_ms = now_tick_ms;

    if (dt_sec < kMinDtSec) {
        dt_sec = kMinDtSec;
    } else if (dt_sec > kMaxDtSec) {
        dt_sec = kMaxDtSec;
    }

    state_.gyro_z_dps = gyro[2] * kRadToDeg - state_.gyro_z_bias_dps;
    state_.accel_z_mps2 = accel[2];
    state_.accel_z_g = state_.accel_z_mps2 / kGravity;
    state_.yaw_deg = wrapDegrees(state_.yaw_deg + state_.gyro_z_dps * dt_sec);
    return true;
}

void Bmi088YawImu::resetYaw(float yaw_deg) {
    state_.yaw_deg = wrapDegrees(yaw_deg);
    state_.last_tick_ms = HAL_GetTick();
}

const YawImuState &Bmi088YawImu::state() const {
    return state_;
}

float Bmi088YawImu::yawDeg() const {
    return state_.yaw_deg;
}

float Bmi088YawImu::gyroZDps() const {
    return state_.gyro_z_dps;
}

float Bmi088YawImu::accelZMps2() const {
    return state_.accel_z_mps2;
}

float Bmi088YawImu::accelZG() const {
    return state_.accel_z_g;
}

float Bmi088YawImu::wrapDegrees(float angle_deg) {
    while (angle_deg > 180.0f) {
        angle_deg -= 360.0f;
    }
    while (angle_deg < -180.0f) {
        angle_deg += 360.0f;
    }
    return angle_deg;
}

}  // namespace yuntai::imu

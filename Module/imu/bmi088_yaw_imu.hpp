#ifndef BMI088_YAW_IMU_HPP
#define BMI088_YAW_IMU_HPP

#include <cstdint>

namespace yuntai::imu {

struct YawImuState {
    bool initialized{false};
    bool bias_ready{false};
    float yaw_deg{0.0f};
    float gyro_z_dps{0.0f};
    float gyro_z_bias_dps{0.0f};
    float accel_z_mps2{0.0f};
    float accel_z_g{0.0f};
    uint32_t last_tick_ms{0U};
};

class Bmi088YawImu {
public:
    bool init();
    bool calibrate(uint16_t sample_count = 500U, uint32_t sample_delay_ms = 2U);
    bool update();

    void resetYaw(float yaw_deg = 0.0f);
    const YawImuState &state() const;

    float yawDeg() const;
    float gyroZDps() const;
    float accelZMps2() const;
    float accelZG() const;

private:
    static float wrapDegrees(float angle_deg);

    YawImuState state_{};
};

}  // namespace yuntai::imu

#endif

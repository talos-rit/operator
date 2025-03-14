#pragma once

#include "dac/PCA9685PW.h"
#include "enc/encoder.h"
#include "gpio/isr.h"

class Motor
{
public:
    enum DriveType
    {
        DEAD,       // No current is being driven through the motors
        DEAD_BRAKE, // Motor is bleeding off back EMF
        DRIVE,      // Motor is active
        BRAKE,      // Motor is braking
        DRIVE_LEN,
    };

private:
    PCA9685PW* dac;                         // DAC used for toggling motor driver
    uint8_t in1_pin, in2_pin, speed_pin;    // TB6612FNG used for motor driver (outputs)
    uint8_t last_speed;
    DriveType type;

public:

    Motor(PCA9685PW* dac, uint8_t in1_pin, uint8_t in2_pin, uint8_t speed_pin);
    ~Motor();

    int RegisterDriver(PCA9685PW* dac);

    int SetDrive(DriveType type);

    /**
     * @param speed Valid range: [-4095, 4095]
     * @returns 0 on success, -1 on failure
     */
    int SetSpeed(int16_t speed);

    /**
     * @returns a value in the valid speed range [-4095, 4095] on success, 2^13 on failure
     */
    int32_t GetSpeed();

    /**
     * Note: queues, but does not flush buffer (Not fit for emergency)
     */
    int Kill();
};
#pragma once

#include "motor/motor.h"
#include "enc/encoder.h"
#include "dac/PCA9685PW.h"
#include "gpio/isr.h"

#define DRIVER_MAX_SPEED 100

class Driver
{
private:
    // Outputs
    Motor motor;
    // Positional input
    IchorISR* isr;
    RotaryEncoder enc;
    uint8_t enc_a, enc_b;
    // Current monitoring
    // TODO: Add ADC
    void* adc;
    uint8_t adc_channel;
    // Tracking
    int32_t target;
    int32_t last_velocity;
    uint8_t speed;
    int16_t (*v_func)(int32_t target, int32_t pos);


public:
    Driver(PCA9685PW* dac, uint8_t in1, uint8_t in2, uint8_t speed,
           IchorISR* isr, uint8_t enc_a, uint8_t enc_b,
           void* adc, uint8_t adc_channel);
    ~Driver();

    int SetTarget(int32_t target);
    int SetSpeedCoefficient(uint8_t speed);

    // Could this be replaced with a more automatable? (high degree polynomials? splines?)
    /**
     * @details func must represent a continous function, with roots at 0 and target
     */
    int SetVelocityFunc(int16_t (*func)(int32_t target, int32_t pos));
    int Abort();
    int Shutdown();
    int Poll();
};
#pragma once

#include "enc/encoder.h"

typedef union _gpio_target
{
    RotaryEncoder* enc;
    void (*callback)(void);
} GPIO_INTR_TARGET;

typedef enum _gpio_intr_type
{
    GPIO_INTR_TYPE_UNINIT,
    GPIO_INTR_TYPE_ENCODER_A,
    GPIO_INTR_TYPE_ENCODER_B,
    GPIO_INTR_TYPE_HOMING,
    GPIO_INTR_TYPE_E_STOP,
    GPIO_INTR_TYPE_LEN,
} GPIO_INTR_TYPE;

/**
 * Depending on the value stored in type, either target or callback will be used
 */
typedef struct _gpio_intr
{
    GPIO_INTR_TYPE type;    /** Describes the type of callback function */
    GPIO_INTR_TARGET target;           /** Object to call callback function on */
} GPIO_Interrupt;
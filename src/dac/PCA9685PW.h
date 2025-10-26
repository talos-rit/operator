#pragma once

#include "data/s_list.h"
#include "serial/i2c_dev.h"
#include "serial/serial.h"

#define DAC_PCA_MISC_LEN 256
#define DAC_PCA_FRAME_LEN 32  // first 16 are reserved for the 16 PWM channels
#define DAC_PCA_QUEUE_LEN 256
#define DAC_PCA_CHANNEL_COUNT 16

// Completely on value
#define DAC_PCA_MAX_DUTY_CYCLE 4096

#define DAC_PCA_ALL_CALL 0

class PCA9685PW {
 private:
  struct __attribute__((packed)) Channel {
    uint8_t ON_L;
    uint8_t ON_H;
    uint8_t OFF_L;
    uint8_t OFF_H;
  };

  struct __attribute__((packed)) PCA_Regs {
    // Registers 0x00 - 0x45
    uint8_t MODE[2];
// Mode 1
#define DAC_PCA_FLAG_MODE1_ALLCALL _BITULL(0)
#define DAC_PCA_FLAG_MODE1_SUB3 _BITULL(1)
#define DAC_PCA_FLAG_MODE1_SUB2 _BITULL(2)
#define DAC_PCA_FLAG_MODE1_SUB1 _BITULL(3)
#define DAC_PCA_FLAG_MODE1_SLEEP _BITULL(4)
#define DAC_PCA_FLAG_MODE1_AI _BITULL(5)
#define DAC_PCA_FLAG_MODE1_EXTCLK _BITULL(6)
#define DAC_PCA_FLAG_MODE1_RESTART _BITULL(7)
// Mode 2
#define DAC_PCA_FLAG_MODE2_OUTNE0 _BITULL(0)
#define DAC_PCA_FLAG_MODE2_OUTNE1 _BITULL(1)
#define DAC_PCA_FLAG_MODE2_OUTDRV _BITULL(2)
#define DAC_PCA_FLAG_MODE2_OCH _BITULL(3)
#define DAC_PCA_FLAG_MODE2_INVRT _BITULL(4)
    uint8_t SUBADDR[3];
    uint8_t ALLCALLADR;
    Channel LEDs[16];

    // Registers 0x46 - 0xF9 are reserved
    // uint8_t RESERVED[0xF9 - 0x46];

    // Registers 0xFA - 0xFF
    Channel ALL_LED;
    uint8_t PRE_SCALE;
    uint8_t TestMode;
  };

  int fd;
  I2CDev dev;       // I2C bus to send values to
  PCA_Regs regs;    // Internal representation of registers
  PCA_Regs staged;  // Uncommited changes to push to the device
  SerialDevice::SerialFrame*
      frames;  // Frame pool for constructing I2C transactions
  uint16_t frames_len;
  S_List free_queue;                 // Pop free frames from here
  S_List queues[DAC_PCA_QUEUE_LEN];  // Enqueue frames for processing here
  uint8_t queue_iter;                // Amount of active queues; resets on flush
  uint8_t misc[DAC_PCA_MISC_LEN];    // Used for miscellaneous data (e.g.
                                     // transmitting a control register without
                                     // dynamically allocating a single byte)
  uint8_t misc_iter;                 // Iter; Misc cleared every flush
  bool inited;  // Tracks whether the physical device has been initialized

  int prep_queue_transaction(uint8_t addr, uint8_t len, bool read);

  /**
   * @brief Queue a read operation
   */
  int QueueRD(uint8_t addr, uint8_t len);

  /**
   * @brief Queue a write operation
   */
  int QueueWR(uint8_t addr, uint8_t len);

 public:
  /**
   * @brief Init new DAC
   * @param fd File descriptor of I2C bus
   * @param addr I2C address of DAC
   */
  PCA9685PW(int fd = -1, int addr = 0);
  ~PCA9685PW();

  /**
   * @brief Initializes the physical device
   */
  int InitDevice();

  int ResetDevice();

  /**
   * @brief Queues a read operation on all registers in the DAC.
   * Once complete, local register representation will be updated.
   */
  int UpdateRegisters();

  /**
   * @brief Queues setting the duty cycle of a given channel
   * @details Value gets copied to local copy of register values
   * @param channel Channel to write to
   * @param duty_cycle Pointer to duty cycle value to set
   * @returns 0 on success, -1 on failure
   */
  int SetDutyCycle(uint8_t channel, uint16_t duty_cycle);

  /**
   * @brief Reads the value of the register values from the local registers
   * @details Valid range of values is [0, 4095]
   * @param channel Channel to write to
   * @param duty_cycle Pointer to duty cycle value destination
   * @returns -1 on failure, a number between 0 and 4096 on success
   */
  int GetDutyCycle(uint8_t channel);

  int ClearQueues();

  /**
   * @brief Flushes queue of commands to serial bus
   * @returns 0 on success, -1 on failure
   */
  int FlushQueues();
};
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

#include "gpio/isr.h"

#include "util/comm.h"
#include "log/log.h"

#define LOG_CONSOLE_THRESHOLD_THIS  LOG_THRESHOLD_DEFAULT
#define LOG_FILE_THRESHOLD_THIS     LOG_THRESHOLD_MAX

IchorISR::IchorISR(const char* chip_path)
{
    memset(&pin_map[0], 0, ISR_PIN_MAP_LEN * sizeof(GPIO_Interrupt));
    req.fd = -1;
    chip_fd = -1;

    chip_fd = open(chip_path, O_RDONLY);
    if (-1 == chip_fd) STD_FAIL_VOID;
}

IchorISR::~IchorISR()
{
    closeISR();
}

int IchorISR::openISR()
{
    // if ISR is already open, fail
    if (-1 != req.fd) STD_FAIL;

    // Iterate through map, adding valid pins to config
    // init request
    memset(&req, 0, sizeof(gpio_v2_line_request));

    // populate offsets
    uint8_t offset_iter = 0;
    for (uint8_t pin_iter = 0; pin_iter < ISR_PIN_MAP_LEN; pin_iter++)
    {
        if (GPIO_INTR_TYPE_UNINIT != pin_map[pin_iter].type)
            req.offsets[offset_iter++] = pin_iter;
    }
    req.num_lines = offset_iter;
    if (!req.num_lines) return 0;

    // setup request config (applies to all lines in request)
    req.config.flags =  0                                       |
                        GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN        |
                        GPIO_V2_LINE_FLAG_INPUT                 |
                        GPIO_V2_LINE_FLAG_EDGE_FALLING          |
                        GPIO_V2_LINE_FLAG_EDGE_RISING           |
                        // GPIO_V2_LINE_FLAG_EVENT_CLOCK_REALTIME  |
                        0;

    // setup line-by-line attributes
    uint8_t attr_iter = 0;
    #if 0
    gpio_v2_line_config_attribute* debounce = &req.config.attrs[attr_iter++];
    debounce->attr.id = GPIO_V2_LINE_ATTR_ID_DEBOUNCE;
    debounce->attr.debounce_period_us = ISR_INPUT_DEBOUNCE_US;
    debounce->mask = ~0; // Default to all lines; subject to change
    #endif
    // Add more attributes here...

    // set num_attrs after attributes have been populted
    req.config.num_attrs = attr_iter;

    // Set name of request; used in debugging
    snprintf(&req.consumer[0], 32 - 1, "%s", "IchorISR");

    // Set event buffer size (maximum capacity of total line edges in FIFO)
    req.event_buffer_size = GPIO_V2_LINES_MAX * 16; // MAX

    int ret = ioctl(chip_fd, GPIO_V2_GET_LINE_IOCTL, &req);
    if (-1 == ret) STD_FAIL;

    return 0;
}

int IchorISR::closeISR()
{
    if (-1 != req.fd) close(req.fd);
    memset(&req, 0, sizeof(gpio_v2_line_request));
    req.fd = -1;
    return 0;
}

int IchorISR::RegisterPin(uint8_t pin, GPIO_INTR_TYPE type, GPIO_INTR_TARGET target)
{
    pin_map[pin].type = type;
    pin_map[pin].target = target;
    return 0;
}

int IchorISR::UnregisterPin(uint8_t pin)
{
    // zero out pin structure
    memset(&pin_map[pin], 0, sizeof(GPIO_Interrupt));
    return 0;
}

int IchorISR::AllocatePins()
{
    if(closeISR()) STD_FAIL;
    if(openISR())  STD_FAIL;
    return 0;
}

int IchorISR::ExecutePin(uint8_t pin)
{
    GPIO_Interrupt* intr = &pin_map[pin];
    switch (intr->type)
    {
        case GPIO_INTR_TYPE_E_STOP:
            // Implement Emergency stop
            // Intential fallthrough
        case GPIO_INTR_TYPE_HOMING:
            if (!intr->target.callback) STD_FAIL;
            intr->target.callback();
            break;
        case GPIO_INTR_TYPE_ENCODER_A:
            intr->target.enc->ToggleA();
            break;
        case GPIO_INTR_TYPE_ENCODER_B:
            intr->target.enc->ToggleB();
            break;
        default:
            STD_FAIL;
            break;
    }

    return 0;
}

void IchorISR::ProcessEvents()
{
    if (-1 == req.fd) STD_FAIL_VOID;    // Do nothing if no event FIFO is set

    // Setup event structures
    struct gpio_v2_line_event event_data[1024];
    struct pollfd poll_event;
    memset(&poll_event, 0, sizeof(pollfd));
    poll_event.fd = req.fd;
    poll_event.events = POLLIN;

    // Check if there's anything in the event FIFO
    int poll_result = poll(&poll_event, 1, 0); // time out immediately

    // Check result
    if (poll_result == 0) return;               // Event FIFO empty
    if (poll_result < 0) STD_FAIL_VOID;         // Poll returned an error
    if (!(poll_event.revents & POLLIN)) return; // Poll events weren't the target event

    // Poll was good; Read as much as possible out of the FIFO (impossible to tell how many events are in there before hand)
    int read_result = read(poll_event.fd, &event_data, sizeof(event_data));
    if (read_result == -1) STD_FAIL_VOID;

    // Process each event
    uint16_t event_count = read_result / sizeof(gpio_v2_line_event);
    LOG_VERBOSE(4, "ISR EVENT COUNT: %d", event_count);
    for (uint16_t iter = 0; iter < event_count; iter++)
    {
        if (ExecutePin(event_data[iter].offset)) LOG_IEC();
    }
}
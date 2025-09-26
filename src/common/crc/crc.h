#pragma once

#include <stdint.h>

uint16_t CRC16(const uint16_t table[], uint16_t seed, const uint8_t *src, uint32_t num_bytes);
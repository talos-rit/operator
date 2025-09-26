#include <stdlib.h>
#include <stdint.h>

#include "crc/crc.h"

// Modified from the LibCRC
uint16_t CRC16(const uint16_t table[], uint16_t seed, const uint8_t *src, uint32_t num_bytes)
{
	uint16_t crc;
	const uint8_t* ptr;

	crc = seed;
	ptr = src;

	if (ptr) for (uint32_t iter = 0; iter < num_bytes; iter++) {

		crc = (crc >> 8) ^ table[ (crc ^ (uint16_t) *ptr++) & 0x00FF ];
	}

	return crc;
}
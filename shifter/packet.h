#ifndef PACKET_H
#define PACKET_H
#include <stdint.h>


struct header_radiotap {
	uint8_t version;
	uint8_t pad;
	uint16_t len;
	uint32_t present;
};

struct header_radiotap contruct_header_radiotap();

#endif

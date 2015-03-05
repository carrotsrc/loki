#ifndef CAPTURE_H
#define CAPTURE_H
#include <stdint.h>
#include "state.h"

	void *device_capture_start(void*);

	struct macaddr_list_item {
		struct macaddr_list_item *prev, *next;
		uint8_t addr[6];
	};

	struct beacon_frame_item {
		struct beacon_frame_item *prev, *next;
		uint8_t mac[6], ssid_len;
		char *ssid;
		uint64_t count;
	};

	struct proberq_frame_item {
		struct proberq_frame_item *prev, *next;
		uint8_t ssid_len;
		char *ssid;
		uint64_t count;
		struct macaddr_list_item *list, *tail;
	};


	struct frame_log {
		struct b_list {
			uint16_t num;
			int16_t selected;
			struct beacon_frame_item *list, *tail;
		} beacon;

		struct prq_list {
			uint16_t num;
			int16_t selected;
			struct proberq_frame_item *list, *tail;
		} proberq;

		char *lastPacket;
		long long totalPackets;
	};
#endif

#ifndef CAPTURE_H
#define CAPTURE_H
#include <stdint.h>

	int device_capture(const char*);


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
	};

	struct frame_log {
		struct b_list {
			int num;
			struct beacon_frame_item *list, *tail;
		} beacon;

		struct prq_list {
			int num;
			struct proberq_frame_item *list, *tail;
		} proberq;

	};
#endif

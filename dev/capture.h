#ifndef CAPTURE_H
#define CAPTURE_H
#include <stdint.h>

	int device_capture(const char*);


	struct frame_list {
		struct frame_list *prev, *next;
		uint8_t mac[6], ssid_len;
		char *ssid;
		uint64_t count;
	};

	struct frame_log {
		struct beacon_frames {
			int num;
			struct frame_list *list;
			struct frame_list *tail;
		} beacon;
	};
#endif

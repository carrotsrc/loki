#ifndef STATE_H
#define STATE_H
#include <pthread.h>
#include <pcap.h>
#include "view/screen.h"
#include "capture.h"

enum loki_mode {
	OVERVIEW = 0,
	FOCUS_AP,
	FOCUS_STA
};

struct loki_state {
	char *dev;
	struct frame_log *log;
	struct screen_list {
		struct screen *overview;
		struct screen *ap;
		struct screen *sta;
	} screens;

	struct controller_list {
		struct mode_controller *overview;
		struct mode_controller *ap;
		struct mode_controller *sta;
	} controllers;

	struct screen *current;
	struct mode_controller *current_controller;
	char *status_msg;
	pcap_t *handle;

};

void set_status_message(char*, struct loki_state*);

static pthread_mutex_t scrmutex;
#endif

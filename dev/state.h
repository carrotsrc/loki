#ifndef STATE_H
#define STATE_H
#include "view/screen.h"

struct loki_state {
	char *dev;
	void *log;
	struct screen_list {
		struct screen *overview;
		struct screen *ap;
		struct screen *sta;
	} screens;
	struct screen *current;
};

enum Mode {
	overview = 0,
	focus_ap,
	focus_sta
};
#endif

#ifndef STATE_H
#define STATE_H
#include "view/screen.h"

enum loki_mode {
	OVERVIEW = 0,
	FOCUS_AP,
	FOCUS_STA
};

struct loki_state {
	char *dev;
	void *log;
	struct screen_list {
		struct screen *overview;
		struct screen *ap;
		struct screen *sta;
	} screens;
	struct screen *current;
	enum loki_mode mode;
};
#endif

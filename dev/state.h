#ifndef STATE_H
#define STATE_H
#include "view/screen.h"

struct loki_state {
	char *dev;
	void *log;
	struct screen *screen;
};
#endif

#ifndef CONTROLLERS_H
#define CONTROLLERS_H
#include "state.h"
#include "capture.h"

typedef void(*controller_callback)(int,struct frame_log*);

struct controller {
	controller_callback input;
};

struct mode_controller {
	controller_callback input;
	struct controller *left, *centre, *right;
	struct controller *selected;
};

void switch_controller(struct loki_state*, struct controller*);
struct mode_controller *create_mode_controller();
struct controller *create_controller();
#endif

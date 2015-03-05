#ifndef CONTROLLERS_H
#define CONTROLLERS_H
#include "state.h"
#include "capture.h"

typedef (void)(*controller_callback)(int,struct frame_log*);
struct controller {
	controller_callback input;
};

void switch_controller(struct *loki_state, struct controller*);

#endif

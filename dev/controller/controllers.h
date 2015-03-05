#ifndef CONTROLLERS_H
#define CONTROLLERS_H
#include "state.h"
#include "capture.h"

typedef void(*mode_controller_callback)(int,struct loki_state*);
typedef void(*controller_callback)(int,struct frame_log*);

typedef void(*controller_switch)(int,struct frame_log*);

struct controller {
	controller_callback input;
};

struct mode_controller {
	mode_controller_callback input;
	struct controller *left, *centre, *right;
	struct controller *selected;
};

void switch_controller(struct loki_state*, struct controller*);
struct mode_controller *create_mode_controller();
struct controller *create_controller();


void controller_overview_left(int, struct frame_log*);
void controller_overview_centre(int, struct frame_log*);
void controller_overview_right(int, struct frame_log*);

void controller_overview_mode(int, struct loki_state*);
void controller_ap_mode(int, struct loki_state*);
void controller_sta_mode(int, struct loki_state*);
#endif

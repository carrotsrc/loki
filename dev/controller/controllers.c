#include <stdlib.h>
#include "controllers.h"

struct mode_controller *create_mode_controller() {
	struct mode_controller *controller;

	controller = (struct mode_controller*)malloc(sizeof(struct mode_controller));

	return controller;
}

struct controller *create_controller() {
	struct controller *controller;

	controller = (struct controller*)malloc(sizeof(struct controller));

	return controller;
}

void controller_overview_left(int code, struct frame_log *log) {
	printw("Received input - left");
}

void controller_overview_centre(int code, struct frame_log *log) {
	printw("Received input - centre");
}

void controller_overview_right(int code, struct frame_log *log) {
	printw("Received input - right");
}

void controller_overview_mode(int code, struct loki_state *state) {
	switch(code) {

	case 'h':
	case 'l':
		if(state->controllers.overview->selected == state->controllers.overview->centre) {
			state->controllers.overview->selected = state->controllers.overview->right;
		} else {
			state->controllers.overview->selected = state->controllers.overview->centre;
		}
		break;
	case 'j':
	case 'k':
		state->controllers.overview->selected->input(code, state->log);
		break;
	}
}

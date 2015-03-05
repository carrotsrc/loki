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
}

void controller_overview_centre(int code, struct frame_log *log) {
	
	switch(code) {
	case 'j':
		if(log->beacon.selected + 1 < log->beacon.num)
			log->beacon.selected++;
		else
			log->beacon.selected = 0;
		break;
	case 'k':
		if(log->beacon.selected - 1 >= 0)
			log->beacon.selected--;
		else
			log->beacon.selected = log->beacon.num-1;

		break;
	}
}

void controller_overview_right(int code, struct frame_log *log) {
	switch(code) {
	case 'j':
		if(log->proberq.selected + 1 < log->proberq.num)
			log->proberq.selected++;
		else
			log->proberq.selected = 0;
		break;
	case 'k':
		if(log->proberq.selected - 1 >= 0)
			log->proberq.selected--;
		else
			log->proberq.selected = log->proberq.num-1;

		break;
	}
}

void controller_overview_mode(int code, struct loki_state *state) {
	switch(code) {

	case 'h':
	case 'l':
		if(state->controllers.overview->selected == state->controllers.overview->centre) {
			state->controllers.overview->selected = state->controllers.overview->right;
			state->log->beacon.selected = -1;
			state->log->proberq.selected = 0;
		} else {
			state->controllers.overview->selected = state->controllers.overview->centre;
			state->log->beacon.selected = 0;
			state->log->proberq.selected = -1;
		}
		break;
	case 'j':
	case 'k':
		state->controllers.overview->selected->input(code, state->log);
		break;
	}
}

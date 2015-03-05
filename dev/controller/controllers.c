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

void switch_controller(struct loki_state *state, struct controller *c) {
	state->log->beacon.selected = -1;
	state->log->proberq.selected = -1;
}

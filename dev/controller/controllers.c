#include "controllers.h"

void switch_controller(struct loki_state *state, struct controller *c) {
	state->log->beacon.selected = -1;
	state->log->proberq.selected = -1;
}

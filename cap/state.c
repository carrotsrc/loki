#include <stdlib.h>
#include "state.h"

void set_status_message(char *msg, struct loki_state *state) {
	if(state->status_msg != NULL)
		free(state->status_msg);

	state->status_msg = msg;
}

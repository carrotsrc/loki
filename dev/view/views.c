#include "views.h"
#include "capture.h"

void print_overview_centre(struct loki_state *state, WINDOW *handle) {
	wmove(handle, 0 , 0);
	wprintw(handle, "Beacon Frames\n--------------\nTotal: \n\n");
	struct beacon_frame_item *item = ((struct frame_log*)state->log)->beacon.list;
	if(item == NULL)
		return;

	do {
		wprintw(handle, "%s\n", item->ssid);
	} while( (item = item->next) != NULL);
}

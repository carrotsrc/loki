#include "views.h"
#include "capture.h"

void print_overview_left(struct loki_state *state, WINDOW *handle) {
	char *packet = NULL;
	if(( packet = ((struct frame_log*)state->log)->lastPacket) != NULL) {
		wprintw(handle, "%s\n---------\n\n", packet);
		free(packet);
		((struct frame_log*)state->log)->lastPacket = NULL;
	}
}

void print_overview_centre(struct loki_state *state, WINDOW *handle) {
	wmove(handle, 0 , 0);
	wprintw(handle, "Beacon Frames\n--------------\nTotal: \n\n");
	struct beacon_frame_item *item = ((struct frame_log*)state->log)->beacon.list;
	if(item == NULL)
		return;

	do {
		wprintw(handle, "%s\n", item->ssid);
	} while( (item = item->next) != NULL);

	wmove(handle, 2, 8);
	wprintw(handle, "%ld", ((struct frame_log*)state->log)->beacon.num);
}

void print_overview_right(struct loki_state *state, WINDOW *handle) {
	wmove(handle, 0 , 0);
	wprintw(handle, "Probe Requests\n--------------\nTotal: \n\n");
	struct proberq_frame_item *item = ((struct frame_log*)(state->log))->proberq.list;
	if(item == NULL)
		return;

	do {
		wprintw(handle, "%ld\t%s\n", item->count, item->ssid);
	} while( (item = item->next) != NULL);

	wmove(handle, 2, 8);
	wprintw(handle, "%ld", ((struct frame_log*)state->log)->proberq.num);

}

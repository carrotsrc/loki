#ifndef VIEWS_H
#define VIEWS_H
#include "state.h"
typedef void(*callback_view_print)(struct loki_state*,WINDOW*);

struct view {
	uint16_t x, y, w, h;
	WINDOW *port;
	callback_view_print write;
//	void (*write)(struct loki_state*,WINDOW*);
};

struct view *create_view(uint16_t,uint16_t,uint16_t,uint16_t, callback_view_print);

void print_overview_left(struct loki_state*,WINDOW*);
void print_overview_centre(struct loki_state*,WINDOW*);
void print_overview_right(struct loki_state*,WINDOW*);

void print_ap_left(struct loki_state*,WINDOW*);
void print_ap_centre(struct loki_state*,WINDOW*);
void print_ap_right(struct loki_state*,WINDOW*);
#endif

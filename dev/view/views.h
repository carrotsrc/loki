#ifndef VIEWS_H
#define VIEWS_H
#include "state.h"
typedef void(*callback_view_print)(struct loki_state*,WINDOW*);

void print_overview_left(struct loki_state*,WINDOW*);
void print_overview_centre(struct loki_state*,WINDOW*);
void print_overview_right(struct loki_state*,WINDOW*);
#endif

#include "init.h"
/** Construct all the screens and their views that are used in lokicap
 */
void create_screens(struct loki_state *state) {
	struct screen *screen = NULL;
	struct view *vleft = NULL, *vright = NULL, *vcentre = NULL;

	// overview focus
	vleft = create_view(2, 2, 50, LINES-4, &print_overview_left);
	vcentre = create_view(52, 2, 37, LINES-4, &print_overview_centre);
	vright = create_view(89, 2, (COLS/3), LINES-4, &print_overview_right);
	scrollok(vleft->port, TRUE);
	idlok(vleft->port, TRUE);

	screen = create_screen();
	screen->left = vleft; // only use one raw feed
	screen->centre = vcentre;
	screen->right = vright;


	// Access Point focus
	state->current = state->screens.overview = screen;

	vcentre = create_view(52, 2, 37, LINES-4, &print_ap_centre);
	vright = create_view(89, 2, (COLS/3), LINES-4, &print_ap_right);
	screen = create_screen();

	screen->left = vleft; // only use one raw feed
	screen->centre = vcentre;
	screen->right = vright;
	state->screens.ap = screen;

	// Station focus
	vcentre = create_view(52, 2, 37, LINES-4, &print_sta_centre);
	vright = create_view(89, 2, (COLS/3), LINES-4, &print_sta_right);
	screen = create_screen();

	screen->left = vleft; // only use one raw feed
	screen->centre = vcentre;
	screen->right = vright;
	state->screens.sta = screen;
}


void create_controllers(struct loki_state *state) {
	struct mode_controller *controller;
	// Build the overview mode controller
	controller = create_mode_controller();

	controller->left = create_controller();
	controller->left->input = &controller_overview_left;

	controller->centre = create_controller();
	controller->centre->input = &controller_overview_centre;
	
	controller->right = create_controller();
	controller->right->input = &controller_overview_right;

	controller->selected = controller->centre;
	controller->input = &controller_overview_mode;

	state->controllers.overview = controller;

	state->current_controller = state->controllers.overview;

	// Build the AP mode controller
	controller = create_mode_controller();

	controller->left = create_controller();
	controller->left->input = NULL;

	controller->centre = create_controller();
	controller->centre->input = NULL;
	
	controller->right = create_controller();
	controller->right->input = NULL;

	controller->selected = controller->centre;
	controller->input = &controller_ap_mode;

	state->controllers.ap = controller;

	// Build the AP mode controller
	controller = create_mode_controller();

	controller->left = create_controller();
	controller->left->input = NULL;

	controller->centre = create_controller();
	controller->centre->input = NULL;
	
	controller->right = create_controller();
	controller->right->input = NULL;

	controller->selected = controller->centre;
	controller->input = &controller_sta_mode;

	state->controllers.sta = controller;
}

#include "packet.h"
#include <stdlib.h>

struct header_radiotap contruct_header_radiotap() {
	struct header_radiotap *header;
	
	header = (struct header_radiotap*) malloc(sizeof(struct header_radiotap));
}

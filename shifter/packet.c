#include "packet.h"
#include <stdlib.h>
#include <string.h>

struct header_radiotap *construct_header_radiotap() {
	struct header_radiotap *header;
	
	header = (struct header_radiotap*) malloc(sizeof(struct header_radiotap));

	header->version = 0;
	header->pad = 0;
	header->len = sizeof(struct header_radiotap);
	header->present = 0;
	return header;
}

struct header_management *construct_header_management(const uint8_t *ta, const uint8_t *ra, const uint8_t *bssid, const struct mac80211_control *stc) {
	struct header_management *mng = (struct header_management*)malloc(sizeof(struct header_management));
	mng->control = encode_mac80211_control(stc);
	mng->duration_id = 500;
	memcpy(mng->ra, ra, 6);
	memcpy(mng->ta, ta, 6);
	memcpy(mng->bssid, bssid, 6);
	mng->seqctrl = 0;
	return mng;
}
#include <stdio.h>
uint16_t encode_mac80211_control(const struct mac80211_control *stc) {
	uint16_t ctrl;
	ctrl |= stc->order;
	ctrl <<= 1;

	ctrl |= stc->protected;
	ctrl <<= 1;

	ctrl |= stc->data;
	ctrl <<= 1;

	ctrl |= stc->powerman;
	ctrl <<= 1;

	ctrl |= stc->retry;
	ctrl <<= 1;

	ctrl |= stc->frags;
	ctrl <<= 1;

	ctrl |= stc->fromDS;
	ctrl <<= 1;

	ctrl |= stc->toDS;
	ctrl <<= 4;

	ctrl |= stc->subtype;
	ctrl <<= 2;

	ctrl |= stc->type;
	ctrl <<= 2;

	ctrl |= stc->protocol;
	return ctrl;
}

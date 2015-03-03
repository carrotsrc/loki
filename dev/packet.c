#include <stdio.h>
#include <stdlib.h>
#include "packet.h"
void print_mac_address(uint8_t *address) {

	int i = 0;
	while(i < 6) {
		if(i > 0)
			printf(" : ");
		printf("%02x", address[i++]);
	}
}

void printraw_packet(const unsigned char *packet, unsigned int len) {
	unsigned int i = 0, j = 0;
	char *buf, *bptr;
	bptr = buf = malloc((len*3)+(len>>4)+10);

	while(i < len) {
		sprintf(bptr, "%02x ", packet[i++]);
		bptr += 3;
		if(++j == 16) {
			sprintf(bptr++, "\n");
			j = 0;
		}
	}
	printf("%s", buf);
	free(buf);
}

struct mac80211_control *decode_mac80211_control(uint16_t cf) {
	struct mac80211_control *fields = malloc(sizeof(struct mac80211_control));
	
	fields->protocol = cf&0x03;
	cf >>= 2;
	
	fields->type = cf&0x03;
	cf >>= 2;

	fields->subtype = cf&0x0f;
	cf >>= 4;

	fields->toDS = cf&0x01;
	cf >>= 1;

	fields->fromDS = cf&0x01;
	cf >>= 1;

	fields->frags = cf&0x01;
	cf >>= 1;

	fields->retry = cf&0x01;
	cf >>= 1;

	fields->powerman = cf&0x01;
	cf >>= 1;

	fields->data = cf&0x01;
	cf >>= 1;

	fields->protected = cf&0x01;
	cf >>= 1;

	fields->order = cf&0x01;
	cf >>= 1;

	return fields;
}

void printraw_management_frame(const uint8_t *packet, uint16_t len) {
	uint8_t mac_begin, hsize;
	struct mac80211_management_hdr *manhdr = NULL;
	struct mac80211_control *mctrl = NULL;

	mac_begin = ((struct pkth_radiotap*)packet)->len;
	len -= mac_begin;

	// Print radiotap header
	printraw_packet((uint8_t*)packet, mac_begin);
	printf("\n\n");

	manhdr = (struct mac80211_management_hdr*) ((uint8_t*)packet+mac_begin);
	mctrl = decode_mac80211_control(manhdr->control);
	hsize = sizeof(struct mac80211_management_hdr);
	if(mctrl->order == 0) hsize -= 4; // We drop the 4-byte HT field
	len -= hsize;

	// print managament header
	printraw_packet((uint8_t*)manhdr, hsize);
	printf("\n\n");

	// print frame body
	printraw_packet((uint8_t*)manhdr+hsize, len);
	printf("\n\n");


}

#include <stdio.h>
#include <stdlib.h>
#include "packet.h"
static void print_mac_address(uint8_t *address) {

	int i = 0;
	while(i < 6) {
		if(i > 0)
			printf(" : ");
		printf("%02x", address[i++]);
	}
}

void printhdr_mac80211(struct pkth_mac80211 *mac) {
	int i;
	printf("Control:\t%x\n", mac->control);
	printf("duration:\t%d ms\n", mac->duration_id);

	printf("dst:\t");
	print_mac_address(mac->dst);

	printf("\nsrc:\t");
	print_mac_address(mac->src);

	printf("\ntrn:\t");
	print_mac_address(mac->trn);

	printf("\nrcv:\t");
	print_mac_address(mac->rcv);

	printf("\nbssid:\t");
	print_mac_address(mac->bssid);
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

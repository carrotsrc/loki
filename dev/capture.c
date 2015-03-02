#include <pcap.h>
#include <stdlib.h>
#include "packet.h"
/* pcap callback for reading a packet */
static void capture_cb(u_char*, const struct pcap_pkthdr*, const u_char*);

int device_capture(const char *dev) {
	pcap_t *handle = NULL;
	char errbuf[PCAP_ERRBUF_SIZE], *filter = NULL;
	bpf_u_int32 ipaddr, netmask;
	struct bpf_program fp;
	const u_char *packet = NULL;


	if(pcap_lookupnet(dev, &ipaddr, &netmask, errbuf) == -1) {
		fprintf(stderr, "Error on device lookup\n%s\n", errbuf);
		return 1;
	}

	handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
	if(handle == NULL) {
		fprintf(stderr, "Couldn't open device %s", errbuf);
		return 1;
	}

	filter = "broadcast";
	if(pcap_compile(handle, &fp, filter, 0, 0) == -1) {
		fprintf(stderr, "Couldn't compiler filter / %s\n", pcap_geterr(handle));
		return 1;
	}

	if(pcap_setfilter(handle, &fp) == -1) {
		fprintf(stderr, "Couldn't install filter\n", pcap_geterr(handle));
		return 1;
	}

	if(pcap_loop(handle, 0, capture_cb, NULL) == -1) {
		fprintf(stderr, "Error on capture loop\n");
		pcap_close(handle);
		return 1;
	}
	pcap_close(handle);
	return 0;

}


void capture_cb(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
	static frameNo = 1;
	unsigned int eth_begin = 0, sz = 0;
	struct pkth_mac80211_management *mac = NULL;
	struct mac80211_control *mctrl = NULL;
	struct pkth_ethernet *ether = NULL;
	struct pkt_mac80211_fbody *frame_body = NULL;

	eth_begin = ((struct pkth_radiotap*)packet)->len;
	sz = header->len - eth_begin;

	mac = (struct pkth_mac80211_management*) (packet+eth_begin);
	sz -= sizeof(struct pkth_mac80211_management);

	mctrl = decode_mac80211_control(mac->control);


	switch(mctrl->subtype) {
	case BEACON:
		printf("Beacon\n");
		break;

	case PROBE_REQUEST:
		printf("Probe Request\n");
		break;
	default:
		free(mctrl);
		return;
	}
	printf("%d bytes\n\n", header->len);

	short drop = 4;
	if(mctrl->order)
		drop = 0;

	short hsize = sizeof(struct pkth_mac80211_management)-drop;
	frame_body = (struct pkt_mac80211_fbody*) ((uint8_t*)packet + eth_begin + hsize);
	uint8_t *ssid_len = ((uint8_t*)packet + eth_begin + hsize + sizeof(struct pkt_mac80211_fbody)+1);
	char *ssid = (uint8_t*)packet + eth_begin + hsize + sizeof(struct pkt_mac80211_fbody)+2;


	sz -= hsize;

	// Print from start of packet to byte before start of ether frame
	printraw_packet((unsigned char*)packet, eth_begin);
	printf("\n\n");

	// print beginning of ether frame to end of packet
	printraw_packet((unsigned char*)packet+eth_begin, hsize);
	printf("\n\n");

	// print beginning of ether frame to end of packet
	printraw_packet((unsigned char*)packet+eth_begin+hsize, sz);
	printf("\n\n");

	printhdr_mac80211_management(mac);
	int i = 0;
	printf("SSID:\t");
	do {
		printf("%c", *(ssid+i));
	} while ( ++i < *ssid_len);
	printf("\n");
	printf("\n-----------\n\n\n");
}

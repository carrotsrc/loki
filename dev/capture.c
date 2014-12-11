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
	unsigned int eth_begin = 0, sz = 0;
	
	eth_begin = ((struct pkth_radiotap*)packet)->len;
	sz = header->len - eth_begin;

	printraw_packet((unsigned char*)packet, eth_begin);
	printf("\n\n");
	printraw_packet((unsigned char*)packet+eth_begin, sz);
	printf("\n\n");

	printhdr_mac80211((struct pkth_mac80211*)(packet+eth_begin));
	printf("\n-----------\n\n\n");
}

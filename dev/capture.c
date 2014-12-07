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
	int i = 0, j = 0, sz = 0, eth_begin = 0;
	char *buf = NULL, *bp = NULL;
	const u_char *eth = NULL;
	
	eth_begin = ((struct pkth_radiotap*)packet)->len;
	eth = packet + eth_begin;	
	sz = (header->len *3) + (header->len/16) + 10;
	bp = buf = malloc(sz);
	bp = buf;
	sz = header->len - eth_begin;

	while(i < eth_begin) {
		sprintf(bp, "%02x ", packet[i++]);
		bp += 3;
		if(++j == 16 || i == sz) {
			sprintf(bp++, "\n");
			if(i == sz)
				sprintf(bp++, "\n");
			j = 0;
		}
	}

	sprintf(bp++, "\n");
	sprintf(bp++, "\n");
	printf("%s", buf);

	bp = buf;
	struct pkth_mac80211 *mac = (struct pkth_mac80211*)eth;
	printf("Control:\t%04x\n", mac->control);
	printf("duration:\t%d ms\n", mac->control);
	printf("dst:\t");
	while(i < 6) {
		if(i > 0)
			printf(" : ");
		printf("%02x", mac->dst[i++]);
	}

	i = 0;
	printf("\nsrc:\t");
	while(i < 6) {
		if(i > 0)
			printf(" : ");
		printf("%02x", mac->src[i++]);
	}

	i = 0;
	printf("\ntrn:\t");
	while(i < 6) {
		if(i > 0)
			printf(" : ");
		printf("%02x", mac->trn[i++]);
	}

	i = 0;
	printf("\nrcv:\t");
	while(i < 6) {
		if(i > 0)
			printf(" : ");
		printf("%02x", mac->rcv[i++]);
	}

	i = 0;
	printf("\nbssid:\t");
	while(i < 6) {
		if(i > 0)
			printf(" : ");
		printf("%02x", mac->bssid[i++]);
	}
	printf("\n\n\n");
	free(buf);

}

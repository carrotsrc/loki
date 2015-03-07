#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pcap.h>

int main(int argc, char *argv[]) {

	pcap_t *handle = NULL;
	char errbuf[PCAP_ERRBUF_SIZE], *filter = NULL, *dev;
	bpf_u_int32 ipaddr, netmask;
	struct bpf_program fp;

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

	pcap_close(handle);
	return 0;
}

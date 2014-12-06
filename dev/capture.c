#include <pcap.h>
int device_capture(const char *dev) {
	char errbuf[PCAP_ERRBUF_SIZE];
	bpf_u_int32 ipaddr, netmask;


	if(pcap_lookupnet(dev, &ipaddr, &netmask, errbuf) == -1) {
		fprintf(stderr, "Error on device lookup\n%s", errbuf);
		return 1;
	}

	return 0;

}

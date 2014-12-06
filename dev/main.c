#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/genl/genl.h>
#include <netinet/if_ether.h>

#include <pcap.h>

#include "nl80211.h"



int main( int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(stderr, "Device unspecified\n");
		exit(EXIT_FAILURE);
	}

	char *dev = argv[1], errbuf[PCAP_ERRBUF_SIZE];
	struct nl_msg *msg = NULL;
	struct nl_sock *nls = nl_socket_alloc();
	signed long long devid;
	int family, cmd, flags = 0;
	if(!nls) {
		fprintf(stderr, "Failed to allocate socket\n");
		exit(EXIT_FAILURE);
	}

	nl_socket_set_buffer_size(nls, 8192, 8192);
	/* this creates a local socket + binds to GENERIC NETLINK*/
	if(genl_connect(nls)) {
		fprintf(stderr, "Failed to create socket fd\n");
		nl_socket_free(nls);
		exit(EXIT_FAILURE);
	}

	/* the resolvers convert family names into unqiue IDs*/
	family = genl_ctrl_resolve(nls, "nl80211");

	if(family < 0) {
		fprintf(stderr, "Failed to resolve ID for nl80211 family\n");
		nl_socket_free(nls);
		exit(EXIT_FAILURE);
	}


	devid = if_nametoindex(dev);
	printf("Device `%s` ID: %lld\n", dev, devid);

	msg = nlmsg_alloc();

	if(!msg) {
		fprintf(stderr, "Failed to allocate netlink message\n");
		nlmsg_free(msg);
		nl_socket_free(nls);
		exit(EXIT_FAILURE);
	}

	cmd = NL80211_CMD_SET_INTERFACE;

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devid);
	NLA_PUT_U32(msg, NL80211_ATTR_IFTYPE, NL80211_IFTYPE_MONITOR);


	/* port is 0 - send to kernel */
	genlmsg_put(msg, 0, 0, family, 0, flags, cmd, 0);

	
nla_put_failure:
	fprintf(stderr, "Failed to build message\n");
	nlmsg_free(msg);
	nl_socket_free(nls);
	exit(EXIT_FAILURE);

clear_out:
	nlmsg_free(msg);
	nl_socket_free(nls);

	
/*
	//pcap_t *handle = NULL;
	struct bpf_program fp;
	bpf_u_int32 net = 0, mask = 0;
	fptr = fopen("pout", "w");

	struct pcap_pkthdr header;
	const u_char *packet;

	char fexp[] = "broadcast";


	dev = pcap_lookupdev(errbuf);
	if(!dev) {
		fprintf(stderr, "Cannot find default device: %s\n", errbuf);
		exit(EXIT_FAILURE);
	}
	if(argv[1] != NULL)
		dev = argv[1];

	printf("Device: %s\n", dev);
	if(pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
		fprintf(stderr, "Can't get netmask for device %s\n", dev);
		net = mask = 0;
	}

	//if((handle = pcap_open_live(argv[1], BUFSIZ ,1, 1000, errbuf)) == NULL) {
	//	fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
	//	exit(EXIT_FAILURE);
	//}



	if((handle = pcap_create(dev, errbuf)) == NULL) {
		fprintf(stderr, "Couldn't create device %s: %s\n", dev, errbuf);
		exit(EXIT_FAILURE);
	}

	
	if(pcap_can_set_rfmon(handle) < 0) {
		fprintf(stderr, "Cannot set media in Monitor Mode\n", dev);
		exit(EXIT_FAILURE);
	}

	int r = pcap_set_rfmon(handle,1);
	if(r != 0 && r != PCAP_ERROR_ACTIVATED ) {
		fprintf(stderr, "Failed to start monitor mode\n");
		exit(EXIT_FAILURE);
	}


	r = pcap_set_promisc(handle, 1);
	if(r != 0 && r != PCAP_ERROR_ACTIVATED) {
		fprintf(stderr, "Cannot set promiscuous mode\n");
		exit(EXIT_FAILURE);
	}

	r = pcap_set_buffer_size(handle, BUFSIZ);
	if(r != 0 && r != PCAP_ERROR_ACTIVATED) {
		fprintf(stderr, "Cannot set buffer size\n");
		exit(EXIT_FAILURE);
	}

	r = pcap_set_buffer_size(handle, BUFSIZ);
	if(r != 0 && r != PCAP_ERROR_ACTIVATED) {
		fprintf(stderr, "Cannot set buffer size\n");
		exit(EXIT_FAILURE);
	}

	r = pcap_set_timeout(handle, 1000);
	if(r != 0 && r != PCAP_ERROR_ACTIVATED) {
		fprintf(stderr, "Cannot set timeout\n");
		exit(EXIT_FAILURE);
	}

	r = pcap_set_snaplen(handle, 65535);
	if(r != 0 && r != PCAP_ERROR_ACTIVATED) {
		fprintf(stderr, "Cannot set timeout\n");
		exit(EXIT_FAILURE);
	}
		
	r = pcap_activate(handle);
	printf("Activate: %d\n", r);
	switch(r){
	case PCAP_WARNING:
		fprintf(stderr, "Warning occured %d\n", r);
		break;
	case PCAP_ERROR_ACTIVATED:
		fprintf(stderr, "Already active\n");
		break;
	case PCAP_ERROR_NO_SUCH_DEVICE:
	case PCAP_ERROR_PERM_DENIED:
		fprintf(stderr, "Failed to activate\n");
		exit(EXIT_FAILURE);
		break;
	}


//	if(pcap_compile(handle, &fp, fexp, 0, net)) {
//		fprintf(stderr, "Couldn't parse filter %s: %s\n", fexp, pcap_geterr(handle));
//		pcap_close(handle);
//		exit(EXIT_FAILURE);
//	}
//
//	if(pcap_setfilter(handle, &fp) == -1) {
//		fprintf(stderr, "Couldn't set filter: %s\n", pcap_geterr(handle));
//		pcap_close(handle);
//		exit(EXIT_FAILURE);
//	}
	if(pcap_loop(handle, 0, on_sniff, NULL) == -1) {
		fprintf(stderr, "Error occured on loop");
		exit(EXIT_FAILURE);
	}
	fclose(fptr);
	printf("Success\n");
	pcap_close(handle);
*/
	exit(EXIT_SUCCESS);
}

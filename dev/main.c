#include <stdio.h>
#include <stdlib.h>
#include <netinet/if_ether.h>

#include "monitor.h"
#include "capture.h"



/* TODO:
 * this needs to be researched more deeply
 * to see what net-tools is up to...
 */
static int ifconfig_device_up(const char *dev, const char *address) {


	char ifcfg[40];
	if(address == NULL)
		sprintf(ifcfg, "ifconfig %s up", dev);
	else
		sprintf(ifcfg, "ifconfig %s %s up", dev, address);

	printf("$ %s\n", ifcfg);

	return system(ifcfg);
}

static int ifconfig_device_down(const char *dev) {


	char ifcfg[20];
	sprintf(ifcfg, "ifconfig %s down", dev);
	printf("$ %s\n", ifcfg);

	return system(ifcfg);
}


int main( int argc, char *argv[]) {
	char *addr = NULL;
	if(argc < 2) {
		fprintf(stderr, "Device unspecified\n");
		exit(EXIT_FAILURE);
	}

	if(argc >= 3) {
		addr = argv[2];
	}

	char *dev = argv[1];
	int r = 0;

	/* switch on monitor mode using libnl */
	if(set_monitor_mode(dev) > 0) {
		fprintf(stderr, "Error on setting monitor flag\n");
		exit(EXIT_FAILURE);
	}
	/* If we're here then we should be in monitor mode.
	 * 
	 * We don't need an address assigned
	 * since it's in monitor mode but 
	 * sometimes it complains if there isn't
	 * any address.
	 */
	r = ifconfig_device_up(dev, addr);
	if(r != EXIT_SUCCESS) {

		if( r == -1)
			fprintf(stderr, "Error in creating child process\n");
		else if(r == EXIT_FAILURE)
			fprintf(stderr, "Child process EXIT_FAILURE\n");

		exit(EXIT_FAILURE);
	}

	printf("device is up\n", dev);
	/* device is now up */

	device_capture(dev);
	/* bring device down */
	ifconfig_device_down(dev);
	exit(EXIT_SUCCESS);

}

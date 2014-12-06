void on_sniff(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
	int i = 0, j = 0, sz = 0;
	sz = (header->len *3) + (header->len/16) + 3;
	char *buf = malloc(sz);
	char *bp = buf;
	while(i < header->len) {
		sprintf(bp, "%02x ", packet[i++]);
		bp += 3;
		if(++j == 16 || i == sz) {
			sprintf(bp++, "\n");
			if(i == sz)
				sprintf(bp++, "\n");
			j = 0;
		}
	}
	sprintf(bp, "\n\n\n");
	printf("%s", buf);
	fwrite(buf, 1, sz, fptr);
	free(buf);

}

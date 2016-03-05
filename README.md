# Loki

Wireless device security demo

I was asked to present basic security problems with wireless devices and how to be vigilant over their hardware. For the talk I put together a small (controlled) demonstration including this software.

### Loki Capture

Loki Capture is a basic single-channel ncurses wireless network analysis tool:

- Overview displays access points and probe requests
- AP focus reveals basic service set + injection
- Probe request focus shows number of devices issuing requests for AP

Of course, since it is a demo for youngsters, I also added in a raw packet dump on the left

Other points of interest:

- Switching wireless card monitor mode on via netlink
- Raw packet construction

### Loki DNS Spoof

Finally got my hands dirty in python! It seemed like a perfect application of the language.

Working backwards from wireshark's packet and hex dumps (I was cross eyed after a few hours) and with a bit of referencing from some manuals, managed to put together a simple DNS spoof server. Tested and works for the demonstration.

I am positive there is a more elegent solution to applying predefine data structure on a sequence of bytes than that hacked together mess I layed out using list indecies.

### Dependecies

- Python
- libnl
- libpcap
- libcurses

# Loki

Wireless device security demo

#### Background

I have been asked to demonstrate some wireless device security problems as part of a someone else's larger lecture to GCSE students.

I have a functional demonstration worked out but I aim to run as much as possible from custom software, so I understand at least whats happening at the level of the software I would otherwise use in the demonstration. It's taking me on quite a rollercoaster through TAP devices, netlink messaging, 80211 standard, DHCP, packet analysis and much more. It's probably more educational for me than it will be for the teens.

Current Progress: Capture

### Loki DNS Spoof

Finally got my hands dirty in python! It seemed like a perfect application of the language.

Working backwards from wireshark's packet and hex dumps (I was cross eyed after a few hours) and with a bit of referencing from some manuals, managed to put together a simple DNS spoof server. Tested and works for the demonstration.

I am positive there is a more elegent solution to applying predefine data structure on a sequence of bytes than that hacked together mess I layed out using list indecies.


### Loki Capture

**2014/12/06**

This is my current work. I started out using pcap to get packets from the device. I wanted to break it down into the specific functions to bring the device up into monitor mode correctly without using iw; however it was spitting out an error (-6 if memory serves) even though the device *can* be put into the mode.

Since *iw* can set the monitor flag correctly, I looked at how that was doing it.

After working through it's preprocessor (black magick) code for setting up the command handlers, I got into the libnl stuff; using iw and the nl80211.h as a reference, the device is now switched into monitor mode correctly via a netlink message.


## NOTE: ##

This is placed here for *educational* purposes.
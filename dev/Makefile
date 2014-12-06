CC=gcc -ggdb
NLFLAGS:=`pkg-config --cflags --libs libnl-3.0`
CFLAGS=-lpcap -lnl-genl-3 -DCONFIG_LIBNL30
SOURCES := $(shell find ./ -name '*.c')
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))

all: $(OBJECTS)
	$(CC) $(CFLAGS) $(NLFLAGS) $(OBJECTS) -o dev

$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) $(NLFLAGS) -c $< -o $@

clean:
	find . -name "*.o" -type f -delete
	make all

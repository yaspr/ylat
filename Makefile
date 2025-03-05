CC=gcc
CFLAGS=-Wall -Wextra -g
OFLAGS=-Ofast

all: ylat

ylat: ylat.c
	$(CC) $(CFLAGS) $(OFLAGS) $< ../../../ynotif/ynotif.c -o $@

clean:
	rm -Rf ylat

VERSION = 0.4

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g

y50-subwoofer: y50-subwoofer.o
	$(CC) -o $@ y50-subwoofer.o -lasound

clean:
	rm -f y50-subwoofer *.o

dist:
	cd ..; mv y50-subwoofer y50-subwoofer-$(VERSION); \
	tar cfz y50-subwoofer-$(VERSION).tar.gz --exclude='.git*' y50-subwoofer-$(VERSION); \
	mv y50-subwoofer-$(VERSION) y50-subwoofer

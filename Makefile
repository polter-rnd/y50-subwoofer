CC = gcc
CFLAGS = -Wall -Wextra -O2

y50-subwoofer: y50-subwoofer.o
	$(CC) -o $@ y50-subwoofer.o -lasound

clean:
	rm -f y50-subwoofer *.o

install: y50-subwoofer
	install y50-subwoofer /usr/local/bin/y50-subwoofer
	install y50-subwoofer.service /etc/systemd/system
	systemctl enable y50-subwoofer

uninstall:
	systemctl disable y50-subwoofer
	rm -f /etc/systemd/system/y50-subwoofer.service
	rm -f /usr/local/bin/y50-subwoofer

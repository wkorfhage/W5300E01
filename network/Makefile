# Makefile for Network programs.

# build options.
LDFLAGS:= 
EXTRA_LDFLAGS:=
CROSS_COMPILE:=arm-linux-
ARCH:=arm

all: udp

clean:
	rm udp.o net.o net.a

main: main.c w5300.a
	/usr/arm-linux/bin/gcc -I./include -L. -o main main.c w5300.a

udp: udp.c w5300.a
	/usr/arm-linux/bin/gcc -I./include -L. -o udp udp.c w5300.a

gpio.o: gpio.c gpio.h
	/usr/arm-linux/bin/gcc -I ./include -fPIC -g -c -Wall gpio.c

lstring.o: lstring.c include/lstring.h
	/usr/arm-linux/bin/gcc -I ./include -fPIC -g -c -Wall lstring.c

socket.o: socket.c include/socket.h
	/usr/arm-linux/bin/gcc -I ./include -fPIC -g -c -Wall socket.c

serial.o: serial.c include/serial.h
	/usr/arm-linux/bin/gcc -I ./include -fPIC -g -c -Wall serial.c

md5.o: md5.c include/md5.h
	/usr/arm-linux/bin/gcc -I ./include -fPIC -g -c -Wall md5.c

w5300.a: w5300.c include/w5300.h md5.c socket.c 
	/usr/arm-linux/bin/gcc -I ./include -fPIC -g -c -Wall w5300.c
	/usr/arm-linux/bin/ar rcs w5300.a w5300.o md5.o socket.o

net.a: net.c net.h
	/usr/arm-linux/bin/gcc -I ./include -fPIC -g -c -Wall net.c
	/usr/arm-linux/bin/ar rcs net.a net.o


# Makefile for LED toggling program.

# build options.
LDFLAGS:= 
EXTRA_LDFLAGS:=
CROSS_COMPILE:=arm-linux-
ARCH:=arm

all: led

clean:
	rm led.o gpio.o gpio.a

led: led.c gpio.a
	/usr/arm-linux/bin/gcc -L. -o led led.c gpio.a

gpio.a: gpio.c gpio.h
	/usr/arm-linux/bin/gcc -fPIC -g -c -Wall gpio.c
	/usr/arm-linux/bin/ar rcs gpio.a gpio.o


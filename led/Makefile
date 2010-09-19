# Makefile for LED toggling program.

# build options.
LDFLAGS:= 
EXTRA_LDFLAGS:=
CROSS_COMPILE:=arm-linux-
ARCH:=arm

led: led.c
	/usr/arm-linux/bin/gcc -L. -o led led.c

# User-level code to drive the character LCD.
#
# build options.
LDFLAGS:= 
EXTRA_LDFLAGS:=
CROSS_COMPILE:=arm-linux-
ARCH:=arm

testprog: testprog.c charlib.a
	/usr/arm-linux/bin/gcc -lrt -L. -o testprog testprog.c charlib.a

charlib.a: charlib.c charlib.h
	/usr/arm-linux/bin/gcc -fPIC -g -c -Wall charlib.c
	/usr/arm-linux/bin/ar rcs charlib.a charlib.o

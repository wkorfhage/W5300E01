CC=arm-linux-gcc

key:	keyboardread.c charlib.c
	${CC} -c -fPIC charlib.c
	${CC} -c keyboardread.c
	${CC} -o key charlib.o keyboardread.o -lrt

all:	adtimer led
	
.c.o:
	${CC} -c $@ $<

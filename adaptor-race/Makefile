CC=arm-linux-gcc

.c.o:	
	${CC} -c $< -o $@

race:	rtc.o adapter.o leds_ctrl.o button.o gpio.o race.o keyboardread.o charlib.o
	${CC} -o race rtc.o adapter.o leds_ctrl.o button.o charlib.o gpio.o keyboardread.o race.o -lrt -lpthread

clean:
	rm -rf *.o race

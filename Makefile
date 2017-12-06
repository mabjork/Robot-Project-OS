CC=gcc
CFLAGS= -I /ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment
default: MainProgram

MainProgram: MainProgram.o EngineController.o

		gcc MainProgram.o EngineController.o -Wall -lm -lev3dev-c -o MainProgram

MainProgram.o:

		$(CC) $(CFLAGS) -c src/Client/MainProgram.c -o MainProgram.o

EngineController.o:

		$(CC) $(CFLAGS) -c src/Client/EngineController.c -o EngineController.o


run:
	./MainProgram

clean:
	rm MainProgram
	rm MainProgram.o
	rm EngineController.o
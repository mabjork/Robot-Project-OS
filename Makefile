CC=gcc
CFLAGS= -I /ev3dev-c/source/ev3 -O2 -std=gnu99 -W -Wall -Wno-comment
default: MainProgram

MainProgram: MainProgram.o EngineController.o SensorController.o

		gcc MainProgram.o EngineController.o SensorController.o -Wall -lm -lev3dev-c -o MainProgram

MainProgram.o:

		$(CC) $(CFLAGS) -c src/Client/MainProgram.c -o MainProgram.o

EngineController.o:

		$(CC) $(CFLAGS) -c src/Client/EngineController.c -o EngineController.o

SensorController.o:

		$(CC) $(CFLAGS) -c src/Client/SensorController.c -o SensorController.o

StopProgram: EngineController.o
		$(CC) $(CFLAGS) -c src/Client/StopEngines.c -o StopProgram.o
		gcc StopProgram.o EngineController.o -Wall -lm -lev3dev-c -o StopProgram


run:
		./MainProgram

clean:
	rm -f MainProgram
	rm -f StopProgram
	rm -f *.o

CC=gcc
CFLAGS= -I /ev3dev-c/source/ev3 -O2 -std=gnu99 -w -Wall -Wno-comment
default: MainProgram StopProgram

MainProgram: MainProgram.o SensorController.o EngineController.o PositionController.o BluetoothController.o

		gcc MainProgram.o EngineController.o SensorController.o PositionController.o BluetoothController.o -Wall -w -lm -lev3dev-c -lm -lbluetooth -o MainProgram

MainProgram.o:

		$(CC) $(CFLAGS) -c src/Client/MainProgram.c -o MainProgram.o

EngineController.o: SensorController.o

		$(CC) $(CFLAGS) -c src/Client/EngineController.c -o EngineController.o

SensorController.o:

		$(CC) $(CFLAGS) -c src/Client/SensorController.c -o SensorController.o

PositionController.o:

		$(CC) $(CFLAGS) -c src/Client/PositionController.c -o PositionController.o

BluetoothController.o:

		$(CC) $(CFLAGS) -c src/Client/BluetoothController.c -o BluetoothController.o

StopProgram: EngineController.o SensorController.o
		$(CC) $(CFLAGS) -c src/Client/StopEngines.c -o StopProgram.o
		gcc StopProgram.o EngineController.o SensorController.o -Wall -w -lm -lev3dev-c -o StopProgram

PositionTest:
		gcc -w -o PositionTest src/Client/PositionController.c -lm


run:
		./MainProgram

clean:
	rm -f MainProgram
	rm -f StopProgram
	rm -f *.o

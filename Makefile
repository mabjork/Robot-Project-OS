CCL=arm-linux-gnueabi-gcc
CC=gcc
CFLAGS= -I /ev3dev-c/source/ev3 -O2 -std=gnu99 -w -Wall -Wno-comment
CFLAGSL= -I ev3dev-c/source/ev3 -O2 -std=gnu99 -w -Wall -Wno-comment
default: MainProgram StopProgram

MainProgram: MainProgram.o SensorController.o EngineController.o PositionController.o BluetoothController.o ArmController.o

		$(CC) MainProgram.o EngineController.o SensorController.o PositionController.o BluetoothController.o ArmController.o -Wall -w -lm -lev3dev-c -lm -lbluetooth -lpthread -o MainProgram

MainProgram.o:

		$(CC) $(CFLAGS) -c src/Client/MainProgram.c -o MainProgram.o

EngineController.o:

		$(CC) $(CFLAGS) -c src/Client/EngineController.c -o EngineController.o

SensorController.o:

		$(CC) $(CFLAGS) -c src/Client/SensorController.c -o SensorController.o

PositionController.o:

		$(CC) $(CFLAGS) -c src/Client/PositionController.c -o PositionController.o

BluetoothController.o:

		$(CC) $(CFLAGS) -c src/Client/BluetoothController.c -o BluetoothController.o

ArmController.o:
		$(CC) $(CFLAGS) -c src/Client/ArmController.c -o ArmController.o

StopProgram: EngineController.o SensorController.o
		$(CC) $(CFLAGS) -c src/Client/StopEngines.c -o StopProgram.o
		$(CC) StopProgram.o EngineController.o SensorController.o -pthread -Wall -w -lm -lev3dev-c -o StopProgram

PositionTest:
		$(CC) -w -o PositionTest src/Client/PositionController.c -lm


local: 
		$(CCL) $(CFLAGSL) -c src/Client/MainProgram.c -o MainProgram.o
		$(CCL) $(CFLAGSL) -c src/Client/EngineController.c -o EngineController.o
		$(CCL) $(CFLAGSL) -c src/Client/SensorController.c -o SensorController.o
		$(CCL) $(CFLAGSL) -c src/Client/PositionController.c -o PositionController.o
		$(CCL) $(CFLAGSL) -c src/Client/ArmController.c -o ArmController.o
		$(CCL) $(CFLAGSL) -c src/Client/StopEngines.c -o StopProgram.o
		$(CCL) StopProgram.o EngineController.o SensorController.o -pthread -Wall -w -lm -lev3dev-c -o StopProgram
		$(CCL) MainProgram.o EngineController.o SensorController.o PositionController.o ArmController.o -pthread -Wall -w -lm -lev3dev-c -lm  -o MainProgram



run:
		./MainProgram

clean:
	rm -f MainProgram
	rm -f StopProgram
	rm -f PositionTest
	rm -f *.o

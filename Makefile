CC = gcc
CFLAGS  = -g -Wall

default: MainProgram

MainProgram: MainProgram.o EngineController.o BluetoothController.o
    $(CC) $(CFLAGS) -o MainProgram MainProgram.o EngineController.o BluetoothController.o


MainProgram.o: MainProgram.c EngineController.h BluetoothController.h
    $(CC) $(CFLAGS) -c MainProgram.c

EngineController.o: EngineController.c EngineController.h
    $(CC) $(CFLAGS) -c EngineController.c

BluetoothController.o: BluetoothController.c BluetoothController.h
    $(CC) $(CFLAGS) -c BluetoothController.c


clean:
	$(RM) count *.o *~
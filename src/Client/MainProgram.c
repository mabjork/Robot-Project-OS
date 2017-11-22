

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "./headers/EngineController.h"



int main(int argc, char const *argv[]) {
    int engines[2] = {1,2};
    if ( ev3_init() == -1 ) exit(0);
    if (initEngines()== 1){
        runEngines(&engines,1000);
    }


}

void startDiscovery(){
    setEngineMode(0);

}

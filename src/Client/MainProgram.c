

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include "./headers/EngineController.h"
#include "ev3.h"



int main(int argc, char const *argv[]) {
    printf( "LOL this should work\n" );
    int engines[2] = {1,2};
    if ( ev3_init() == -1 ) return ( 1 );
    initEngines();
    runEngines();
    return 0;

}

void startDiscovery(){
    //setEngineMode(0);

}

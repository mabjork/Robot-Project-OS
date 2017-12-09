#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include "./headers/EngineController.h"
#include "ev3.h"
#define Sleep( msec ) usleep(( msec ) * 1000 )

enum {
    START_DISCOVERY,
    STOP_DISCOVERY,
    BACK_TO_START,
    RELEASE_BALL,
};



int main(int argc, char const *argv[]) {
    printf( "LOL this should work\n" );
    if ( ev3_init() == -1 ) return ( 1 );
    printf("Ev3 initiated\n");
    initEngines();
    printf("Engines initiated\n");
    discoverEngines();
    printf("why is it not printing");
    printf("Engines discovered");
    int max_speed = getMaxSpeed();
    printf("Max speed %i",max_speed);
    int test_speed = max_speed*0.5;
    runTimed(test_speed,1000);
    Sleep(2000);
    runToRelPos(test_speed,1000,1000);
    Sleep(3000);
    turnLeft(test_speed,90);
    Sleep(2000);
    turnRight(test_speed,90);
    while(1){
        Sleep(10000);
        break;
    }
    stopEngines();
    
    
    return 0;

}

void startDiscovery(){
    int max_speed = getMaxSpeed();
    int regular_speed = max_speed * 0.5;
    int turn_speed = 0.3 * max_speed;
    turnLeft(turn_speed,90);
    runForever(regular_speed);
    while(1){
        int command = readCommand();

        int is_running = isRunning();

        int sensor_value = getSensorValue();
        if(sensor_value < 1000 && is_running){
            stopEngines();
        }
        if(shouldTurn()){

        }
        else{

        }

    }
}
int shouldTurn(){
    return 0;
}
void turnAndContinue(){

}


void backAwayAndTurn(){
    int max_speed = getMaxSpeed();
    int regular_speed = max_speed * 0.5;
    int turn_speed = 0.3 * max_speed;
    runTimed(regular_speed,1000);
    turnLeft(turn_speed,180);
}

int* findDirectionNotDiscovered(){

}
int readCommand(){

}




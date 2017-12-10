#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include "headers/EngineController.h"
#include "headers/SensorController.h"
#include "headers/PositionController.h"
#include "ev3.h"

#define Sleep( msec ) usleep(( msec ) * 1000 )
#define OBJECT_TO_CLOSE 350
#define TIME_TO_CHECK_SURROUNDINGS 10
#define TIME_TO_CHECK_WALL_CLOSENES 5
#define MAP_HEIGHT 100
#define MAP_WIDTH 100
#define MAP_SCALE 1/3

enum directions{
    FRONT,
    LEFT,
    BACK,
    RIGHT
};
enum commands{
    START_DISCOVERY,
    STOP_DISCOVERY,
    BACK_TO_START,
    RELEASE_BALL,
};
enum robot_state{
    STOPPED,
    RUNNING
};
enum method_state{
    FINISHED,
    INTERUPTED
};
int max_speed;
int regular_speed;
int turn_speed;
unsigned time_since_last_surroundings_check;
unsigned time_since_last_wall_closenes_check;

char surroundins_map[MAP_HEIGHT][MAP_WIDTH];
float distances_around_robot[4];

struct timeval tval_before, tval_after, tval_result;



int main(int argc, char const *argv[]) {
    printf( "LOL this should work\n" );
    if ( ev3_init() == -1 ) return ( 1 );
    printf("Ev3 initiated\n");
    initEngines();
    printf("Engines initiated\n");
    discoverEngines();
    initSensors();

    startDiscovery();
    return 0;

}

void startDiscovery(){
    max_speed = getMaxSpeed();
    regular_speed = max_speed * 0.5;
    turn_speed = 0.3 * max_speed;
    turnLeft(turn_speed,90);
    waitForCommandToFinish();
    //runForever(regular_speed);
    time_since_last_surroundings_check = (unsigned)time(NULL);
    time_since_last_wall_closenes_check = (unsigned)time(NULL);
    printf("LOL");
    while(1){
        gettimeofday(&tval_before, NULL);
        int command = readCommand();

        int is_running = isRunning();
        float distance = getDistanceSensorValue();
        float degrees = getCompassDegrees();
        int gyro_value =getGyroDegress();
        printf("Distance sensor value: %f\n", distance);
        printf("Time since last check %u\n",(time(0)-time_since_last_wall_closenes_check));
        printf("Current compass degree %f\n",degrees);
        printf("Current gyro value %f\n",gyro_value);
        
        if(distance < OBJECT_TO_CLOSE && is_running){
            //turnLeftAndContinue(20);
            //evaluatePosition();
        }
        /*
        if(time(0) - time_since_last_surroundings_check > TIME_TO_CHECK_SURROUNDINGS){
            checkSouroundings();
            time_since_last_surroundings_check = time(0);
        }
        */
        if(time(0) - time_since_last_wall_closenes_check > TIME_TO_CHECK_WALL_CLOSENES){
            //checkIfCloseToWall();
            time_since_last_wall_closenes_check = time(NULL);
        }
        gettimeofday(&tval_after, NULL);

        timersub(&tval_after, &tval_before, &tval_result);

        updateRobotPosition(&tval_result);

        Sleep(100);

    }
}
void evaluatePosition(){
    stopEngines();
    checkSouroundings();
    int direction = evaluateSurroundings();
    

}
void checkIfCloseToWall(){
    stopEngines();
    Sleep(1000);
    turnLeft(turn_speed,45);
    waitForCommandToFinish();
    Sleep(1000);
    turnRight(turn_speed,45);
    waitForCommandToFinish();
    Sleep(500);
    turnRight(turn_speed,45);
    waitForCommandToFinish();
    Sleep(1000);
    turnLeft(turn_speed,45);
    waitForTurnToComplete();
    Sleep(1000);
    runForever(regular_speed);   
}
int shouldTurn(){
    return 0;
}
void checkSouroundings(){
    stopEngines();
    Sleep(1000);
    distances_around_robot[FRONT] = getDistanceSensorValue();
    turnLeft(turn_speed,45);
    Sleep(1000);
    distances_around_robot[LEFT] = getDistanceSensorValue();
    turnLeft(turn_speed,45);
    Sleep(1000);
    distances_around_robot[BACK] = getDistanceSensorValue();
    turnLeft(turn_speed,45);
    Sleep(1000);
    distances_around_robot[RIGHT] = getDistanceSensorValue();
    turnLeft(turn_speed,45);
    Sleep(1000);
    
}
int evaluateSurroundings(){
    int current_highest = 0;
    for(int i = 1; i<sizeof(distances_around_robot);i++){
        if(distances_around_robot[i] > distances_around_robot[current_highest]){
            current_highest = i;
        }
    }
    return current_highest;
}
void turnLeftAndContinue(int angle){
    stopEngines();
    Sleep(100);
    turnLeft(turn_speed,angle);
    waitForCommandToFinish();
    Sleep(1000);
    runForever(regular_speed);
}
void turnRightAndContinue(int angle){
    stopEngines();
    Sleep(100);
    turnRight(turn_speed,angle);
    waitForCommandToFinish();
    Sleep(1000);
    runForever(regular_speed);
}


void backAwayAndTurn(){
    int max_speed = getMaxSpeed();
    int regular_speed = max_speed * 0.5;
    int turn_speed = 0.3 * max_speed;
    runTimed(regular_speed,1000);
    turnLeft(turn_speed,180);
}


int readCommand(){

}
/*
int waitForCommandToFinish(){
    int stateL;
    int stateR;
    do {
        stateL = getLeftEngineState();
        stateR = getRightEngineState();
        float distance = getDistanceSensorValue();
        printf("Distance sensor value: %f\n", distance);
        if(distance < OBJECT_TO_CLOSE){
            printf("To close");
            //turnLeftAndContinue(20);
            stopEngines();
            return INTERUPTED;
        }
    } while(stateR == STOPPED && stateL == STOPPED);

    return FINISHED;
}
*/

void initMap(){
    for(int i = 0; i<MAP_HEIGHT;i++){
        for(int j = 0;j<MAP_WIDTH;j++){
            surroundins_map[i][j] = " ";
        }
    }
    for(int i = MAP_HEIGHT/2-1;i<  MAP_HEIGHT/2+2; i++){
        for(int j =  MAP_WIDTH/2-1; j < MAP_WIDTH/2+2; j++){
            surroundins_map[i][j] = "R";
        }
    }
}
void updateMap(){
    float distance = getDistanceSensorValue();
    float driven_distance;
    for(int i = MAP_HEIGHT/2-1;i<  MAP_HEIGHT/2+2; i++){
        for(int j =  MAP_WIDTH/2-1; j < MAP_WIDTH/2+2; j++){
            surroundins_map[i][j] = " ";
        }
    }
}
void waitForTurnToComplete(){
    int stateL;
    int stateR;
    do {
        stateL = getLeftEngineState();
        stateR = getRightEngineState();
        
    } while(stateL && stateR);


    
}





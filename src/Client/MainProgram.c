#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "headers/EngineController.h"
#ifndef SENSORCONTROLLER
#include "headers/SensorController.h"
#endif
#include "headers/PositionController.h"
#include "ev3.h"
#include <math.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "headers/BluetoothController.h"

#define Sleep( msec ) usleep(( msec ) * 1000 )
#define OBJECT_TO_CLOSE 250
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
    QUIT
};
enum robot_state{
    STOPPED,
    RUNNING
};
enum method_state{
    FINISHED,
    INTERUPTED
};
enum obstacles{
    MOVABLE,
    NON_MOVABLE,
    OTHER
};
//Counts per second. Or degrees per second. One count = one degree.
int max_speed;
int regular_speed;
int turn_speed;
unsigned time_since_last_surroundings_check;
unsigned time_since_last_wall_closenes_check;

char surroundins_map[MAP_HEIGHT][MAP_WIDTH];
float distances_around_robot[4];

struct timeval tval_before, tval_after, tval_result;

int main(int argc, char const *argv[]) {
    btcommunication();
    init();
    startDiscovery();
    //stopmessage();
    return 0;
}

void init(){
    if ( ev3_init() == -1 ) return ( 1 );
    printf("Ev3 initiated\n");
    initEngines();
    printf("Engines initiated\n");
    discoverEngines();
    initSensors();
    initPositionController(getCompassDegrees());
}

void startDiscovery(){
    max_speed = getMaxSpeed();
    regular_speed = max_speed * 0.2;
    turn_speed = 0.1 * max_speed;
    turnLeft(turn_speed,90);
    waitForCommandToFinish();
    runForever(regular_speed);
    time_since_last_surroundings_check = (unsigned)time(NULL);
    time_since_last_wall_closenes_check = (unsigned)time(NULL);
    gettimeofday(&tval_before, NULL);
    while(1){
        
        //int command = readCommand();

        int is_running = isRunning();
        float distance = getDistanceSensorValue();
        float heading = getCompassDegrees();
        //int gyro_value =getGyroDegress();
        //printf("Distance sensor value: %f\n", distance);
        //printf("Time since last check %u\n",(time(NULL)-time_since_last_wall_closenes_check));
        //printf("Current compass degree %f\n",heading);
        //printf("Current gyro value %f\n",gyro_value);
        
        if(distance < OBJECT_TO_CLOSE && is_running){
            measureAndUpdateTraveledDistance(heading);
            evaluatePosition();
            gettimeofday(&tval_before, NULL);
            time_since_last_wall_closenes_check = time(NULL);
        }
        
        else{

            if(time(NULL) - time_since_last_wall_closenes_check > TIME_TO_CHECK_WALL_CLOSENES){
                measureAndUpdateTraveledDistance(heading);
                checkIfCloseToWall();
                time_since_last_wall_closenes_check = time(NULL);
                gettimeofday(&tval_before, NULL);

            }else{
                measureAndUpdateTraveledDistance(heading);
                gettimeofday(&tval_before, NULL);
            }
            
            
            
    
            
        }
        //Sleep(1000);
        
        

    }
}

double calculateDistance(struct timeval *time){
    long time_in_usec = (time->tv_sec*1000000.0 + time->tv_usec);
    //printf("Time in micro seconds: %d\n",time_in_usec);
    //Distance in counts each usec;

    double counts = regular_speed*time_in_usec/1000000.000;
    double wheel_radius = getWheelDiameter()/2;
    //Distance in cm.
    double distance = counts * wheel_radius * M_PI / 360 ;


    return distance;
}

void measureAndUpdateTraveledDistance(int heading){
    setCurrentHeading(heading);
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    //long diff_sec = tval_after.tv_sec - tval_before.tv_sec;
    //long diff_usec = (tval_after.tv_usec - tval_before.tv_usec);
    //printf("Diff sec %ld . Diff usec %ld\n",diff_sec,diff_usec);
    double traveled_distance = calculateDistance(&tval_result);
    printf("Traveled distance %lf\n", traveled_distance);
    updateRobotPosition(traveled_distance);
    
}


//TODO: Update the map with the value for the position.
void setMapPointValue(){
    int obstacle = whatIsObstacle();
    if (obstacle == NON_MOVABLE){

    }else if (obstacle == MOVABLE){
        
    }else if (obstacle == OTHER){
        
    }
}
//TODO: Make this method discover what the obstable is.
int whatIsObstacle(){

}


void evaluatePosition(){
    stopEngines();
    checkSouroundings();
    int direction = evaluateSurroundings();
    printf("Should take this direction %i\n", direction);
    if(direction == FRONT){
        runForever(regular_speed);
    }else if(direction == BACK){
        turnNumberOfDegsCorrected(turn_speed,180);
        runForever(regular_speed);
    }else if(direction == LEFT){
        turnNumberOfDegsCorrected(turn_speed,90);
        runForever(regular_speed);
    }else if(direction == RIGHT){
        turnNumberOfDegsCorrected(turn_speed,-90);
        runForever(regular_speed);
    }

}

void checkSouroundings(){
    stopEngines();
    Sleep(1000);
    distances_around_robot[FRONT] = getDistanceSensorValue();
    turnNumberOfDegsCorrected(turn_speed,90);
    Sleep(1000);
    distances_around_robot[LEFT] = getDistanceSensorValue();
    turnNumberOfDegsCorrected(turn_speed,90);
    Sleep(1000);
    distances_around_robot[BACK] = getDistanceSensorValue();
    turnNumberOfDegsCorrected(turn_speed,90);
    Sleep(1000);
    distances_around_robot[RIGHT] = getDistanceSensorValue();
    turnNumberOfDegsCorrected(turn_speed,90);
    Sleep(1000);
    
}
int evaluateSurroundings(){
    int current_highest = 0;
    for(int i = 1; i<4;i++){
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



void waitForTurnToComplete(){
    int stateL;
    int stateR;
    do {
        stateL = getLeftEngineState();
        stateR = getRightEngineState();
        
    } while(stateL && stateR);
    
}

void checkIfCloseToWall(){
    float distanceLeft;
    float distanceRight;
    stopEngines();
    Sleep(1000);
    turnLeft(turn_speed,30);
    waitForCommandToFinish();
    distanceLeft = getDistanceSensorValue();
    Sleep(1000);
    turnRight(turn_speed,60);
    waitForCommandToFinish();
    distanceRight = getDistanceSensorValue();
    Sleep(1000);
    turnLeft(turn_speed,30);
    waitForCommandToFinish();
    Sleep(1000);

    if(distanceLeft <= OBJECT_TO_CLOSE && distanceRight <= OBJECT_TO_CLOSE){
        printf("Close both directions, turn around");
        turnNumberOfDegsCorrected(turn_speed,180);
        runForever(regular_speed);
    }
    else if(distanceLeft <= OBJECT_TO_CLOSE){
        printf("Close on the left, correct right");
        turnNumberOfDegsCorrected(turn_speed,30);
        runForever(regular_speed);

    }
    else if(distanceRight <= OBJECT_TO_CLOSE){
        printf("Close on the right, correct left");
        turnNumberOfDegsCorrected(turn_speed,-30);
        runForever(regular_speed);
    }else{
        printf("Continue foreward");
        runForever(regular_speed);
    }
       
}
int shouldTurn(){
    return 0;
}





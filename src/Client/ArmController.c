#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ev3.h"
#include "ev3_port.h"
#include "ev3_tacho.h"
#include "ev3_sensor.h"
#include "headers/EngineController.h"
#include "headers/SensorController.h"
#include "headers/ArmController.h"


#include <math.h>
#include <unistd.h>
#include <time.h>

#define Sleep( msec ) usleep(( msec ) * 1000 )


int pos;
int status;

enum {
    RAISED,
    LOWERED
};

enum {
    NOBALL,
    HASBALL
};

void initArm(){
    pos = RAISED;
    status = NOBALL;
}

void armLogic(){
    pos = RAISED;
    status = NOBALL;
    if (pos == LOWERED ){
        raiseArm();
    

    }
}

int checkIfMovable(){
    int objType = recognizeObject();
    printf("Object Type is %i\n", objType);
    if (objType == 2){
        return 1;
    }
    else{ return 0; }
}

int checkIfCloseEnough(){
    int distance = getDistanceSensorValue();
    printf("Distance is: %i\n", distance);
    int maxDistance = 50;
    int minDistance = 0;
    if (distance <= maxDistance && distance >= minDistance){
        return 1;
    }
    else{ return 0; }
}



void driveWithBall(){

}

void armCapturingMovable(){
    lowerArm();
    status = HASBALL;
    pos = LOWERED;
}

void armReleasingMovable(){
    raiseArm();
    status = NOBALL;
    pos = RAISED;

}

void testingBallRelease(int speed){
    initArm();
    int i;
    while(1){
        i += 1;
        if (pos == RAISED && status == NOBALL){
            printf("Arm raised and no ball.\n");
            if (checkIfMovable && checkIfCloseEnough){
                armCapturingMovable();
                printf("Ball Captured!\n");
                Sleep(2000);
            }
        }
        else if (pos  == LOWERED && status == HASBALL){
            runDistance(speed, 1000);
            waitForCommandToFinish();
            armReleasingMovable();
            printf("Ball released!\n");
            runDistance(speed, -1000);
            waitForCommandToFinish();
        }
        Sleep(500);
        printf("Iteration %i!\n", i);

    }
}
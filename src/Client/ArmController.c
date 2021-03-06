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
#include "headers/BluetoothController.h"


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
    status = HASBALL;
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
    int maxDistance = 70;
    int minDistance = 0;
    if ((distance <= maxDistance && distance >= minDistance) || distance == 2550){
        return 1;
    }
    else{ return 0; }
}


void armCapturingMovable(){
    lowerArm();
    status = HASBALL;
    pos = RAISED;
}

void armReleasingMovable(){
    if (pos == RAISED && status == HASBALL){
        lowerArm();
        status = NOBALL;
        pos = LOWERED;
        Sleep(1000);
        raiseArm();
        pos = RAISED;
        bt_obstacle();
    }


}

void testingBallRelease(int speed){
    initArm();
    int i;
    int check1;
    int check2;
    while(1){
        i += 1;
        if (pos == RAISED && status == NOBALL){
            printf("Arm raised and no ball.\n");
            check1 = checkIfMovable();
            check2 = checkIfCloseEnough();
            if (check1 == 1 && check2 == 1){
                armCapturingMovable();
                printf("Ball Captured!\n");
                Sleep(1000);
            }
        }
        if (pos == LOWERED && status == HASBALL){
            runDistance(speed, 500);
            waitForCommandToFinish();
            Sleep(1000);
            armReleasingMovable();
            printf("Ball released!\n");
            runDistance(speed, -500);
            waitForCommandToFinish();
            printf("Finished.");
            break;
        }
        Sleep(1000);
        printf("Iteration %i!\n", i);

    }
}
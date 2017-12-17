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

int pos;
int status;

enum {
    RAISED,
    LOWERED
};

enum {
    HASBALL,
    NOBALL
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
    if (objType == 2){
        return 1;
    }
    else{ return 0; }
}

int checkIfMovableCloseEnough(){
    int distance = getDistanceSensorValue();
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
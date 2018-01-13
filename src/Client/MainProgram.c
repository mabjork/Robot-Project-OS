#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "headers/EngineController.h"
#include "headers/SensorController.h"
#include "headers/PositionController.h"
#include "headers/ArmController.h"
#include "ev3.h"
#include <math.h>
#include "headers/BluetoothController.h"

#define Sleep( msec ) usleep(( msec ) * 1000 )
#define OBJECT_TO_CLOSE 250
#define COLOR_CHECK_DISTANCE 100
#define TIME_TO_CHECK_WALL_CLOSENES 4
#define TIME_TO_SEND_POS 1



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
int check_color_speed;
unsigned time_since_last_surroundings_check;
unsigned time_since_last_wall_closenes_check;
unsigned time_since_last_position_update;


//char surroundins_map[MAP_HEIGHT][MAP_WIDTH];
float distances_around_robot[4];

struct timeval tval_before, tval_after, tval_result;

int main(int argc, char const *argv[]) {
    //btcommunication();
    init();
    initArm();
    //startDiscovery();
    testReleaseMoveable();
    //stopmessage();
    return 0;
}

void test(){
    turn2(90);
    float init_deg = getInitialHeading();
    float current = getGyroDegrees();
    printf("Initial heading : %f\n",init_deg);
    printf("Current heading : %f\n",current);
    /*
    float value;
    float heading;
    while(1){
        //calibrateGyro();
        value = getGyroDegrees();
        //heading = getCompassDegrees();
        printf("Current gyro value %f\n",value);
        printf("Current compass degree %f\n",heading);
        Sleep(1000);
    }
    */
    
}
void testReleaseMoveable(){
    printf("Ready to release movable.");
    armReleasingMovable();
    printf("Movable is released.");

}

void init(){
    if ( ev3_init() == -1 ) return ( 1 );
    printf("Ev3 initiated\n");
    initEngines();
    printf("Engines initiated\n");
    //discoverEngines();
    initSensors();
    calibrateGyro();
    float init_heading = getGyroDegrees();
    printf("Setting heading : %f\n",init_heading);
    initPositionController(init_heading);
    

}

void startDiscovery(){
    max_speed = getMaxSpeed();
    regular_speed = max_speed * 0.2;
    turn_speed = 0.1 * max_speed;
    check_color_speed = 0.05 * max_speed;
    //turnNumberOfDegsCorrected(turn_speed,90);
    //waitForCommandToFinish();
   
    
    time_since_last_surroundings_check = (unsigned)time(NULL);
    time_since_last_wall_closenes_check = (unsigned)time(NULL);
    time_since_last_position_update = (unsigned)time(NULL);
    //goToNextUndiscoveredPoint();
    gettimeofday(&tval_before, NULL);
    //bt_send_position();
    runForever(regular_speed);

    while(1){
        
        //int command = readCommand();

        int is_running = isRunning();       
        float distance = getDistanceSensorValue();
        float current_heading = getGyroDegrees();
        //int gyro_value =getGyroDegress();
        printf("Distance sensor value: %f\n", distance);
        //printf("Time since last check %u\n",(time(NULL)-time_since_last_wall_closenes_check));
        printf("Current compass degree %f\n",current_heading);
        //printf("Current gyro value %f\n",gyro_value);
        
        if(distance < OBJECT_TO_CLOSE && is_running){
            printf("Object to close\n");
            measureAndUpdateTraveledDistance(regular_speed,&current_heading);

            gettimeofday(&tval_before, NULL);
            int obstacle = whatIsObstacle();
            measureAndUpdateTraveledDistance(check_color_speed,&current_heading);

            gettimeofday(&tval_before, NULL);
            runDistance(-regular_speed, 1000);
            waitForCommandToFinish();
            measureAndUpdateTraveledDistance(-regular_speed,&current_heading);
            //evaluatePosition();
            turnNumberOfDegsCorrected(turn_speed,110);
            runForever(regular_speed);
            gettimeofday(&tval_before, NULL);
            time_since_last_wall_closenes_check = time(NULL);
        }
        else if(isArrivedAtPoint() == 1){
            printf("LOLOLOLOLOLOLOL should not happen \n");
            //measureAndUpdateTraveledDistance(regular_speed,&current_heading);
            //turnToInitialHeading();
            //checkSouroundings();
            //goToNextUndiscoveredPoint();
        }
        else{

            if(time(NULL) - time_since_last_wall_closenes_check > TIME_TO_CHECK_WALL_CLOSENES){
                measureAndUpdateTraveledDistance(regular_speed,&current_heading);
                //checkIfCloseToWall();
                time_since_last_wall_closenes_check = time(NULL);
                gettimeofday(&tval_before, NULL);

            }else{
                measureAndUpdateTraveledDistance(regular_speed,&current_heading);
                gettimeofday(&tval_before, NULL);
            }
            
        }
        /*
        if(time(NULL) - time_since_last_position_update > TIME_TO_SEND_POS){
            bt_send_position();
            time_since_last_position_update = time(NULL);
        }
        */
        //Sleep(1000);
        
        

    }
}

double calculateDistance(int speed,struct timeval *time){
    long time_in_usec = (time->tv_sec*1000000.0 + time->tv_usec);
    //printf("Time in micro seconds: %d\n",time_in_usec);
    //Distance in counts each usec;

    double counts = speed*time_in_usec/1000000.000;
    double wheel_radius = getWheelDiameter()/2;
    //Distance in cm.
    double distance = counts * wheel_radius * M_PI / 360 ;


    return distance;
}

void measureAndUpdateTraveledDistance(int speed,float* heading){
    setCurrentHeading(heading[0]);
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    //long diff_sec = tval_after.tv_sec - tval_before.tv_sec;
    //long diff_usec = (tval_after.tv_usec - tval_before.tv_usec);
    //printf("Diff sec %ld . Diff usec %ld\n",diff_sec,diff_usec);
    double traveled_distance = calculateDistance(speed,&tval_result);
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
    runForever(check_color_speed);
    while(1){
        
        float distance = getDistanceSensorValue();
        if(distance <= COLOR_CHECK_DISTANCE){
            stopEngines();
            break;
        }
    }
    turnNumberOfDegsCorrected(turn_speed,30),
    waitForCommandToFinish();
    Sleep(500);
    float dist1 = getDistanceSensorValue();
    Sleep(500);
    turnNumberOfDegsCorrected(turn_speed,-60);
    waitForCommandToFinish();
    Sleep(500);
    float dist2 = getDistanceSensorValue();
    Sleep(500);
    turnNumberOfDegsCorrected(turn_speed,30);
    waitForCommandToFinish();
    Sleep(500);
    int object = recognizeObject();
    if(object == 2){
        printf("The object is movable !!!!!!!!!!!!!!!!!!\n");
        return MOVABLE;
    }
    if (dist1 > 200 && dist2 > 200){
        printf("The object is movable !!!!!!!!!!!!!!!!!!\n");
        //Sleep(10000);
        return MOVABLE;
    }
    else{
        printf("The object is non movable !!!!!!!!!!!!!!!!!!\n");
        //Sleep(10000);
        return NON_MOVABLE;
    }

    
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
    printf("Checking surroundings");
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
/*
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
*/

/*
void waitForTurnToComplete(){
    int stateL;
    int stateR;
    do {
        stateL = getLeftEngineState();
        stateR = getRightEngineState();
        
    } while(stateL && stateR);
    
}
*/
void checkIfCloseToWall(){
    float distanceLeft;
    float distanceRight;
    stopEngines();
    Sleep(1000);
    turnNumberOfDegsCorrected(turn_speed,30);
    waitForCommandToFinish();
    distanceLeft = getDistanceSensorValue();
    Sleep(1000);
    turnNumberOfDegsCorrected(turn_speed,-60);
    waitForCommandToFinish();
    distanceRight = getDistanceSensorValue();
    Sleep(1000);
    turnNumberOfDegsCorrected(turn_speed,30);
    waitForCommandToFinish();
    Sleep(1000);

    if(distanceLeft <= OBJECT_TO_CLOSE && distanceRight <= OBJECT_TO_CLOSE){
        printf("Close both directions, turn around");
        turnNumberOfDegsCorrected(turn_speed,180);
        runForever(regular_speed);
    }
    else if(distanceLeft <= OBJECT_TO_CLOSE){
        printf("Close on the left, correct right");
        turnNumberOfDegsCorrected(turn_speed,20);
        runForever(regular_speed);

    }
    else if(distanceRight <= OBJECT_TO_CLOSE){
        printf("Close on the right, correct left");
        turnNumberOfDegsCorrected(turn_speed,-20);
        runForever(regular_speed);
    }else{
        printf("Continue foreward");
        runForever(regular_speed);
    }
       
}

void turnToInitialHeading(){
    int initial_heading = (int)getInitialHeading();
    turnToDeg(turn_speed,initial_heading);
}

void goToNextUndiscoveredPoint(){
    findPoints();
    int * point;
    point = popFromQueue();
    int x = *(point);
    int y = *(point + 1);
    printf("x = %i , y = %i\n",x,y);
    double dx;
    double dy;
    float h;
    getDistanceAndDirectionToPoint(x,y,&dx,&dy,&h);
    double dist = sqrt(pow(dx,2) + pow(dy,2));
    printf("dx is %lf , dy is %lf, h is %f\n",dx,dy,h);
    runToRelPos(regular_speed,dist,h);
}

int isArrivedAtPoint(){
    return 0;
}

void findAlternateRoute(){
    
}







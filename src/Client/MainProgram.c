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
#include "ev3.h"
#include <math.h>
#include "headers/BluetoothController.h"

#define Sleep( msec ) usleep(( msec ) * 1000 )
#define OBJECT_TO_CLOSE 250
#define SHOULD_AVOID 400
#define COLOR_CHECK_DISTANCE 100
#define TIME_TO_CHECK_WALL_CLOSENES 3
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
int discovering;


//char surroundins_map[MAP_HEIGHT][MAP_WIDTH];
float distances_around_robot[4];

struct timeval tval_before, tval_after, tval_result;

int main(int argc, char const *argv[]) {
    //btcommunication();
    init();
    next_point_test();
    //wall_close_test();
    //test();
    //recognize_test();
    //startDiscovery();
    //thread_test();
    //printf("Running forever\n");
    //runForeverCorrected(regular_speed);
    //Sleep(10000);
    //stopEngines();
    //stopmessage();
    //turnNumberOfDegsCorrected(turn_speed,180);
    //float current_change = getGyroDegrees();
    //printf("Current change is %f\n",current_change);
    //float heading_diff = current_change- last_gyro_read;
    //updateCurrentHeading(heading_diff);
    //printf("Current heading is %f\n",HEADING);
    //turnToInitialHeading();
    exit(0);
}
void next_point_test(){
    last_gyro_read = getGyroDegrees();
    goToNextUndiscoveredPoint();
    float current_heading = getGyroDegrees();
    float heading_diff = current_heading - last_gyro_read;
    printf("Heading diff %f\n",heading_diff);
    updateCurrentHeading(-heading_diff);
    turnToInitialHeading();
}
void thread_test(){
    runForeverCorrected(regular_speed);
    Sleep(10000);
    stopEngines();
    Sleep(5000);
    runForeverCorrected(regular_speed);
    Sleep(10000);
    stopEngines();
}
void wall_close_test(){
    checkIfCloseToWall();
}
void recognize_test(){
    float current_heading = getGyroDegrees();
    gettimeofday(&tval_before, NULL);
    printf("Starting test\n");
    int object = whatIsObstacle();
    measureAndUpdateTraveledDistance(check_color_speed,&current_heading);
    int x,y;
    getSquareInFront(COLOR_CHECK_DISTANCE/10,&x,&y);
    printf("This is x: %i , This is y: %i\n",x,y);
}
void test(){
    turn2(90);
    float init_deg = INITIAL_HEADING;
    float current = getGyroDegrees();
    printf("Initial heading : %f\n",init_deg);
    printf("Current heading : %f\n",current);
    updateCurrentHeading(current);
    float new = getCurrentHeading();
    printf("Updated heading : %f\n", new);
    calibrateGyro();
    current = getGyroDegrees();
    printf("Current heading : %f\n",current);
    Sleep(1000);
    turn2(90);
    init_deg = INITIAL_HEADING;
    current = getGyroDegrees();
    printf("Initial heading : %f\n",init_deg);
    printf("Current heading : %f\n",current);
    updateCurrentHeading(current);
    new = getCurrentHeading();
    printf("Updated heading : %f\n", new);
    calibrateGyro();
    current = getGyroDegrees();
    printf("Current heading : %f\n",current);

    
}

void init(){
    if ( ev3_init() == -1 ) return ( 1 );
    printf("Ev3 initiated\n");
    initEngines();
    printf("Engines initiated\n");
    initSensors();
    printf("Sensors initiated\n");
    calibrateGyro();
    printf("Gyro calibrated\n");
    initPositionController(90,5,3);
    printf("Position controller initiated\n");
    getEngineSpeeds();
    initTimes();
    discovering = 1;
}
void getEngineSpeeds(){
    max_speed = getMaxSpeed();
    regular_speed = max_speed * 0.2;
    turn_speed = 0.1 * max_speed;
    check_color_speed = 0.05 * max_speed;
}
void initTimes(){
    time_since_last_surroundings_check = (unsigned)time(NULL);
    time_since_last_wall_closenes_check = (unsigned)time(NULL);
    time_since_last_position_update = (unsigned)time(NULL);
    gettimeofday(&tval_before, NULL);
}
void startDiscovery(){
    
    //bt_send_position();
    last_gyro_read = getGyroDegrees();
    goToNextUndiscoveredPoint();
    //runForeverCorrected(regular_speed);
    while(discovering){
        int is_running = isRunning();       
        float distance = getDistanceSensorValue();
        float current_heading = getGyroDegrees();
        float heading_diff = current_heading- last_gyro_read;
        last_gyro_read = current_heading;
        if(fabs(current_heading) >= 1){
            //calibrateGyro();
        }
        printf("Distance sensor value: %f\n", distance);
        //printf("Time since last check %u\n",(time(NULL)-time_since_last_wall_closenes_check));
        printf("Current heading: %f\n",current_heading);
        
        if(distance < OBJECT_TO_CLOSE && is_running){
            printf("Object to close\n");
            measureAndUpdateTraveledDistance(regular_speed,&heading_diff);
            int obstacle = whatIsObstacle();
            float close_dist = getDistanceSensorValue();
            setMapPointValue(obstacle,close_dist);
            gettimeofday(&tval_before, NULL);
            backAwayTimed(regular_speed,1500);
            measureAndUpdateTraveledDistance(-regular_speed,&current_heading);
            turnNumberOfDegsCorrected(turn_speed,90);
            checkIfCloseToWall();
            runForeverCorrected(regular_speed);
            gettimeofday(&tval_before, NULL);
            time_since_last_wall_closenes_check = time(NULL);
        }
        else if(isArrivedAtPoint() == 1){
            printf("LOLOLOLOLOLOLOL should not happen \n");
            measureAndUpdateTraveledDistance(regular_speed,&current_heading);
            turnToInitialHeading();
            //checkSouroundings();
            goToNextUndiscoveredPoint();
        }
        else{

            if(time(NULL) - time_since_last_wall_closenes_check > TIME_TO_CHECK_WALL_CLOSENES){
                measureAndUpdateTraveledDistance(regular_speed,&heading_diff);
                //checkIfCloseToWall();
                time_since_last_wall_closenes_check = time(NULL);
                gettimeofday(&tval_before, NULL);

            }else{
                measureAndUpdateTraveledDistance(regular_speed,&heading_diff);
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
    stopEngines();
    exit(0);
}

double calculateDistance(int speed,struct timeval *time){
    long time_in_usec = (time->tv_sec*1000000.0 + time->tv_usec);
    //printf("Time in micro seconds: %d\n",time_in_usec);
    //Distance in counts each usec;

    double counts = speed*time_in_usec/1000000.000;
    double wheel_radius = WHEEL_DIAMETER/2;
    //Distance in cm.
    double distance = counts * wheel_radius * M_PI / 360 ;


    return distance;
}

void measureAndUpdateTraveledDistance(int speed,float* heading){
    updateCurrentHeading(-heading[0]);
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);

    double traveled_distance = calculateDistance(speed,&tval_result);
    printf("Traveled distance %lf\n", traveled_distance);
    updateRobotPosition(traveled_distance);
    
}


//TODO: Update the map with the value for the position.
void setMapPointValue(int obstacle,float distance){
    int x;
    int y;
    getSquareInFront(distance,&x,&y);
    if (obstacle == NON_MOVABLE){
        updateMap(x,y,'N');
    }else if (obstacle == MOVABLE){
        updateMap(x,y,'M');
    }else if (obstacle == OTHER){
        
    }
}
//TODO: Make this method discover what the obstable is.
int whatIsObstacle(){
    printf("Running at speed: %i\n",check_color_speed);
    runForeverCorrected(check_color_speed);
    float current_heading;
    float heading_diff;
    while(1){
        current_heading = getGyroDegrees();
        heading_diff = current_heading - last_gyro_read;
        last_gyro_read = current_heading;
        if(fabs(current_heading) >= 1){
            //calibrateGyro();
        }
        gettimeofday(&tval_before, NULL);
        float distance = getDistanceSensorValue();
        if(distance <= COLOR_CHECK_DISTANCE){
            stopEngines();
            measureAndUpdateTraveledDistance(check_color_speed,&heading_diff);
            break;
        }
        Sleep(100);
        measureAndUpdateTraveledDistance(check_color_speed,&heading_diff);
    }
    turnNumberOfDegsCorrected(turn_speed,25),
    Sleep(50);
    float dist1 = getDistanceSensorValue();
    Sleep(50);
    turnNumberOfDegsCorrected(turn_speed,-50);
    Sleep(50);
    float dist2 = getDistanceSensorValue();
    Sleep(50);
    turnNumberOfDegsCorrected(turn_speed,25);
    Sleep(50);
    if (dist1 > 200 && dist2 > 200){
        printf("The object is movable !!!!!!!!!!!!!!!!!!\n");
        return MOVABLE;
    }
    else{
        printf("The object is non movable !!!!!!!!!!!!!!!!!!\n");
        return NON_MOVABLE;
    }/*
    int object = recognizeObject();
    if(object == 1){
        return NON_MOVABLE;
    }else if(object == 2){
        return MOVABLE;
    }else{
        return OTHER;
    }
    */
}
void findCenterOfObject(){

}


void checkIfCloseToWall(){
    float distanceLeft;
    float distanceRight;
    stopEngines();
    Sleep(50);
    turnNumberOfDegsCorrected(turn_speed,45);
    distanceLeft = getDistanceSensorValue();
    Sleep(50);
    turnNumberOfDegsCorrected(turn_speed,-90);
    distanceRight = getDistanceSensorValue();
    Sleep(50);
    turnNumberOfDegsCorrected(turn_speed,45);
    Sleep(50);

    if(distanceLeft <= SHOULD_AVOID && distanceRight <= SHOULD_AVOID){
        printf("Close both directions, turn around\n");
        turnNumberOfDegsCorrected(turn_speed,180);
        
    }
    else if(distanceLeft <= SHOULD_AVOID){
        printf("Close on the left, correct right\n");
        turnNumberOfDegsCorrected(turn_speed,-20);
        

    }
    else if(distanceRight <= SHOULD_AVOID){
        printf("Close on the right, correct left\n");
        turnNumberOfDegsCorrected(turn_speed,20);
        
    }else{
        printf("Continue foreward\n");
        
    }
    runForeverCorrected(regular_speed);
       
}

void turnToInitialHeading(){
    float heading = INITIAL_HEADING;
    printf("LOL should turn to this %f\n",heading);
    turnToDegCorrected(turn_speed,INITIAL_HEADING);
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
    printf("Running to rel pos\n");
    runToRelPos(regular_speed,dist,h);
}

int isArrivedAtPoint(){
    return 0;
}

void findAlternateRoute(){
    
}








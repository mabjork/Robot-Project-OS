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
#define OBJECT_TO_CLOSE 200
#define SHOULD_AVOID 300
#define COLOR_CHECK_DISTANCE 100
#define TIME_TO_CHECK_WALL_CLOSENES 3
#define TIME_TO_SEND_POS 1
#define RELEASE_OBJECT_TIME 60



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
unsigned game_start_time;
int discovering;


//char surroundins_map[MAP_HEIGHT][MAP_WIDTH];
float distances_around_robot[4];

struct timeval tval_before, tval_after, tval_result;

int main(int argc, char const *argv[]) {
    init();
    startDiscovery();
    Sleep(1000);
    //distance_test();
    /*
    if( bt_connect() == 0 ) {
        printf("Connected!\n");
        bt_transmit();
        while(1){
            bt_check();
        }
    }
    else {
        fprintf (stderr, "Failed to connect to server...\n");
        sleep (2);
        exit (EXIT_FAILURE);
    }
    */
    //engine_reset();
    return 0;
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
    initPositionController(90,5,4);
    printf("Position controller initiated\n");
    getEngineSpeeds();
    initTimes();
    discovering = 1;
    startPositionUpdateThread();
    //initArm();
}

void getEngineSpeeds(){
    max_speed = getMaxSpeed();
    regular_speed = max_speed * 0.2;
    turn_speed = 0.1 * max_speed;
    check_color_speed = 0.1 * max_speed;
    
}

void initTimes(){
    time_since_last_surroundings_check = (unsigned)time(NULL);
    time_since_last_wall_closenes_check = (unsigned)time(NULL);
    time_since_last_position_update = (unsigned)time(NULL);
    game_start_time = (unsigned)time(NULL);
}
/**
 * Stage one in the discovery algorithm. Works its way from side to side up the course.
 * */
void startDiscovery(){

    turnNumberOfDegsCorrected(turn_speed,-90);
    //correctAngle();
    time_since_last_wall_closenes_check = (unsigned)time(NULL);
    runForeverCorrected(regular_speed);
    while(discovering){
        int is_running = isRunning();       
        float distance = getDistanceSensorValue();
        printf("Distance sensor value: %f\n", distance);
        if(time(NULL) - game_start_time >= 240){
            break;
        }
        if(time(NULL) - game_start_time >= RELEASE_OBJECT_TIME){
            armReleasingMovable();
            turnAndContinue();
        }
        if((distance < OBJECT_TO_CLOSE && is_running)){
            
            printf("Object to close\n");
            int obstacle = whatIsObstacle();
            float close_dist = getDistanceSensorValue();
            setMapPointValue(obstacle,close_dist);
            Sleep(50);
            backAwayTimed(regular_speed,500);
            Sleep(50);
            turnAndContinue();
            Sleep(50);
            correctAngle();
            Sleep(50);
            runForeverCorrected(regular_speed);
            time_since_last_wall_closenes_check = time(NULL);
        }
        if(current_square_x == 0 || current_square_y == 0){
            if(HEADING >= 135 && HEADING <= 315){
                turnAndContinue();
                Sleep(50);
                correctAngle();
                runDistance(regular_speed,5);
                runForeverCorrected(regular_speed);
            }
            
            
        }
        
        if(time(NULL) - time_since_last_wall_closenes_check > TIME_TO_CHECK_WALL_CLOSENES){
                checkIfCloseToWall();
                time_since_last_wall_closenes_check = time(NULL);

            }
        Sleep(50);
    }
    stopEngines();
    findLastPoints();
}
/**
 * Stage two of the discovery algorithm. Finds remainding points and discoveres them sequentially.
 * */
void findLastPoints(){
    goToNextUndiscoveredPoint();
    while(discovering){
        int is_running = isRunning();       
        float distance = getDistanceSensorValue();
        if((distance < OBJECT_TO_CLOSE && is_running)){
            
            printf("Object to close\n");
            int obstacle = whatIsObstacle();
            float close_dist = getDistanceSensorValue();
            setMapPointValue(obstacle,close_dist);
            Sleep(100);
            backAwayTimed(regular_speed,500);
            Sleep(100);
            correctAngle();
            Sleep(100);
            turnAndContinue();
            correctAngle();
            time_since_last_wall_closenes_check = time(NULL);
        }
        if(current_square_x <= 0 || current_square_y <= 0){
            if(HEADING < 360 && HEADING > 180){
                //turnAndContinue();
            }
            
        }
        if(isArrivedAtPoint() == 1){
            //turnToInitialHeading();
            //checkSouroundings();
            goToNextUndiscoveredPoint();
        }
        else{

            if(time(NULL) - time_since_last_wall_closenes_check > TIME_TO_CHECK_WALL_CLOSENES){
                //checkIfCloseToWall();
                time_since_last_wall_closenes_check = time(NULL);

            }
            
        }
        Sleep(100);

    }
}
void goBackToStart(){

}
int turn_direction = 1;

void turnAndContinue(){
    stopEngines();
    turnNumberOfDegsCorrected(turn_speed,90 * turn_direction);
    Sleep(100);
    float distance = getDistanceSensorValue();

    if(distance <= OBJECT_TO_CLOSE){
        turnNumberOfDegsCorrected(turn_speed,180);
        distance = getDistanceSensorValue();
        if(distance <= OBJECT_TO_CLOSE){
            turnNumberOfDegsCorrected(turn_speed,-90);
        }
        
    }
    else{
        runDistance(regular_speed,10);
        Sleep(500);
        turnNumberOfDegsCorrected(turn_speed,90 * turn_direction);
        

    }
    stopEngines();
    //runForeverCorrected(regular_speed);
    turn_direction = turn_direction * (-1);

}

void correctAngle(){
    int current_heading = HEADING;
    if(current_heading < 135 && current_heading >= 45){
        turnToDegCorrected(turn_speed,90);
    }else if(current_heading < 215 && current_heading >= 135){
        turnToDegCorrected(turn_speed,180);
    }else if(current_heading < 305 && current_heading >= 215){
        turnToDegCorrected(turn_speed,270);
    }else{
        turnToDegCorrected(turn_speed,0);
    }
}

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

int whatIsObstacle(){
    stopEngines();
    Sleep(100);
    printf("Running at speed: %i\n",check_color_speed);
    runForeverCorrected(check_color_speed);
    int time_since_start = time(NULL);
    while(1){
        float distance = getDistanceSensorValue();
        if(distance <= COLOR_CHECK_DISTANCE){
            stopEngines();
            break;
        }else if(time_since_start - time(NULL) >= 5){
            stopEngines();
            return OTHER;
        }
        Sleep(100);
    }
    stopEngines();
    turnNumberOfDegsCorrected(turn_speed,25),
    Sleep(50);
    float dist1 = getDistanceSensorValue();
    Sleep(50);
    turnNumberOfDegsCorrected(turn_speed,-25);
    Sleep(50);
    turnNumberOfDegsCorrected(turn_speed,-25);
    Sleep(50);
    float dist2 = getDistanceSensorValue();
    Sleep(50);
    turnNumberOfDegsCorrected(turn_speed,25);
    Sleep(50);
    int object = recognizeObject();
    if(object == 2){
        printf("The object is movable !!!!!!!!!!!!!!!!!!\n");
        return MOVABLE;
    }
    if (dist1 > 200 && dist2 > 200){
        printf("The object is movable !!!!!!!!!!!!!!!!!!\n");
        return MOVABLE;
    }
    else{
        printf("The object is non movable !!!!!!!!!!!!!!!!!!\n");
        return NON_MOVABLE;
    }

    
}

void findCenterOfObject(){
    int heading1;
    int heading2;
    int dist1;
    int dist2;
    turnWhileCheckDistance(35,heading1,dist1);
    Sleep(100);
    turnNumberOfDegsCorrected(turn_speed,-35);
    Sleep(100);
    turnWhileCheckDistance(-35,heading2,dist2);
    Sleep(100);
    turnNumberOfDegsCorrected(turn_speed,35);
    if(heading1 < heading2){
        turnToDegCorrected(turn_speed,heading1);
    }else{
        turnToDegCorrected(turn_speed,heading2);
    }
}


void checkIfCloseToWall(){
    float distanceLeft;
    float distanceRight;
    stopEngines();
    Sleep(50);
    turnNumberOfDegsCorrected(turn_speed,35);
    distanceLeft = getDistanceSensorValue();
    Sleep(50);
    turnNumberOfDegsCorrected(turn_speed,-35);
    Sleep(50);
    turnNumberOfDegsCorrected(turn_speed,-35);
    Sleep(50);
    distanceRight = getDistanceSensorValue();
    Sleep(50);
    turnNumberOfDegsCorrected(turn_speed,35);
    Sleep(50);

    if(distanceLeft <= SHOULD_AVOID && distanceRight <= SHOULD_AVOID){
        printf("Close both directions, turn around\n");
        turnNumberOfDegsCorrected(turn_speed,180);
        
    }
    else if(distanceLeft <= SHOULD_AVOID){
        printf("Close on the left, correct right\n");
        avoidObstacle(-20);
    }
    else if(distanceRight <= SHOULD_AVOID){
        printf("Close on the right, correct left\n");
        avoidObstacle(20);
        
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
    sortPositionsBasedOnDistance();
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

void avoidObstacle(int heading_diff){
    turnNumberOfDegsCorrected(turn_speed,heading_diff);
    runForeverCorrected(regular_speed);
    int aborted = checkIfObjectClose(1500);
    stopEngines();
    turnNumberOfDegsCorrected(turn_speed,-heading_diff);
    if(aborted == 1){
        turnNumberOfDegsCorrected(turn_speed,180);
    }
    //runForever(regular_speed);
}
int checkIfObjectClose(int msec){
    int sleep_time = 50;
    if (msec > 0){
        while(msec > 0) {
            float close_dist = getDistanceSensorValue();
            if (close_dist <= OBJECT_TO_CLOSE){
                return 1;
            }
            Sleep(sleep_time);
            msec -= sleep_time;
        }
    }
    return 0;
    
}










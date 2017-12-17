#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ev3.h"
#include "ev3_port.h"
#include "ev3_tacho.h"
#include "ev3_sensor.h"
#include "headers/EngineController.h"
#include "headers/SensorController.h"
#include <math.h>
#include <unistd.h>
#include <time.h>

#define Sleep( msec ) usleep(( msec ) * 1000 )
#define L_MOTOR_PORT      OUTPUT_C
#define L_MOTOR_EXT_PORT  EXT_PORT__NONE_
#define R_MOTOR_PORT      OUTPUT_B
#define R_MOTOR_EXT_PORT  EXT_PORT__NONE_
#define SPEED_LINEAR      75  /* Motor speed for linear motion, in percents */
#define SPEED_CIRCULAR    50  /* ... for circular motion */
#define DEGREE_TO_COUNT( d )  (( d ) * 260 / 90 )

#define DEGREE_ERROR_MARGIN 2
#define WHEEL_DIAMETER 5.6 // In centimetres


int max_speed;  /* Motor maximal speed */
int moving;   /* Current moving */

enum { L, R };
uint8_t motor[s]; /* Sequence numbers of motors */
uint8_t arm;


time_t start_drive_time;
time_t stop_drive_time;



enum {
    RUNNING,
    STOPPED
};


void discoverEngines(){
    uint8_t sn;
    char s[ 256 ];
    int i;
    for (i = 0; i < DESC_LIMIT; i++ ) {
        if ( ev3_tacho[ i ].type_inx != TACHO_TYPE__NONE_ ) {
            printf( "  type = %s\n", ev3_tacho_type( ev3_tacho[ i ].type_inx ));
			printf( "  port = %s\n", ev3_tacho_port_name( i, s ));
			printf("  port = %d %d\n", ev3_tacho_desc_port(i), ev3_tacho_desc_extport(i));
        }
    }
    int port=65;
    int counter = 0;
	for (port=65; port<69; port++){
	    if ( ev3_search_tacho_plugged_in(port,0, &sn, 0 )) {
            int speed;
            printf("LEGO engine found\n");
            printf("Sequence number %i\n",sn);
            get_tacho_max_speed( sn, &speed );
            motor[counter] = sn;
            counter++;
            max_speed = speed;
	
	    } else {
		    printf( "LEGO_EV3_M_MOTOR 1 is NOT found\n" );
        }
        printf("Port: %i\n",port);
    }
    arm = motor[2];

    return;
}

int initEngines(){
    while ( ev3_tacho_init() < 1 ) Sleep( 1000 );
    return 0;
}


int stopEngines(){
    moving = STOPPED;
    stop_drive_time = time(0);
    multi_set_tacho_command_inx( motor, TACHO_STOP );
}
int isRunning( void )
{
    FLAGS_T state = TACHO_STATE__NONE_;
    get_tacho_state_flags( motor[ L ], &state );
    if ( state != TACHO_STATE__NONE_ ) return ( 1 );
    get_tacho_state_flags( motor[ R ], &state );
    if ( state != TACHO_STATE__NONE_ ) return ( 1 );
    return ( 0 );
}
void runDistance(int speed,double distance){
    set_tacho_speed_sp( motor[ L ], speed );
    set_tacho_speed_sp( motor[ R ], speed );
    set_tacho_position_sp( motor[ L ], distance );
    set_tacho_position_sp( motor[ R ], distance );
    multi_set_tacho_command_inx( motor, TACHO_RUN_TO_REL_POS );
}

void runForever( int speed)
{
    set_tacho_speed_sp( motor[ L ], speed );
    set_tacho_speed_sp( motor[ R ], speed );
    multi_set_tacho_command_inx( motor, TACHO_RUN_FOREVER );
    start_drive_time = time(0);
    moving = RUNNING;
}
void runToRelPos( int speed,double distance, int h){  
    turnToDeg(max_speed*0.1,h);
    runDistance(speed,distance);

}
void runTimed( int speed, int ms )
{
    printf("Left engine %i\n",motor[L]);
    printf("Right engine %i\n",motor[R]);
    printf("Speed %i\n",speed);
    set_tacho_speed_sp( motor[ L ], speed );
    set_tacho_speed_sp( motor[ R ], speed );
    multi_set_tacho_time_sp( motor, ms );
    multi_set_tacho_command_inx( motor, TACHO_RUN_TIMED );
    printf("Should now be running\n");
    start_drive_time = time(0);
    moving = STOPPED;
}

void turnRight(int speed,int degrees){
    int deg1 = degToDist(degrees);
    int deg2 = degToDist(-degrees);
    set_tacho_speed_sp( motor[ L ], speed );
    set_tacho_speed_sp( motor[ R ], -speed );
    set_tacho_position_sp( motor[ L ], deg1 );
    set_tacho_position_sp( motor[ R ], deg2 );
    multi_set_tacho_command_inx( motor, TACHO_RUN_TO_REL_POS );
}

void turnLeft(int speed,int degrees){
    int deg1 = degToDist(degrees);
    int deg2 = degToDist(-degrees);
    set_tacho_speed_sp( motor[ L ], -speed );
    set_tacho_speed_sp( motor[ R ], speed );
    set_tacho_position_sp( motor[ L ], deg2 );
    set_tacho_position_sp( motor[ R ], deg1 );
    multi_set_tacho_command_inx( motor, TACHO_RUN_TO_REL_POS );
}
int getMaxSpeed(){
    return max_speed;
}

void waitForCommandToFinish(){
    FLAGS_T stateL;
    FLAGS_T stateR;
    do {
        get_tacho_state_flags( motor[L], &stateL );
        get_tacho_state_flags( motor[R], &stateR );
        
    } while ( stateR && stateL);
    
}


void raiseArm(){
    int degree = DEGREE_TO_COUNT(90);
    set_tacho_speed_sp( arm, max_speed * 0.2);
    set_tacho_position_sp( arm,degree);
    set_tacho_command_inx( arm, TACHO_RUN_TO_REL_POS );
}

void lowerArm(){
    int degree = DEGREE_TO_COUNT(-90);
    set_tacho_speed_sp( arm, max_speed * 0.2);
    set_tacho_position_sp( arm,degree);
    set_tacho_command_inx( arm, TACHO_RUN_TO_REL_POS );
}

int getLeftEngineState(){
    FLAGS_T stateL;

    get_tacho_state_flags( motor[L], &stateL );

    return stateL;
}
int getRightEngineState(){
    FLAGS_T state;

    get_tacho_state_flags( motor[L], &state );
    printf("State: %i",state);
    return state;
}
void turnToDeg(int speed,int target){
    
    int current_deg = getCompassDegrees();
    int diff;
    int left_diff;
    int right_diff;
    do{
        printf("Turning to degree : %i !!!!!!!!!!!\n",target);
        printf("Current degree is : %i\n",current_deg);
        /*
        if (abs(target - current_deg) < abs( current_deg -target) ){
            
        }else{
            
        }
        */
        if(target > current_deg){
            left_diff = (target - current_deg);
            right_diff = (target - current_deg) - 360;

        }else {
            right_diff = (target - current_deg);
            left_diff = 360 + (target - current_deg);
        }
        
        printf("This is left diff : %i , This is right diff %i\n",left_diff,right_diff);
        if (abs(left_diff) <= abs(right_diff)){
            diff = left_diff;
        }else if(abs(left_diff)  > abs(right_diff)){
            diff = right_diff;
        }

        //diff = (target - current_deg);
        
        printf("Degree difference: %i\n", diff);
        turnNumberOfDegs(speed,diff);
        waitForCommandToFinish();
        Sleep(1000);
        current_deg = getCompassDegrees();
    }while(abs(diff) > DEGREE_ERROR_MARGIN || abs(current_deg-target) > DEGREE_ERROR_MARGIN);    
}

void turnNumberOfDegs(int turn_speed, int degrees){
    int deg1 = DEGREE_TO_COUNT(degrees);
    int deg2 = DEGREE_TO_COUNT(-degrees);
    set_tacho_speed_sp( motor[ L ], turn_speed );
    set_tacho_speed_sp( motor[ R ], turn_speed);
    set_tacho_position_sp( motor[ L ], deg2 );
    set_tacho_position_sp( motor[ R ], deg1 );
    multi_set_tacho_command_inx( motor, TACHO_RUN_TO_REL_POS );
}

void turnNumberOfDegsCorrected(int speed,int degree){
    int current_deg = getCompassDegrees();
    int target = (current_deg + degree) % 360;

    turnToDeg(speed,target);
}
void correctError(){

}

void getDistanceSinceLastCheck(int distance,int error){

}
double getWheelDiameter(){
    return WHEEL_DIAMETER;
}
int degToDist(int deg){
    return M_PI * WHEEL_DIAMETER * deg /360;
}




















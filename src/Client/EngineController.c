#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ev3.h"
#include "ev3_port.h"
#include "ev3_tacho.h"
#include "ev3_sensor.h"
#include "./headers/EngineController.h"
#include <math.h>
#include <unistd.h>
#include <time.h>

#define Sleep( msec ) usleep(( msec ) * 1000 )
#define L_MOTOR_PORT      OUTPUT_C
#define L_MOTOR_EXT_PORT  EXT_PORT__NONE_
#define R_MOTOR_PORT      OUTPUT_B
#define R_MOTOR_EXT_PORT  EXT_PORT__NONE_
#define IR_CHANNEL        0
#define SPEED_LINEAR      75  /* Motor speed for linear motion, in percents */
#define SPEED_CIRCULAR    50  /* ... for circular motion */
#define DEGREE_TO_COUNT( d )  (( d ) * 260 / 90 )

int app_alive;
int max_speed;  /* Motor maximal speed */
int mode;  /* Driving mode */
int moving;   /* Current moving */
int command;  /* Command for the 'drive' coroutine */
int angle;    /* Angle of rotation */
uint8_t ir, touch;  /* Sequence numbers of sensors */
enum { L, R };
uint8_t motor[3]; /* Sequence numbers of motors */
uint8_t arm;
int currX;
int currY;
int start_deg;
int curr_deg;
time_t start_drive_time;
time_t stop_drive_time;
enum {
    MOVE_NONE,
    MOVE_FORWARD,
    MOVE_BACKWARD,
    TURN_LEFT,
    TURN_RIGHT,
    TURN_ANGLE,
    STEP_BACKWARD,
};


enum {
    MODE_REMOTE,  /* IR remote control */
    MODE_AUTO,    /* Self-driving */
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
            printf("Counter %i\n",counter);
            printf("Sequence number %i\n",sn);
            get_tacho_max_speed( sn, &speed );
            printf("Max speed: %i\n",speed);
            motor[counter] = sn;
            counter++;
            max_speed = speed;
	
	    } else {
		    printf( "LEGO_EV3_M_MOTOR 1 is NOT found\n" );
        }
        printf("Port: %i\n",port);
    }
    arm = motor[3];


    printf("%i\n",motor[L]);
    printf("%i\n",motor[R]);
    printf("lols\n");
    return;
}

int initEngines(){
    while ( ev3_tacho_init() < 1 ) Sleep( 1000 );
    currX = 0;
    currY = 0;
    return 0;
}


int stopEngines(){
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



void runForever( int speed)
{
    set_tacho_speed_sp( motor[ L ], speed );
    set_tacho_speed_sp( motor[ R ], speed );
    multi_set_tacho_command_inx( motor, TACHO_RUN_FOREVER );
    start_drive_time = time(0);
}
void runToRelPos( int speed,int x, int y )
{   int newX = currX - x;
    int newY = currY - y;
    int turnDeg = atan (newY/newX);
    int turn_speed = max_speed * 0.3;
    if(x<currX){
        turnLeft(turn_speed,turnDeg);
    }
    else{
        turnRight(turn_speed,turnDeg);
    }
    int dist = sqrt(pow(newX,2) + pow(newY,2));
    printf("Discance %i\n",dist);
    set_tacho_speed_sp( motor[ L ], speed );
    set_tacho_speed_sp( motor[ R ], speed );
    set_tacho_position_sp( motor[ L ], dist );
    set_tacho_position_sp( motor[ R ], dist );
    multi_set_tacho_command_inx( motor, TACHO_RUN_TO_REL_POS );
    start_drive_time = time(0);
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
}

int getXPos(){
    return currX;
}
int getYPos(){
    return currY;
}
void turnRight(int speed,int degrees){
    int deg1 = DEGREE_TO_COUNT(degrees);
    int deg2 = DEGREE_TO_COUNT(-degrees);
    curr_deg = (curr_deg - degrees ) % 360;
    set_tacho_speed_sp( motor[ L ], speed );
    set_tacho_speed_sp( motor[ R ], -speed );
    set_tacho_position_sp( motor[ L ], deg1 );
    set_tacho_position_sp( motor[ R ], deg2 );
    multi_set_tacho_command_inx( motor, TACHO_RUN_TO_REL_POS );
}

void turnLeft(int speed,int degrees){
    int deg1 = DEGREE_TO_COUNT(degrees);
    int deg2 = DEGREE_TO_COUNT(-degrees);
    curr_deg = (curr_deg + degrees ) % 360;
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
//Line eeeeee

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
















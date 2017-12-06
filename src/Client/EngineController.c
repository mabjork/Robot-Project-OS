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
int currX;
int currY;
int start_deg;
int curr_deg;

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

void runEngines(){
    
}

int stopEngines(){
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
}
void runToRelPos( int speed,int x, int y )
{   int newX = currX - x;
    int newY = currY - y;
    int turnDeg = atan (newY/newX);
    int dist = sqrt(pow(newX,2) + pow(newY,2));
    printf("Discance %i\n",dist);
    set_tacho_speed_sp( motor[ L ], speed );
    set_tacho_speed_sp( motor[ R ], speed );
    set_tacho_position_sp( motor[ L ], dist );
    set_tacho_position_sp( motor[ R ], dist );
    multi_set_tacho_command_inx( motor, TACHO_RUN_TO_REL_POS );
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
}

int* getPos(){

}
void turnRight(int speed,int degrees){
    int deg1 = DEGREE_TO_COUNT(degrees);
    int deg2 = DEGREE_TO_COUNT(-degrees);
    curr_deg = (curr_deg - degrees ) % 360;
    set_tacho_speed_sp( motor[ L ], speed );
    set_tacho_speed_sp( motor[ R ], speed );
    set_tacho_position_sp( motor[ L ], deg1 );
    set_tacho_position_sp( motor[ R ], deg2 );
    multi_set_tacho_command_inx( motor, TACHO_RUN_TO_REL_POS );
}
void turnLeft(int speed,int degrees){
    int deg1 = DEGREE_TO_COUNT(degrees);
    int deg2 = DEGREE_TO_COUNT(-degrees);
    curr_deg = (curr_deg + degrees ) % 360;
    set_tacho_speed_sp( motor[ L ], speed );
    set_tacho_speed_sp( motor[ R ], speed );
    set_tacho_position_sp( motor[ L ], deg2 );
    set_tacho_position_sp( motor[ R ], deg1 );
    multi_set_tacho_command_inx( motor, TACHO_RUN_TO_REL_POS );
}
int getMaxSpeed(){
    return max_speed;
}










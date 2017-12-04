#include <stdio.h>
#include <stdlib.h>

#include <stdarg.h>
#include "ev3.h"
#include "ev3_port.h"
#include "ev3_tacho.h"
#include "ev3_sensor.h"
#include "./headers/EngineController.h"


// WIN32 /////////////////////////////////////////
#ifdef __WIN32__
#include <windows.h>
// UNIX //////////////////////////////////////////
#else
#include <unistd.h>
#define Sleep( msec ) usleep(( msec ) * 1000 )
#endif

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
uint8_t motor[ 3 ] = { DESC_LIMIT, DESC_LIMIT, DESC_LIMIT };  /* Sequence numbers of motors */

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


int * discoverEngines(){

}

int initEngines(){
    while ( ev3_tacho_init() < 1 ) Sleep( 1000 );
    return 0;
}

void runEngines(){
    int i;
    uint8_t sn;
    FLAGS_T state;
    char s[ 256 ];

    //if ( ev3_init() == -1 ) return ( 1 );

    //

    for ( i = 0; i < DESC_LIMIT; i++ ) {
        if ( ev3_tacho[ i ].type_inx != TACHO_TYPE__NONE_ ) {
            printf( "  type = %s\n", ev3_tacho_type( ev3_tacho[ i ].type_inx ));
            printf( "  port = %s\n", ev3_tacho_port_name( i, s ));
        }
    }
    int port=65;
	for (port=65; port<69; port++){
	if ( ev3_search_tacho_plugged_in(port,0, &sn, 0 )) {
		int max_speed;

		printf( "LEGO_EV3_M_MOTOR 1 is found, run for 5 sec...\n" );
		get_tacho_max_speed( sn, &max_speed );
		printf("  max speed = %d\n", max_speed );
		set_tacho_stop_action_inx( sn, TACHO_COAST );
		set_tacho_speed_sp( sn, max_speed * 2 / 3 );
		set_tacho_time_sp( sn, 5000 );
		set_tacho_ramp_up_sp( sn, 2000 );
		set_tacho_ramp_down_sp( sn, 2000 );
		set_tacho_command_inx( sn, TACHO_RUN_TIMED );
		/* Wait tacho stop */
		Sleep( 100 );
		do {
			get_tacho_state_flags( sn, &state );
		} while ( state );
		printf( "run to relative position...\n" );
		set_tacho_speed_sp( sn, max_speed / 2 );
		set_tacho_ramp_up_sp( sn, 0 );
		set_tacho_ramp_down_sp( sn, 0 );
		set_tacho_position_sp( sn, 90 );
		for ( i = 0; i < 8; i++ ) {
			set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
			Sleep( 500 );
		}
	
	} else {
		printf( "LEGO_EV3_M_MOTOR 1 is NOT found\n" );
	}
	}
    return ( 0 );
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

void setMode( int value )
{
    if ( value == MODE_AUTO ) {
        /* IR measuring of distance */
        set_sensor_mode_inx( ir, LEGO_EV3_IR_IR_PROX );
        mode = MODE_AUTO;
    } else {
        /* IR remote control */
        set_sensor_mode_inx( ir, LEGO_EV3_IR_IR_REMOTE );
        mode = MODE_REMOTE;
    }
}


void runForever( int l_speed, int r_speed )
{
    set_tacho_speed_sp( motor[ L ], l_speed );
    set_tacho_speed_sp( motor[ R ], r_speed );
    multi_set_tacho_command_inx( motor, TACHO_RUN_FOREVER );
}
void runToRelPos( int l_speed, int l_pos, int r_speed, int r_pos )
{
    set_tacho_speed_sp( motor[ L ], l_speed );
    set_tacho_speed_sp( motor[ R ], r_speed );
    set_tacho_position_sp( motor[ L ], l_pos );
    set_tacho_position_sp( motor[ R ], r_pos );
    multi_set_tacho_command_inx( motor, TACHO_RUN_TO_REL_POS );
}
void runTimed( int l_speed, int r_speed, int ms )
{
    set_tacho_speed_sp( motor[ L ], l_speed );
    set_tacho_speed_sp( motor[ R ], r_speed );
    multi_set_tacho_time_sp( motor, ms );
    multi_set_tacho_command_inx( motor, TACHO_RUN_TIMED );
}








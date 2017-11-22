#include <stdio.h>
#include <stdlib.h>

#include <stdarg.h>
#include "ev3.h"
#include "ev3_port.h"
#include "ev3_tacho.h"
#include "./headers/EngineController.h"
#include "coroutine.h"

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





int * discoverEngines(){

}

bool initEngines(){
    int counter = 0;
    while ( ev3_tacho_init() < 1 ) {
        Sleep( 1000 );
        counter++;
        if(counter==10)return false;
    }
    return true;
}

void runEngines(int * engines, int time){
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
    if ( ev3_search_tacho( LEGO_EV3_M_MOTOR, &sn, 0 )) {
        int max_speed;
        printf( "LEGO_EV3_M_MOTOR is found, run for 5 sec...\n" );
        get_tacho_max_speed( sn, &max_speed );
        printf("  max_speed = %d\n", max_speed );
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
        printf( "LEGO_EV3_M_MOTOR is NOT found\n" );
    }
    ev3_uninit();
    printf( "*** ( EV3 ) Bye! ***\n" );
    return ( 0 );
}

bool stopEngines(){
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

CORO_CONTEXT( handle_touch );
CORO_CONTEXT( handle_brick_control );
CORO_CONTEXT( handle_ir_control );
CORO_CONTEXT( handle_ir_proximity );
CORO_CONTEXT( drive );


/* Coroutine of the TOUCH sensor handling */
CORO_DEFINE( handle_touch ){
        CORO_LOCAL int val;
        CORO_BEGIN();
        if ( touch == DESC_LIMIT ) CORO_QUIT();
        for ( ; ; ){
            /* Waiting the button is pressed */
            CORO_WAIT(get_sensor_value(0, touch, &val) && (val));
            /* Stop the vehicle */
            command = MOVE_NONE;
            /* Switch mode */
            _set_mode((mode == MODE_REMOTE) ? MODE_AUTO : MODE_REMOTE);
            /* Waiting the button is released */
            CORO_WAIT(get_sensor_value(0, touch, &val) && (!val));
        }
        CORO_END();
}


/* Coroutine of the EV3 brick keys handling */
CORO_DEFINE( handle_brick_control )
        {
                CORO_LOCAL uint8_t keys, pressed = EV3_KEY__NONE_;
        CORO_BEGIN();
        for ( ; ; ) {
            /* Waiting any key is pressed or released */
            CORO_WAIT( ev3_read_keys( &keys ) && ( keys != pressed ));
            pressed = keys;
            if ( pressed & EV3_KEY_BACK ) {
                /* Stop the vehicle */
                command = MOVE_NONE;
                /* Quit */
                app_alive = 0;
            } else if ( pressed & EV3_KEY_UP ) {
                /* Stop the vehicle */
                command = MOVE_NONE;
                /* Switch mode */
                _set_mode(( mode == MODE_REMOTE ) ? MODE_AUTO : MODE_REMOTE );
            }
            CORO_YIELD();
        }
        CORO_END();
        }

/* Coroutine of IR remote control handling */
CORO_DEFINE( handle_ir_control )
        {
                CORO_LOCAL int val;
        CORO_BEGIN();
        for ( ; ; ) {
            /* Waiting IR remote control mode */
            CORO_WAIT( mode == MODE_REMOTE );
            val = IR_REMOTE__NONE_;
            get_sensor_value( IR_CHANNEL, ir, &val );
            switch ( val ) {
                /* Forward */
                case RED_UP_BLUE_UP:
                    command = MOVE_FORWARD;
                    break;
                    /* Backward */
                case RED_DOWN_BLUE_DOWN:
                    command = MOVE_BACKWARD;
                    break;
                    /* Left */
                case RED_UP:
                case RED_UP_BLUE_DOWN:
                case BLUE_DOWN:
                    command = TURN_LEFT;
                    break;
                    /* Right */
                case BLUE_UP:
                case RED_DOWN_BLUE_UP:
                case RED_DOWN:
                    command = TURN_RIGHT;
                    break;
                    /* Stop */
                case IR_REMOTE__NONE_:
                case RED_UP_RED_DOWN:
                case BLUE_UP_BLUE_DOWN:
                case BEACON_MODE_ON:
                    command = MOVE_NONE;
                    break;
            }
            CORO_YIELD();
        }
        CORO

        _END();
        }
/* Coroutine of IR proximity handling (self-driving),
   based on Franz Detro drive_test.cpp */
CORO_DEFINE( handle_ir_proximity )
        {
                CORO_LOCAL int front, prox;
        CORO_BEGIN();
        for ( ; ; ) {
            /* Waiting self-driving mode */
            CORO_WAIT( mode == MODE_AUTO );
            prox = 0;
            get_sensor_value( 0, ir, &prox );
            if ( prox == 0 ) {
                /* Oops! Stop the vehicle */
                command = MOVE_NONE;
            } else if ( prox < 20 ) {  /* Need for detour... */
                front = prox;
                /* Look to the left */
                angle = -30;
                do {
                    command = TURN_ANGLE;
                    CORO_WAIT( command == MOVE_NONE );
                    prox = 0;
                    get_sensor_value( 0, ir, &prox );
                    if ( prox < front ) {
                        if ( angle < 0 ) {
                            /* Still looking to the left - look to the right */
                            angle = 60;
                        } else {
                            /* Step back */
                            command = STEP_BACKWARD;
                            CORO_WAIT( command == MOVE_NONE );
                        }
                    }
                } while (( prox > 0 ) && ( prox < 40 ) && ( mode == MODE_AUTO ));
            } else {
                /* Track is clear - Go! */
                command = MOVE_FORWARD;
            }
            CORO_YIELD();
        }
        CORO_END();
        }


/* Coroutine of control the motors */
CORO_DEFINE( drive )
        {
                CORO_LOCAL int speed_linear, speed_circular;
        CORO_LOCAL int _wait_stopped;
        CORO_BEGIN();
        speed_linear = max_speed * SPEED_LINEAR / 100;
        speed_circular = max_speed * SPEED_CIRCULAR / 100;
        for ( ; ; ) {
            /* Waiting new command */
            CORO_WAIT( moving != command );
            _wait_stopped = 0;
            switch ( command ) {
                case MOVE_NONE:
                    _stop();
                    _wait_stopped = 1;
                    break;
                case MOVE_FORWARD:
                    _run_forever( -speed_linear, -speed_linear );
                    break;
                case MOVE_BACKWARD:
                    _run_forever( speed_linear, speed_linear );
                    break;
                case TURN_LEFT:
                    _run_forever( speed_circular, -speed_circular );
                    break;
                case TURN_RIGHT:
                    _run_forever( -speed_circular, speed_circular );
                    break;
                case TURN_ANGLE:
                    _run_to_rel_pos( speed_circular, DEGREE_TO_COUNT( -angle )
                            , speed_circular, DEGREE_TO_COUNT( angle ));
                    _wait_stopped = 1;
                    break;
                case STEP_BACKWARD:
                    _run_timed( speed_linear, speed_linear, 1000 );
                    _wait_stopped = 1;
                    break;
            }
            moving = command;
            if ( _wait_stopped ) {
                /* Waiting the command is completed */
                CORO_WAIT( !_is_running());
                command = moving = MOVE_NONE;
            }
        }
        CORO_END();
        }

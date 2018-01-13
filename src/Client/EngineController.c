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
#define RIGHT 66
#define LEFT 65
#define BALL 68

#define Sleep( msec ) usleep(( msec ) * 1000 )
#define L_MOTOR_PORT      OUTPUT_C
#define L_MOTOR_EXT_PORT  EXT_PORT__NONE_
#define R_MOTOR_PORT      OUTPUT_B
#define R_MOTOR_EXT_PORT  EXT_PORT__NONE_
#define SPEED_LINEAR      75  /* Motor speed for linear motion, in percents */
#define SPEED_CIRCULAR    50  /* ... for circular motion */
#define DEGREE_TO_COUNT( d )  (( d ) * 260 / 180 )

#define DEGREE_ERROR_MARGIN 2
#define WHEEL_DIAMETER 5.6 // In centimetres


int max_speed;  /* Motor maximal speed */
int moving;   /* Current moving */
int stop_turn;
uint8_t arm;
uint8_t sn_engineR;
uint8_t sn_engineL;
uint8_t sn_engineLR[3];
uint8_t sn_engineM;
uint8_t sn_engineLRM[4];

enum {
    RUNNING,
    STOPPED
};

/*
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
*/

int initEngines(){
    if (!ev3_tacho_init()){
        perror("ev3_tacho_init() failed! Aborting...");
        abort();
    }
    if (!ev3_search_tacho_plugged_in( LEFT, 0, &sn_engineL, 0 )) {
        perror("Left engine not found! Aborting...");
        abort();
    }
    if (!ev3_search_tacho_plugged_in( RIGHT, 0, &sn_engineR, 0)){
        perror("Right engine not found! Aborting...");
        abort();
    }
    if (!ev3_search_tacho_plugged_in( BALL, 0, &sn_engineM, 0)){
        perror("Middle engine not found! Aborting...");
        abort();
    } 
    sn_engineLR[0] = sn_engineL;
    sn_engineLR[1] = sn_engineR;
    sn_engineLR[2] = DESC_LIMIT;

    sn_engineLRM[0] = sn_engineL;
    sn_engineLRM[1] = sn_engineR;
    sn_engineLRM[2] = sn_engineM;
    sn_engineLRM[3] = DESC_LIMIT;
    stop_turn == 0;
    int speed;
    get_tacho_max_speed( sn_engineL, &speed );
    max_speed = speed;
    // Set default value of ramp up/down speed
    multi_set_tacho_ramp_up_sp( sn_engineLR, 1000 ); // TODO: Check this value
    multi_set_tacho_ramp_down_sp( sn_engineLR, 0 ); // TODO: Check this value
    set_tacho_ramp_up_sp(sn_engineM, 1000);
    set_tacho_ramp_down_sp(sn_engineM, 1000);
    multi_set_tacho_stop_action_inx( sn_engineLRM, TACHO_HOLD );

    return 0;
}


int stopEngines(){
    moving = STOPPED;
    multi_set_tacho_command_inx( sn_engineLRM, TACHO_STOP );
    return;
}
int isRunning( void )
{
    FLAGS_T state = TACHO_STATE__NONE_;
    get_tacho_state_flags( sn_engineL, &state );
    if ( state != TACHO_STATE__NONE_ ) return ( 1 );
    get_tacho_state_flags( sn_engineR, &state );
    if ( state != TACHO_STATE__NONE_ ) return ( 1 );
    return ( 0 );
}
void runDistance(int speed,double distance){
    set_tacho_speed_sp(sn_engineL, speed );
    set_tacho_speed_sp( sn_engineR, speed );
    set_tacho_position_sp( sn_engineL, distance );
    set_tacho_position_sp( sn_engineR, distance );
    multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_TO_REL_POS );
}

void runForever( int speed)
{
    set_tacho_speed_sp( sn_engineL, speed );
    set_tacho_speed_sp( sn_engineR, speed );
    multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
    moving = RUNNING;
}
void runToRelPos( int speed,double distance, int h){  
    turnToDeg(max_speed*0.1,h);
    runDistance(speed,distance);

}
void runTimed( int speed, int mseconds )
{
    int sleep_time = 250; // [ms]
    multi_set_tacho_stop_action_inx( sn_engineLR, TACHO_BRAKE );
    multi_set_tacho_polarity_inx( sn_engineLR, TACHO_NORMAL);
    multi_set_tacho_speed_sp( sn_engineLR, speed );
    multi_set_tacho_time_sp( sn_engineLR, mseconds);

    if ( mseconds > 0 ) {
        int initial_angle = getGyroDegrees();
        int current_angle = initial_angle;
        int error;
        multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_TIMED );

        // TODO: REPLACE THIS WITH A PROPER PID CONTROLER!
        while (mseconds > 0){
            current_angle = getGyroDegrees();
            error = current_angle-initial_angle;
            if (error > 1 || error < -1) {
                set_tacho_speed_sp(sn_engineR, speed+(error*2));
                set_tacho_speed_sp(sn_engineL, speed-(error*2));
                multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_TIMED ); // TODO: FIX TIMED!
            }
            mseconds-=sleep_time;
            Sleep(sleep_time);        
        }
        multi_set_tacho_command_inx( sn_engineLR, TACHO_STOP );
    } else {
        multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
    }
    return;
}
/*
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
*/
int getMaxSpeed(){
    return max_speed;
}

void waitForCommandToFinish(){
    FLAGS_T stateL;
    FLAGS_T stateR;
    do {
        get_tacho_state_flags( sn_engineL, &stateL );
        get_tacho_state_flags( sn_engineR, &stateR );
        
    } while ( stateR && stateL);
    
}


void raiseArm(){
    int degree = DEGREE_TO_COUNT(60);
    set_tacho_speed_sp( sn_engineM, max_speed * 0.25);
    set_tacho_position_sp( sn_engineM,degree);
    set_tacho_command_inx( sn_engineM, TACHO_RUN_TO_REL_POS );
}

void lowerArm(){
    int degree = DEGREE_TO_COUNT(-60);
    set_tacho_speed_sp( sn_engineM, max_speed * 0.01);
    set_tacho_position_sp( sn_engineM,degree);
    set_tacho_command_inx( sn_engineM, TACHO_RUN_TO_REL_POS );
}

void turnToDeg(int speed,int target){
    printf("Turning number of degs !!!!!!");
    int current_deg = getGyroDegrees();
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
        Sleep(1500);
        current_deg = getGyroDegrees();
    }while(abs(diff) > DEGREE_ERROR_MARGIN);  
    printf("DONE!!!\n");
}

void turnNumberOfDegs(int turn_speed,int degrees){
    //printf("Turning number of degs !!!!!!");
    int deg = DEGREE_TO_COUNT(degrees);
    int deg2 = DEGREE_TO_COUNT(-degrees);
    set_tacho_speed_sp( sn_engineL, turn_speed);
    set_tacho_speed_sp( sn_engineR, turn_speed);
    set_tacho_position_sp( sn_engineL, deg2 );
    set_tacho_position_sp( sn_engineR, deg );
    multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_TO_REL_POS );
    printf("Should drive \n");
}


void turnNumberOfDegsCorrected(int speed,int x){
    /*
    int current_deg = getGyroDegrees();
    int target = (current_deg + degree) % 360;
    if(target < 0){
        target += 360;
    }
    printf("Should turn to %i\n",target);
    turnToDeg(speed,target);
    */
    stopEngines();
    printf("Turning2 by: %d\n", x);
    stop_turn = 0;

    if ( x > 0 ) {
        set_tacho_polarity_inx( sn_engineL, TACHO_INVERSED);
        set_tacho_polarity_inx( sn_engineR, TACHO_NORMAL);
    } else  if ( x < 0 ) {
        set_tacho_polarity_inx( sn_engineL, TACHO_NORMAL);
        set_tacho_polarity_inx( sn_engineR, TACHO_INVERSED);
    } else {
        return;
    }

    multi_set_tacho_stop_action_inx( sn_engineLR, TACHO_HOLD );
    multi_set_tacho_speed_sp( sn_engineLR, 250);
    multi_set_tacho_command_inx( sn_engineLR, TACHO_STOP );

    // Active

    int current_angle = -getGyroDegrees();
    int target_angle = current_angle + x;

    // Start the active engine
    multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );

    // Gyro control loop
    int deg_left = target_angle - current_angle;
    int deg_left_abs = abs(deg_left);
    int stage = 3;
    while (stop_turn == 0 && ((deg_left > 0 && x > 0 ) || ((deg_left < 0) && x < 0))) { // TODO: Check gyro value +/-
        current_angle = -getGyroDegrees();
        deg_left = target_angle - current_angle;
        deg_left_abs = abs(deg_left);

        printf("TURN: T: %d C: %d Deg_Left: %d\n",target_angle,current_angle,deg_left);

        if ( stage == 1 && 0 < deg_left_abs && deg_left_abs <= 10 ) {
            multi_set_tacho_speed_sp( sn_engineLR, 20);
            multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
            stage--;
        } else if ( stage == 2 && deg_left_abs <= 25 ) {
            multi_set_tacho_speed_sp( sn_engineLR, 75);
            multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
            stage--;
        } else if ( stage == 3 && deg_left_abs <= 35 ) {
            multi_set_tacho_speed_sp( sn_engineLR, 200);
            multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
            stage--;
        }
        Sleep(50);
    }
    // HALT!
    multi_set_tacho_polarity_inx( sn_engineLR, TACHO_NORMAL);
    multi_set_tacho_command_inx( sn_engineLR, TACHO_STOP);
    Sleep(500); 

    return;
}

void turnToDegCorrected(int target){
    int current_deg = getGyroDegrees();
    int diff;
    int left_diff;
    int right_diff;

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
    int x = diff;

    stopEngines();
    printf("Turning2 by: %d\n", x);
    stop_turn = 0;

    if ( x > 0 ) {
        set_tacho_polarity_inx( sn_engineL, TACHO_INVERSED);
        set_tacho_polarity_inx( sn_engineR, TACHO_NORMAL);
    } else  if ( x < 0 ) {
        set_tacho_polarity_inx( sn_engineL, TACHO_NORMAL);
        set_tacho_polarity_inx( sn_engineR, TACHO_INVERSED);
    } else {
        return;
    }

    multi_set_tacho_stop_action_inx( sn_engineLR, TACHO_HOLD );
    multi_set_tacho_speed_sp( sn_engineLR, 250);
    multi_set_tacho_command_inx( sn_engineLR, TACHO_STOP );

    // Active

    int current_angle = -getGyroDegrees();
    int target_angle = current_angle + x;

    // Start the active engine
    multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );

    // Gyro control loop
    int deg_left = target_angle - current_angle;
    int deg_left_abs = abs(deg_left);
    int stage = 3;
    while (stop_turn == 0 && ((deg_left > 0 && x > 0 ) || ((deg_left < 0) && x < 0))) { // TODO: Check gyro value +/-
        current_angle = -getGyroDegrees();
        deg_left = target_angle - current_angle;
        deg_left_abs = abs(deg_left);

        printf("TURN: T: %d C: %d Deg_Left: %d\n",target_angle,current_angle,deg_left);

        if ( stage == 1 && 0 < deg_left_abs && deg_left_abs <= 10 ) {
            multi_set_tacho_speed_sp( sn_engineLR, 20);
            multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
            stage--;
        } else if ( stage == 2 && deg_left_abs <= 25 ) {
            multi_set_tacho_speed_sp( sn_engineLR, 75);
            multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
            stage--;
        } else if ( stage == 3 && deg_left_abs <= 35 ) {
            multi_set_tacho_speed_sp( sn_engineLR, 200);
            multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
            stage--;
        }
        Sleep(50);
    }
    // HALT!
    multi_set_tacho_polarity_inx( sn_engineLR, TACHO_NORMAL);
    multi_set_tacho_command_inx( sn_engineLR, TACHO_STOP);
    Sleep(500); 

    return;
}

double getWheelDiameter(){
    return WHEEL_DIAMETER;
}
int degToDist(int deg){
    return M_PI * WHEEL_DIAMETER * deg /360;
}

void turn2( int x)
{
    stopEngines();
    printf("Turning2 by: %d\n", x);
    stop_turn = 0;

    if ( x > 0 ) {
        set_tacho_polarity_inx( sn_engineL, TACHO_INVERSED);
        set_tacho_polarity_inx( sn_engineR, TACHO_NORMAL);
    } else  if ( x < 0 ) {
        set_tacho_polarity_inx( sn_engineL, TACHO_NORMAL);
        set_tacho_polarity_inx( sn_engineR, TACHO_INVERSED);
    } else {
        return;
    }

    multi_set_tacho_stop_action_inx( sn_engineLR, TACHO_HOLD );
    multi_set_tacho_speed_sp( sn_engineLR, 250);
    multi_set_tacho_command_inx( sn_engineLR, TACHO_STOP );

    // Active

    int current_angle = -getGyroDegrees();
    int target_angle = current_angle + x;

    // Start the active engine
    multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );

    // Gyro control loop
    int deg_left = target_angle - current_angle;
    int deg_left_abs = abs(deg_left);
    int stage = 3;
    while (stop_turn == 0 && ((deg_left > 0 && x > 0 ) || ((deg_left < 0) && x < 0))) { // TODO: Check gyro value +/-
        current_angle = -getGyroDegrees();
        deg_left = target_angle - current_angle;
        deg_left_abs = abs(deg_left);

        printf("TURN: T: %d C: %d Deg_Left: %d\n",target_angle,current_angle,deg_left);

        if ( stage == 1 && 0 < deg_left_abs && deg_left_abs <= 10 ) {
            multi_set_tacho_speed_sp( sn_engineLR, 20);
            multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
            stage--;
        } else if ( stage == 2 && deg_left_abs <= 25 ) {
            multi_set_tacho_speed_sp( sn_engineLR, 75);
            multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
            stage--;
        } else if ( stage == 3 && deg_left_abs <= 35 ) {
            multi_set_tacho_speed_sp( sn_engineLR, 200);
            multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
            stage--;
        }
        Sleep(50);
    }
    // HALT!
    multi_set_tacho_polarity_inx( sn_engineLR, TACHO_NORMAL);
    multi_set_tacho_command_inx( sn_engineLR, TACHO_STOP);
    Sleep(500); 

    return;
}
void get_encoders_values(int * disp_left, int * disp_right){
    get_tacho_position(sn_engineL, disp_left);
    get_tacho_position(sn_engineR, disp_right);
}

void adjust_speed_by(int err){
    set_tacho_speed_sp(sn_engineR, max_speed+(err*1));
    set_tacho_speed_sp(sn_engineL, max_speed-(err*1));
    multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
}




















#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ev3.h"
#include "ev3_port.h"
#include "ev3_tacho.h"
#include "ev3_sensor.h"
#include "headers/EngineController.h"
#include "headers/SensorController.h"
#include "headers/PositionController.h"
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
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
 // In centimetres


int max_speed;  /* Motor maximal speed */
int moving;   /* Current moving */
int stop_turn;
uint8_t arm;
uint8_t sn_engineR;
uint8_t sn_engineL;
uint8_t sn_engineLR[3];
uint8_t sn_engineM;
uint8_t sn_engineLRM[4];

pthread_t engine_tid;
pthread_mutex_t engine_lock;

enum {
    RUNNING,
    STOPPED
};



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
    pthread_mutex_init(&engine_lock, NULL);
    stopp_engine_thread = 1;
    current_speed = 0;
    pthread_mutex_unlock(&engine_lock);
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
    printf("Distance is %lf\n",distance);
    int run_time = 100000 * distance/(speed * WHEEL_DIAMETER/2);
    printf("Running for time %i\n",run_time);
    runTimed(speed,run_time);
}

void *runForever(void *args)
{
    int speed = *((int *) args);
    pthread_mutex_init(&engine_lock, NULL);
    current_speed = speed;
    pthread_mutex_unlock(&engine_lock);
    free(args);
    int sleep_time = 100; // [ms]
    multi_set_tacho_stop_action_inx( sn_engineLR, TACHO_BRAKE );
    multi_set_tacho_polarity_inx( sn_engineLR, TACHO_NORMAL);
    multi_set_tacho_speed_sp( sn_engineLR, speed );
    int initial_angle = getGyroDegrees();
    int current_angle = initial_angle;
    int error;
    multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );

    // TODO: REPLACE THIS WITH A PROPER PID CONTROLER!
    while (stopp_engine_thread == 0){
        current_angle = getGyroDegrees();
        error = current_angle-initial_angle;
        if (error > 1 || error < -1) {
            set_tacho_speed_sp(sn_engineR, speed+(error*2));
            set_tacho_speed_sp(sn_engineL, speed-(error*2));
            multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
        }
        Sleep(sleep_time);        
    }
    multi_set_tacho_command_inx( sn_engineLR, TACHO_STOP );
    
    pthread_join(engine_tid, NULL);
    //return;
}
void runToRelPos( int speed,double distance, int heading_change){  
    turn2(heading_change);
    runDistance(speed,distance);
    
}
void runTimed( int speed, int mseconds )
{
    pthread_mutex_init(&engine_lock, NULL);
    current_speed = speed;
    pthread_mutex_unlock(&engine_lock);
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
void backAwayTimed(int speed,int mseconds){
    pthread_mutex_init(&engine_lock, NULL);
    current_speed = -speed;
    pthread_mutex_unlock(&engine_lock);
    int sleep_time = 100; // [ms]
    multi_set_tacho_stop_action_inx( sn_engineLR, TACHO_BRAKE );
    multi_set_tacho_polarity_inx( sn_engineLR, TACHO_INVERSED);
    multi_set_tacho_speed_sp( sn_engineLR, speed );
    multi_set_tacho_time_sp( sn_engineLR, mseconds );

    if ( mseconds > 0 ) {
        int initial_angle = getGyroDegrees();
        int current_angle = initial_angle;
        //int mseconds = seconds * 1000;
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

 


void turnNumberOfDegsCorrected(int speed,int x){

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
            multi_set_tacho_speed_sp( sn_engineLR, 50);
            multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
            stage--;
        } else if ( stage == 3 && deg_left_abs <= 35 ) {
            multi_set_tacho_speed_sp( sn_engineLR, 150);
            multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
            stage--;
        }
        Sleep(50);
    }
    Sleep(200);
    pthread_mutex_init(&engine_lock, NULL);
    calibrateGyro();
    last_gyro_read = getGyroDegrees();
    pthread_mutex_unlock(&engine_lock);
    // HALT!
    multi_set_tacho_polarity_inx( sn_engineLR, TACHO_NORMAL);
    multi_set_tacho_command_inx( sn_engineLR, TACHO_STOP);
    Sleep(500); 

    return;
}

void turnToDegCorrected(int speed,float target){
    printf("Should turn to %i\n",target);
    float current_deg = HEADING;
    printf("This is the current heading %f\n",current_deg);
    float diff;
    float left_diff;
    float right_diff;

    if(target > current_deg){
        left_diff = (target - current_deg);
        right_diff = (target - current_deg) - 360;

    }else {
        right_diff = (target - current_deg);
        left_diff = 360 + (target - current_deg);
    }
        
    printf("This is left diff : %i , This is right diff %i\n",left_diff,right_diff);
    if (fabs(left_diff) <= fabs(right_diff)){
        diff = left_diff;
    }else if(fabs(left_diff)  > fabs(right_diff)){
        diff = right_diff;
    }
    int x;
    if(diff < 0){
        x = ceil(diff);
    }else{
        x = floor(diff);
    }
    

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
    multi_set_tacho_speed_sp( sn_engineLR, 200);
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
            multi_set_tacho_speed_sp( sn_engineLR, 50);
            multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
            stage--;
        } else if ( stage == 3 && deg_left_abs <= 35 ) {
            multi_set_tacho_speed_sp( sn_engineLR, 150);
            multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
            stage--;
        }
        Sleep(50);
    }
    Sleep(200);
    pthread_mutex_init(&engine_lock, NULL);
    calibrateGyro();
    last_gyro_read = getGyroDegrees();
    pthread_mutex_unlock(&engine_lock);
    
    // HALT!
    multi_set_tacho_polarity_inx( sn_engineLR, TACHO_NORMAL);
    multi_set_tacho_command_inx( sn_engineLR, TACHO_STOP);
    Sleep(500); 

    return;
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
    multi_set_tacho_speed_sp( sn_engineLR, 200);
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
            multi_set_tacho_speed_sp( sn_engineLR, 50);
            multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
            stage--;
        } else if ( stage == 3 && deg_left_abs <= 35 ) {
            multi_set_tacho_speed_sp( sn_engineLR, 150);
            multi_set_tacho_command_inx( sn_engineLR, TACHO_RUN_FOREVER );
            stage--;
        }
        Sleep(50);
    }
    pthread_mutex_init(&engine_lock, NULL);
    calibrateGyro();
    last_gyro_read = getGyroDegrees();
    pthread_mutex_unlock(&engine_lock);
    // HALT!
    multi_set_tacho_polarity_inx( sn_engineLR, TACHO_NORMAL);
    multi_set_tacho_command_inx( sn_engineLR, TACHO_STOP);
    Sleep(100); 

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
void runForeverCorrected(int speed){
    int *arg = malloc(sizeof(int));
    pthread_mutex_init(&engine_lock, NULL);
    stopp_engine_thread = 0;
    pthread_mutex_unlock(&engine_lock);
    *arg = speed;
    pthread_create(&engine_tid, NULL, runForever, arg);
    printf("Thread created\n");
}






















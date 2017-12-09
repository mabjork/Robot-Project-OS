#include <string.h>
#include <stdio.h>
#include "ev3.h"
#include "ev3_port.h"
#include "ev3_sensor.h"
#include "headers/SensorController.h"

// WIN32 /////////////////////////////////////////
#ifdef __WIN32__

#include <windows.h>

// UNIX //////////////////////////////////////////
#else

#include <unistd.h>
#define Sleep( msec ) usleep(( msec ) * 1000 )

//////////////////////////////////////////////////
#endif

const char const *color[] = { "?", "BLACK", "BLUE", "GREEN", "YELLOW", "RED", "WHITE", "BROWN" };
#define COLOR_COUNT  (( int )( sizeof( color ) / sizeof( color[ 0 ])))

//# Functions -----------------------------------------------------------------

//# Initialize Sensors ------------------------------
void initSensors(){
  ev3_sensor_init();
}


//# COLOR SENSOR ------------------------------------
int getColor(){
	int val;
  uint8_t sn_color;
  
  if ( ev3_search_sensor( LEGO_EV3_COLOR, &sn_color, 0 )) {
    if ( ev3_search_sensor( LEGO_EV3_COLOR, &sn_color, 0 )) {
        printf( "COLOR sensor is found, reading COLOR...\n" );
        set_sensor_mode( sn_color, "COL-COLOR" );
        for ( ; ; ) {
          if ( !get_sensor_value( 0, sn_color, &val ) || ( val < 0 ) || ( val >= COLOR_COUNT )) {
            val = 0;
          }
          printf( "\r(%s)", color[ val ]);
          fflush( stdout );
          Sleep( 200 );
        }
    } else {
        printf( "COLOR sensor is NOT found\n" );
        //while ( !_check_pressed( sn_touch )) Sleep( 100 );
    }
  }
  return val;
}

recognizeObject(){

}




//# GYRO SENSOR -------------------------------------
int getAngle(){
	int val;
  uint8_t sn_gyro;
  if ( ev3_search_sensor( LEGO_EV3_GYRO, &sn_gyro, 0 )) {
    set_sensor_mode( sn_gyro, "GYRO-ANG" );
  }
  return val;
}

resetGyro(){

}

//# SONAR SENSOR ------------------------------------
int getDistanceSensorValue(){
	int value;
  uint8_t sn_sonar;
  if ( ev3_search_sensor( LEGO_EV3_US, &sn_sonar, 0 )) {
    set_sensor_mode( sn_sonar, "US-DIST-CM" );
    if ( !get_sensor_value0(sn_sonar, &value )) {
				value = -1;
			}
  }
  return value;
  
}




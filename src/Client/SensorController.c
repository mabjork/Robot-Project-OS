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
      printf( "COLOR sensor is found, reading COLOR...\n" );
      set_sensor_mode( sn_color, "COL-COLOR" );
      if ( !get_sensor_value( 0, sn_color, &val ) || ( val < 0 ) || ( val >= COLOR_COUNT )) {
        val = 0;
      }
      printf( "\r(%d) \n", val);
      fflush( stdout );
  } else {
      printf( "COLOR sensor is NOT found\n" );
  }
  
  return val;
}

int recognizeObject(){
  int obj_color;

  obj_color = getColor();
  //Black color, recognized as non-moveable obj
  if ( obj_color == 1){
    printf( "Black detected. This is a non-moveable object.\n");
    return 1;
  }

  //Red color detected, recognized as moveable obj
  else if ( obj_color == 5){
    printf( "Red detected. This is a moveable object.\n");
    return 2;
  }

  else if( obj_color == 2){
    printf("Blue detected. This is a moveable object.\n");
    return 2;
  }
  
  //Recognizes nothing. This may be changed
  else { 
    printf("No color detected.\n");
    return 0; 
    }

}


//# GYRO SENSOR -------------------------------------

int getGyroDegrees(){
  int val;
  uint8_t sn_gyro;
  if ( ev3_search_sensor( LEGO_EV3_GYRO, &sn_gyro, 0 )) {
    if ( !get_sensor_value0( sn_gyro, &val )) {
      val = 0;
    }
    printf( "\r(%d) \n", val);
    fflush( stdout );

  }
  return val;
}


void calibrateGyro(){

  uint8_t sn_gyro;
  set_sensor_mode( sn_gyro, "GYRO-RATE" );
  set_sensor_mode( sn_gyro, "GYRO-ANG" );
  Sleep(100);

}

//# SONAR SENSOR ------------------------------------

float getDistanceSensorValue(){
	float val;

  uint8_t sn_sonar;
  if ( ev3_search_sensor( LEGO_EV3_US, &sn_sonar, 0 )) {
    set_sensor_mode( sn_sonar, "US-DIST-CM" );
    if ( !get_sensor_value0( sn_sonar, &val )) {
        val = -1;
      }
    //printf( "\r(%f) \n", val);
    //fflush( stdout );

  }
  return val;
  
}

// COMPASS SENSOR ----------------------------------
float getCompassDegrees(){
  float val;
  uint8_t sn_compass;
  if (ev3_search_sensor(HT_NXT_COMPASS, &sn_compass, 0)){
    set_sensor_mode( &sn_compass, "COMPASS" );
    if (!get_sensor_value0( sn_compass, &val )){
      val = 0;
    }

    //printf("\r(%f) \n", val);
    //fflush( stdout );

  }

  return val;
}




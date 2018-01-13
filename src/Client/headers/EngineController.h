#ifndef ENGINECONTROLLER_H   /* Include guard */
#define ENGINECONTROLLER_H

#include <pthread.h>
#define WHEEL_DIAMETER 5.6
int stopp_engine_thread;
int current_speed;

int initEngines();
int isRunning();
int stopEngines();
int getMaxSpeed();
int getXPos();
int getYPos();
int getLeftEngineState();
int getRightEngineState();

double getWheelDiameter();

void turn2(int x);
void runForeverCorrected(int speed);
void runToRelPos( int speed,double distance, int heading_change);
void discoverEngines();
void waitForCommandToFinish();
void turnToDeg(int speed,int target);
void turnToDegCorrected(int speed,float target);
void turnNumberOfDegsCorrected(int turn_speed,int degree);
void backAwayTimed(int speed,int mseconds);
void runDistance(int speed,double distance);
void raiseArm();
void lowerArm();
#endif
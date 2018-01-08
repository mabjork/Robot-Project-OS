#ifndef ENGINECONTROLLER_H   /* Include guard */
#define ENGINECONTROLLER_H

int initEngines();
int isRunning();
int stopEngines();
int getMaxSpeed();
int getXPos();
int getYPos();
int getLeftEngineState();
int getRightEngineState();

void turn2(int x);
void runForever(int speed);
void runToRelPos( int speed,double distance, int h);
void discoverEngines();
void waitForCommandToFinish();
void turnToDeg(int speed,int target);
void turnNumberOfDegsCorrected(int turn_speed,int degree);

double getWheelDiameter();



#endif
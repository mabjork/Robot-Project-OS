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

void turnRight(int speed,int degrees);
void turnLeft(int speed,int degrees);
void runForever(int speed);
void runToRelPos( int speed,int x, int y  , int currX,int currY);
void discoverEngines();
void waitForCommandToFinish();
void turnToDeg(int speed,int target);
void turnNumberOfDegsCorrected(int turn_speed,int degree);

double getWheelDiameter();



#endif
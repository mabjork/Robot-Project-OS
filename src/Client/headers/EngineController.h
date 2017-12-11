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
void runToRelPos(int x, int y, int speed);
void discoverEngines();
void waitForCommandToFinish();
void turnToDeg(int target);
void turnNumberOfDegsCorrected(int degree);



#endif
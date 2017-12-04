#ifndef ENGINECONTROLLER_H   /* Include guard */
#define ENGINECONTROLLER_H

int initEngines();  /* An example function declaration */
void runEngines();
int isRunning();
int stopEngines();
void setEngineMode(int value);
int turnRight(int degrees);
int turnLeft(int degrees);
int runForever(int speed);


#endif
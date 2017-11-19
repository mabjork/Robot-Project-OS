#ifndef ENGINECONTROLLER_H   /* Include guard */
#define ENGINECONTROLLER_H

int initEngines();  /* An example function declaration */
void runEngines(int * engines,int time);
bool isRunning();
bool stopEngines();
void setEngineMode(int value);

#endif
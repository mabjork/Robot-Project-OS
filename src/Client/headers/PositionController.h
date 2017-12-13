#ifndef POSITIONCONTROLLER_H   /* Include guard */
#define POSITIONCONTROLLER_H

int getCurrentHeading();
void setCurrentHeading(int heading);
void initPositionController(int initialHeading);
void updateRobotPosition(double distance);
void updateMap(int x,int y,char value);

#endif
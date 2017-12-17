#ifndef POSITIONCONTROLLER_H   /* Include guard */
#define POSITIONCONTROLLER_H

int START_SQUARE_Y;
int START_SQUARE_X;
int current_square_y; 
int current_square_x;

int getCurrentHeading();
void setCurrentHeading(int heading);
void initPositionController(int initialHeading);
void updateRobotPosition(double distance);
void updateMap(int x,int y,char value);

#endif
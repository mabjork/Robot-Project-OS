#ifndef POSITIONCONTROLLER_H   /* Include guard */
#define POSITIONCONTROLLER_H

int START_SQUARE_Y;
int START_SQUARE_X;
int current_square_y; 
int current_square_x;
int * popFromQueue();

float getCurrentHeading();
float getInitialHeading();

void updateCurrentHeading(float heading);
void initPositionController(float initialHeading,int start_x,int start_y);
void updateRobotPosition(double distance);
void updateMap(int x,int y,char value);
void findPoints();
void getDistanceAndDirectionToPoint(int x,int y,double *diff_x,double *diff_y,float *target_angle);
void getSquareInFront(int distance,int * x,int *y);


#endif
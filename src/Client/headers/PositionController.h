#ifndef POSITIONCONTROLLER_H   /* Include guard */
#define POSITIONCONTROLLER_H

<<<<<<< HEAD
int START_SQUARE_Y;
int START_SQUARE_X;
int current_square_y; 
int current_square_x;

int getCurrentHeading();
void setCurrentHeading(int heading);
void initPositionController(int initialHeading);
=======
float getCurrentHeading();
void setCurrentHeading(float heading);
void initPositionController(float initialHeading);
>>>>>>> master
void updateRobotPosition(double distance);
void updateMap(int x,int y,char value);
void findPoints();
int * popFromQueue();
void getDistanceAndDirectionToPoint(int x,int y,double *diff_x,double *diff_y,float *target_angle);


#endif
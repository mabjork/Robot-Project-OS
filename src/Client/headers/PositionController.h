#ifndef POSITIONCONTROLLER_H   /* Include guard */
#define POSITIONCONTROLLER_H

float getCurrentHeading();
void setCurrentHeading(float heading);
void initPositionController(float initialHeading);
void updateRobotPosition(double distance);
void updateMap(int x,int y,char value);
void findPoints();
int * popFromQueue();
void getDistanceAndDirectionToPoint(int x,int y,double *diff_x,double *diff_y,float *target_angle);


#endif
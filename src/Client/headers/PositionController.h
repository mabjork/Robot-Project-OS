#ifndef POSITIONCONTROLLER_H   /* Include guard */
#define POSITIONCONTROLLER_H

double POS_X;
double POS_Y;
double REL_POS_X;
double REL_POS_Y;
int current_square_x;
int current_square_y;
float INITIAL_HEADING;
float HEADING;
int START_SQUARE_X;
int START_SQUARE_Y;

pthread_t position_tid;
pthread_mutex_t position_lock;

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
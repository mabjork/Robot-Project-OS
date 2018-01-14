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

struct Map{
    struct Array *rows;
    size_t height;
    size_t width;
    size_t longest_row;
};
struct Array{
    char * array;
    size_t used;
    size_t size;
};



typedef struct Map;
typedef struct Array;
struct Map neighbour_matrix;
struct Map map;
int stopp_position_thread;
float last_gyro_read;

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
void startPositionUpdateThread();


#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include "headers/PositionController.h"
#include "headers/EngineController.h"
#include "headers/SensorController.h"
#include <sys/time.h>

#define CalcHeading(deg) ((deg + HEADING) % 360)
#define CalcSquareX (int)((POS_X)/SQUARE_WIDTH)
#define CalcSquareY (int)((POS_Y)/SQUARE_HEIGHT)
#define SQUARE_WIDTH 5
#define SQUARE_HEIGHT 5
#define Sleep( msec ) usleep(( msec ) * 1000 )
enum{
    UP,
    LEFT,
    DOWN,
    RIGHT
};



struct PointQueue {
    int ** queue;
    size_t size;
    size_t used;
};


typedef struct PointQueue;


struct PointQueue point_queue;

struct timeval tval_before, tval_after, tval_result;


void updateCurrentHeading(float change){
    HEADING += change;
    printf("Changed heading : %f\n",HEADING);
    
    if(HEADING < 0){
        printf("Heading is negative\n");
        float p1 = HEADING / 360;
        printf("The float %f\n",p1);

        int neg_mod = (int)abs(floorf(p1));
        printf("This is the mod : %i\n",neg_mod);
        HEADING += neg_mod * 360;
    }else{
        HEADING = (int)HEADING % 360;
    }
    printf("Current heading is %f\n",HEADING);
}
float getCurrentHeading(){
    return HEADING;
}
void initPositionController(float initialHeading,int start_x,int start_y){
    START_SQUARE_X = start_x;
    START_SQUARE_Y = start_y;
    current_square_x = start_x;
    current_square_y = start_y;
    printf("This is the recived heading: %f\n",initialHeading);
    INITIAL_HEADING = initialHeading;
    HEADING = initialHeading;
    printf("Init heading value: %f\n",initialHeading);

    initMap(&map,current_square_x+2,current_square_y+2);
    POS_X = current_square_x * SQUARE_WIDTH + SQUARE_WIDTH/2;
    POS_Y = current_square_y * SQUARE_HEIGHT + SQUARE_HEIGHT/2;
    REL_POS_X = 0;
    REL_POS_Y = 0;
    updateMap(current_square_x,current_square_y,'S');
    printMatrix(&map);
    initQueue(100);
}

void updateRobotPosition(double distance){
    float rad = (HEADING-INITIAL_HEADING) * M_PI / 180;
    double dx;
    double dy;
    if(HEADING < 90){
        printf("Heading is less then 90\n");
        dy = fabs(distance * cos(rad));
        dx = fabs(distance * sin(rad));
    }else if(HEADING < 180) {
        printf("Heading is less then 180\n");
        dy = fabs(distance * cos(rad));
        dx = -fabs(distance * sin(rad));
    }else if (HEADING < 270){
        printf("Heading is less then 270\n");
        dy = -fabs(distance * cos(rad));
        dx = -fabs(distance * sin(rad));
    }else{
        printf("Heading is less then 360\n");
        dy = -fabs(distance * cos(rad));
        dx = fabs(distance * sin(rad));
    }
    REL_POS_X += dx;
    REL_POS_Y += dy;
    printf("Diff x : %lf , Diff y : %lf\n",dx,dy);
    POS_X += dx;
    POS_Y += dy;
    int square_x = CalcSquareX;//(POS_X);
    int square_y = CalcSquareY;//(POS_Y);
    
    if(square_x != current_square_x || square_y != current_square_y){
        updateMap(current_square_x,current_square_y, 'D');
    }
    
    if(square_x != current_square_x){
        
        printf("Current square x is changed to %i\n", square_x);
        current_square_x = square_x;
        if(current_square_x < 0){
            printf("Adding col to start of map!\n");
            for(int i = 0;i<abs(current_square_x);i++){
                addColLower(&map);
            }
            POS_X += abs(current_square_x)*SQUARE_WIDTH;
            current_square_x = 0;
            START_SQUARE_X += 1;
            printf("Start square x %i\n", START_SQUARE_X);
        }else if (current_square_x >= map.width){
            
            int diff = current_square_x - map.width + 1;
            for(int i = 0; i<diff;i++){
                printf("Adding col to end of map!\n");
                addCol(&map);
            }
        }
        
    }
    if(square_y != current_square_y){
        
        printf("Current square y is changed to %i\n", square_y);
        current_square_y = square_y;
        if(current_square_y < 0){
            printf("Adding row to start of map!\n");
            for(int i = 0;i<abs(current_square_y);i++){
                addRowLower2(&map);
            }
            POS_Y += abs(current_square_y)*SQUARE_HEIGHT;
            current_square_y = 0;
            START_SQUARE_Y += 1;
            printf("Start square y %i\n", START_SQUARE_Y);
        }else if (current_square_y >= map.height){
            
            int diff = current_square_y - map.height + 1;
            for(int i = 0; i<diff;i++){
                printf("Adding row to end of map!\n");
                addRow(&map);
            }
        }
    }
    printf("Current pos is %lf,%lf\n",POS_X,POS_Y);
    
    updateMap(current_square_x,current_square_y,'R');
    printMatrix(&map);
}

void updateMap(int x,int y,char value){
    printf("Map height: %i, Map width: %i\n",map.height,map.width);
    printf("x is: %i, y is: %i\n",x,y);
    if(x < 0 || y < 0 || map.width < x || map.height < y)return;
    //y = map.height - 1 - y;
    printf("Updating map\n");
    struct Array *rows = map.rows;
    struct Array row = rows[y];
    if(row.array[x] == 'W' || row.array[x] == 'B' || row.array[x] == 'S'){
        return 0;
    }
    row.array[x] = value;
    //printMatrix(&map);
    
}
void getSquareInFront(int distance,int * x,int *y){
    float rad = (HEADING-INITIAL_HEADING) * M_PI / 180;
    double dx;
    double dy;
    if(HEADING <= 90){
        dx = -fabs(distance * cos(rad));
        dy = -fabs(distance * sin(rad));
    }else if(HEADING <= 180) {
        dx = -fabs(distance * cos(rad));
        dy = fabs(distance * sin(rad));
    }else if (HEADING <= 270){
        dx = fabs(distance * cos(rad));
        dy = fabs(distance * sin(rad));
    }else{
        dx = fabs(distance * cos(rad));
        dy = -fabs(distance * sin(rad));
    }
    double pos_x = POS_X + dx;
    double pos_y = POS_Y + dy;
    int sq_x = (int)pos_x/SQUARE_WIDTH;
    int sq_y = (int)pos_y/SQUARE_HEIGHT;

    *x = sq_x;
    *y = sq_y;
}
void initQueue(size_t initialSize){
    point_queue.queue = (int *)malloc(initialSize * sizeof(int));
    point_queue.used = 0;
    point_queue.size = initialSize;
}
void appentToQueue(int* point){
    printf("Appending %i,%i\n",point[0],point[1]);
    if(point_queue.size == point_queue.used){
        point_queue.size *= 2;
        printf("Next size is %i\n",point_queue.size);
        point_queue.queue = (int *)realloc(point_queue.queue,point_queue.size* sizeof(int));
    }

    point_queue.queue[point_queue.used] = point;
    point_queue.used += 1;
    printf("Used %i\n", point_queue.used);
    printf("Point added %i,%i\n",point_queue.queue[point_queue.used-1][0],point_queue.queue[point_queue.used-1][1]);
    
}
int * popFromQueue(){
    int * point;
    if(point_queue.used>0){
        point_queue.used--;
        point = point_queue.queue[point_queue.used];
        printf("This shuld be point x %i\n",*(point));
        printf("This shuld be point y %i\n",*(point+1));
        return point;
        
        
    }
    
    return point;
}

void initArray(struct Array *a, size_t initialSize) {
  a->array = (char *)malloc(initialSize * sizeof(int));
  a->used = 0;
  a->size = initialSize;
  
  for(int i = 0; i<initialSize;i++){
        a->array[i] = 'U';
  }
}

void insertArray(struct Array *a, int element) {
  // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
  // Therefore a->used can go up to a->size 
  if (a->used == a->size) {
    a->size *= 2;
    a->array = (int *)realloc(a->array, a->size * sizeof(int));
  }
  a->array[a->used++] = element;
}
void freeArray(struct Array *a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}
void initMap(struct Map *m,size_t initialWidth,size_t initialHeight){
    m->rows = (struct Array *)malloc(((initialHeight)*(initialWidth)) * sizeof(struct Array));
    m->height = initialHeight;
    m->width = initialWidth;
    for(int i = 0; i<initialWidth; i++){
        struct Array row;
        initArray(&row,m->width);
        m->rows[i] = row;
    }
    
    
}
void insertIntoMap(struct Map *m,int x,int y,char value){
    
    struct Array *rows;
    if(y > m->height){
        m->height = y;
        m-> rows = (struct Array *)realloc(m->rows,m->height * sizeof(struct Array));
    }
    rows = m->rows;
    struct Array row = *(rows + y);
    if(x >row.size){
        row.size = x;
        row.array = (char *)realloc(row.array,row.size * sizeof(char));
    }
    row.array[x] = value;
}

void addRowLower(struct Map *m){
    m->height += 1;
    struct Array row;
    //printf("Matrix hight %i\n", m-> height);
    //printf("Add lower width %i\n", m->width);
    initArray(&row,m->width);
    //printf("Array width %i\n",row.size);
    struct Array *rows = m-> rows;
    m->rows = (struct Array *)realloc(m->rows,m->height * sizeof(row));
    struct Array row_before;
    struct Array row_target;
    for (int i = m->height-1;i>0;i--){
        //printf("This is the i %i\n",i);
        //row_target = rows[i];
        //printf("Target size %i\n",row_target.size);
        row_before = rows[i-1];
        printf("Before size %i\n",row_before.size);
        rows[i] = rows[i-1];
         
    }
    
    printf("THis is the last row size : %i\n", rows[m->height-1].size);
    rows[0] = row;
    m->rows = rows;
    printf("THis is the last row size : %i\n", rows[m->height-1].size);
   
}
void addRowLower2(struct Map * m){
    addRow(m);
    struct Array * rows = m->rows;
    struct Array row;
    struct Array next_row;
    for(int i = m->height-1; i>0;i--){
        row = rows[i-1];
        next_row = rows[i];
        for(int j = 0;j<row.size;j++){
            int val = row.array[j];
            next_row.array[j] = val;
        }
    }
    row = rows[0];
    for(int i = 0; i<row.size;i++){
        row.array[i] = 'U';
    }
}
void addRow(struct Map *m){
    m->height += 1;
    struct Array row;
    initArray(&row,m->width);
    m->rows = (struct Array*)realloc(m->rows,m->height * sizeof(row));
    //printf("HAHA row size %i\n", row.size);
    //printf("Should be put at %i\n",m->height-1);
    struct Array* rows;
    rows = m->rows;
    int last = m->height -1;
    //memmove(&rows[last],&row,m->width);
    rows[last] = row;

    //printf("TestTESTtest %i\n", rows[last].size);
}
void addCol(struct Map *m){
    m->width += 1;
    struct Array row;
    struct Array *rows = m-> rows;
    for (int i = 0;i<m->height;i++){
        row = *(rows + i);
        //printf("This row size %i\n",row.size);
        row.size = m->width;
        //printf("This row size %i\n",row.size);
        row.array = (char *)realloc(row.array,row.size * sizeof(char));
        row.array[row.size-1] = 'U';
        *(rows + i) = row;
        //printf("Last element %c\n",row.array[row.size-1]);
    }
    //printf("SIZE : %i\n",(*(rows)).size);
}
void addColLower(struct Map *m){
    m->width += 1;
    struct Array row;
    struct Array *rows = m-> rows;
    for (int i = 0;i<m->height;i++){
        
        row = *(rows + i);
        row.size = m->width;
        //printf("THis is the width %i\n", m->width);
        row.array = (char *)realloc(row.array,row.size * sizeof(char));

        *(rows + i) = row;
    }
    for (int i = 0;i<m->height;i++){
        //printf("Second loop infinite\n");
        row = *(rows + i);
        //printf("Lol  Row size : %i\n",row.size);
        
        for(int j = row.size; j>0;j--){
            //printf("J is %i\n",j);
            
            //printf("Third loop infinite");
            row.array[j] = row.array[j-1];
        }
        
        row.array[0] = 'U';

        *(rows + i) = row;
    }
    
    
}
void findUndiscoveredPoint(int*x,int*y){
    struct Array * rows = map.rows;
    struct Array row;
    for(int i = 0; i<map.height;i++){
        row = rows[i];
        for(int j = 0 ; j< row.size;j++){
            if (row.array[j] == 'U'){
                x = j;
                y = i;
                return;
            }
        }
    }
    return;
}
void updateClosedPositions(){
    struct Array * rows = neighbour_matrix.rows;
    struct Array row;
    for(int i = 0; i<map.height;i++){
        row = rows[i];
        for(int j = 0 ; j< row.size;j++){
            
        }
    }
}
void findPoints(){
    struct Array * rows = map.rows;
    struct Array row;
    for(int i = 0; i<map.height;i++){
        row = rows[i];
        for(int j = 0 ; j< row.size;j++){
            if (row.array[j] == 'U'){
                int *point = malloc(2*sizeof(int));
                point[0] =j;
                point[1] =i;
                printf("Found point %i,%i\n",point[0],point[1]);
                appentToQueue(point);
                
            }
        }
    }
}


void getDistanceAndDirectionToPoint(int x ,int y,double *diff_x,double *diff_y,float *target_angle){
    int target_x = x;
    printf("Target x %i\n",x);
    int target_y = y;
    printf("Target y %i\n",y);
    *diff_x = (target_x*SQUARE_WIDTH+SQUARE_WIDTH/2) - POS_X;
    *diff_y = (target_y*SQUARE_HEIGHT+SQUARE_HEIGHT/2) - POS_Y;
    printf("getDistance diff x %lf\n",*diff_x);
    printf("getDistance diff y %lf\n",*diff_y);
    printf("Heading is %f\n",HEADING);
    printf("Arctan %f\n",atan(1)*180/M_PI);
    *target_angle = ceil((atan(*diff_y / *diff_x) * 180 / M_PI) - HEADING);
    printf("Target angle : %f\n",*target_angle);
}

/*
int main(int argc, char const *argv[]) {
    int target =  (200 - 340) % 360;
    target += 360;
    printf("Target is %i\n", target);
    //printf("Rads %f\n",rad);
    //printf("%f\n",cos(rad));
    int *point;
    int x;
    int y;
    
    initPositionController(0,4,4);
    printMatrix(&map);
    findPoints();
    printQueue(&point_queue);
    sortPositionsBasedOnDistance();
    printf("Used positions %i\n",point_queue.used);
    point = popFromQueue();
    x = *(point);
    y = *(point + 1);
    free(point);
    printf("Used positions %i\n",point_queue.used);
    printf("This is closest x: %i , This is closest y: %i \n",x,y);
    point = popFromQueue();
    x = *(point);
    y = *(point + 1);
    free(point);
    printf("Used positions %i\n",point_queue.used);
    printf("This is closest x: %i , This is closest y: %i \n",x,y);
    point = popFromQueue();
    x = *(point);
    y = *(point + 1);
    free(point);
    printf("Used positions %i\n",point_queue.used);
    printf("This is closest x: %i , This is closest y: %i \n",x,y);
    point = popFromQueue();
    x = *(point);
    y = *(point + 1);
    free(point);
    printf("Used positions %i\n",point_queue.used);
    printf("This is closest x: %i , This is closest y: %i \n",x,y);
    point = popFromQueue();
    x = *(point);
    y = *(point + 1);
    free(point);
    printf("Used positions %i\n",point_queue.used);
    printf("This is closest x: %i , This is closest y: %i \n",x,y);
    point = popFromQueue();
    x = *(point);
    y = *(point + 1);
    free(point);
    printf("Used positions %i\n",point_queue.used);
    printf("This is closest x: %i , This is closest y: %i \n",x,y);
    point = popFromQueue();
    x = *(point);
    y = *(point + 1);
    free(point);
    printf("Used positions %i\n",point_queue.used);
    printf("This is closest x: %i , This is closest y: %i \n",x,y);
    point = popFromQueue();
    x = *(point);
    y = *(point + 1);
    free(point);
    printf("Used positions %i\n",point_queue.used);
    printf("This is closest x: %i , This is closest y: %i \n",x,y);
    
    setCurrentHeading(45);
    updateRobotPosition(5);
    setCurrentHeading(90);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    setCurrentHeading(270);
    updateRobotPosition(5);
    updateRobotPosition(5);
    setCurrentHeading(0);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    setCurrentHeading(270);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    updateRobotPosition(5);
    
    
}
*/
/*
int main(int argc, char const *argv[]) {
    initPositionController(90,5,3);
    updateRobotPosition(5);
    updateCurrentHeading(90);
    updateRobotPosition(5);
}
*/




void printMatrix(struct Map *m){
    struct Array row;
    struct Array *rows = m-> rows;
    printf("Map height %i\n",m->height);
    
    for(int i = m->height-1;i > -1;i--){
        row = rows[i];
        
        for(int j = 0;j < m->width; j++){

            printf("%c",row.array[j]);
        }
        printf("\n");
    }
    printf("\n");
    
}
void printQueue(struct PointQueue *queue){
    int * point;
    int x;
    int y;
    for(int i = 0; i<queue->used;i++){
        point = queue->queue[i];
        x = *point;
        y = *(point + 1);
        printf("x = %i , y = %i\n", x,y);
    }
}

void sortPositionsBasedOnDistance(){
    struct PointQueue queue;
    int * point;
    int * next_point;
    int x1,x2;
    int y1,y2;
    double d1,d2;

    for(int i = 0; i< point_queue.used - 1; i++){
        for(int j = 0 ; j < point_queue.used - i - 1; j++){
            point = point_queue.queue[j];
            next_point = point_queue.queue[j + 1];
            x1 = *point;
            y1 = *(point + 1);
            d1 = sqrt( pow(x1-POS_X,2) + pow(y1-POS_Y,2));

            x2 = *next_point;
            y2 = *(next_point + 1);
            d2 = sqrt( pow(x2-POS_X,2) + pow(y2-POS_Y,2));

            if(d1 < d2){
                point_queue.queue[j] = next_point;
                point_queue.queue[j+1] = point;
            }

        }
            
    }
    
}
int calcSquareX(double x){
    if(x < 0){
        return (int)x/SQUARE_WIDTH;
    }
    return ceil(x/SQUARE_WIDTH);
}
int calcSquareY(double y){
    if(y < 0){
        return (int)y/SQUARE_HEIGHT;
    }
    return ceil(y/SQUARE_HEIGHT);
}
void *updatePositionInThread(void *args){
    printf("Starting thread\n");
    gettimeofday(&tval_before, NULL);
    last_gyro_read = getGyroDegrees();
    while(!stopp_position_thread){
        printf("Updating position\n");
        float gyro_read = getGyroDegrees();
        float heading_diff = gyro_read - last_gyro_read;
        last_gyro_read = gyro_read;
        printf("Heading diff %f",heading_diff);
        measureAndUpdateTraveledDistance(current_speed,&heading_diff);
        if(fabs(gyro_read) >= 1){
            //calibrateGyro();
        }
        gettimeofday(&tval_before, NULL);
        Sleep(250);
    }
    pthread_join(position_tid, NULL);
}

void startPositionUpdateThread(){
    printf("Starting position update thread\n");
    int *arg = malloc(sizeof(int));
    pthread_mutex_init(&position_lock, NULL);
    stopp_position_thread = 0;
    pthread_mutex_unlock(&position_lock);
    pthread_create(&position_tid, NULL, updatePositionInThread, arg);
    printf("Position thread created\n");
}



double calculateDistance(int speed,struct timeval *time){
    long time_in_usec = (time->tv_sec*1000000.0 + time->tv_usec);
    //printf("Time in micro seconds: %d\n",time_in_usec);
    //Distance in counts each usec;
    double counts = speed*time_in_usec/1000000.000;
    double wheel_radius = WHEEL_DIAMETER/2;
    //Distance in cm.
    double distance = counts * wheel_radius * M_PI / 360 ;
    return distance;
}

void measureAndUpdateTraveledDistance(int speed,float *heading){
    pthread_mutex_init(&position_lock, NULL);
    updateCurrentHeading(-heading[0]);
    if(speed = 0){
        pthread_mutex_unlock(&position_lock);
        return;
    }
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    double traveled_distance = calculateDistance(speed,&tval_result);
    printf("Traveled distance %lf\n", traveled_distance);
    updateRobotPosition(traveled_distance);
    pthread_mutex_unlock(&position_lock);
    
}

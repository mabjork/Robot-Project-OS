#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>

#define CalcHeading(deg) ((deg + HEADING) % 360)
#define CalcSquareX (int)((POS_X)/SQUARE_WIDTH)
#define CalcSquareY (int)((POS_Y)/SQUARE_HEIGHT)
#define SQUARE_WIDTH 5
#define SQUARE_HEIGHT 5

enum{
    UP,
    LEFT,
    DOWN,
    RIGHT
};

double POS_X;
double POS_Y;
int current_square_x;
int current_square_y;
float INITIAL_HEADING;
float HEADING;
int START_SQUARE_X;
int START_SQUARE_Y;

struct PointQueue {
    int ** queue;
    size_t size;
    size_t used;
};

struct Array{
    char * array;
    size_t used;
    size_t size;
};
struct Map{
    struct Array *rows;
    size_t height;
    size_t width;
    size_t longest_row;
};


typedef struct Map;
typedef struct Array;
typedef struct PointQueue;

struct Map neighbour_matrix;
struct Map map;
struct PointQueue point_queue;



float getInitialHeading(){
    return INITIAL_HEADING;
}
void setCurrentHeading(float heading){
    printf("Current heading is %f\n", heading);
    HEADING = heading;
}
float getCurrentHeading(){
    return HEADING;
}
void initPositionController(int initialHeading){
    START_SQUARE_X = 0;
    START_SQUARE_Y = 0;
    INITIAL_HEADING = initialHeading;
    initMap(&map,3);
    updateMap(1,1,'S');
    printMatrix(&map);
    initQueue(&point_queue,5);
}

void updateRobotPosition(double distance){
    double dx = distance * sin(HEADING-INITIAL_HEADING);
    double dy = distance * cos(HEADING-INITIAL_HEADING);
    printf("Diff x : %lf , Diff y : %lf\n",dx,dy);
    POS_X += dx;
    POS_Y += dy;
    int square_x = CalcSquareX;
    int square_y = CalcSquareY;
    if(square_x != current_square_x || square_y != current_square_y){
        updateMap(current_square_x-START_SQUARE_X,current_square_y-START_SQUARE_Y, 'D');
    }
    if(square_x != current_square_x){
        
        printf("Current square x is changed to %i\n", square_x);
        current_square_x = square_x;
        if(current_square_x < 0){
            int diff = current_square_x + START_SQUARE_X;
            for(int i = 0; i<diff;i++){
                printf("Adding col lower to map!");
                addColLower(&map);
            }
            START_SQUARE_X += diff;
            printf("Start square x %i\n", START_SQUARE_X);
        }else if (current_square_x >= map.width){
            
            int diff = current_square_x - START_SQUARE_X;
            for(int i = 0; i<diff;i++){
                printf("Adding col to end of map!");
                addCol(&map);
            }
        }
        
    }
    if(square_y != current_square_y){
        
        printf("Current square y is changed to %i\n", square_y);
        current_square_y = square_y;
        if(current_square_y < 0){
            int diff = current_square_y + START_SQUARE_Y;
            for(int i = 0; i<diff;i++){
                printf("Adding row to start of map!");
                addRowLower2(&map);
            }
            START_SQUARE_Y += diff;
            printf("Start square y %i\n", START_SQUARE_Y);
        }else if (current_square_y >= map.height){
            
            int diff = current_square_y - START_SQUARE_Y;
            for(int i = 0; i<diff;i++){
                printf("Adding row to end of map!");
                addRow(&map);
            }
        }
    }
    printMatrix(&map);
    updateMap(current_square_x-START_SQUARE_X,current_square_y-START_SQUARE_Y,'R');

}

void updateMap(int x,int y,char value){

    if(x < 0 || y < 0 || map.width < x || map.height < y)return;
    printf("Updating map\n");
    struct Array *rows = map.rows;
    struct Array row = rows[y];
    if(row.array[x] == 'W' || row.array[x] == 'B'){
        return 0;
    }
    row.array[x] = value;
    printMatrix(&map);
    
}

void initQueue(struct PointQueue * queue,size_t initialSize){
    queue->queue = (int *)malloc(initialSize * sizeof(int));
    queue->used = 0;
    queue->size = initialSize;
}
void appentToQueue(struct PointQueue * queue, int* point){
    if(queue->size == queue->used){
        queue->size *= 2;
        queue->queue = (int *)realloc(queue->queue,queue->size * sizeof(int));
    }
    queue->queue[queue->used++] = point;
}
int *  popFromQueue(){
    int * point;
    if(point_queue.used>0){
        point_queue.used--;
        return point_queue.queue[point_queue.used];
        
        
    }
    printf("This shuld be point y %i\n",*(point+1));
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
void initMap(struct Map *m,size_t initialHeight){
    m->rows = (struct Array *)malloc(initialHeight * sizeof(struct Array));
    m->height = initialHeight;
    m->width = initialHeight;
    for(int i = 0; i<initialHeight; i++){
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
void moveElementsUp(struct Map *m){

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
    for(int i = 0; i< m->height-1;i++){
        row = rows[i];
        next_row = rows[i+1];
        for(int j = 0;j<row.size;j++){
            next_row.array[j] = row.array[j];
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
                printf("Found point %i,%i\n",j,i);
                int point[2] = {j,i};
                appentToQueue(&point_queue,&point);
            }
        }
    }
}


void getDistanceAndDirectionToPoint(int x ,int y,double *diff_x,double *diff_y,float *target_angle){
    int target_x = x;
    printf("Target x %i\n",x);
    int target_y = y;
    printf("Target y %i\n",y);
    *diff_x = POS_X - target_x;
    *diff_y = POS_Y - target_y;
    printf("getDistance diff x %lf\n",*diff_x);
    printf("getDistance diff y %lf\n",*diff_y);
    printf("Heading is %f\n",HEADING);
    *target_angle = HEADING - atan(*diff_y / (*diff_x));
    printf("Target angle : %f\n",*target_angle);
}
/*
int main(int argc, char const *argv[]) {
    initPositionController(0);
    setCurrentHeading(45);
    updateRobotPosition(10);
    updateRobotPosition(10);
    updateRobotPosition(10);
    updateRobotPosition(10);
}
*/


void printMatrix(struct Map *m){
    struct Array row;
    struct Array *rows = m-> rows;
    printf("Map height %i\n",m->height);
    
    for(int i = 0;i<m->height;i++){
        row = rows[i];
        
        for(int j = 0;j < m->width; j++){

            printf("%c",row.array[j]);
        }
        printf("\n");
    }
    printf("\n");
    
}

void sortPositionsBasedOnDistance(){

}



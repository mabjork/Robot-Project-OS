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


int POS_X;
int POS_Y;

int current_square_x;
int current_square_y;

int INITIAL_HEADING;
int HEADING;


struct Array{
    char * array;
    size_t used;
    size_t size;
};
struct Map{
    struct Array *rows;
    size_t height;
    size_t width;
};


typedef struct Map;
typedef struct Array;

struct Map map;

void setCurrentHeading(int heading){
    HEADING = heading;
}
void getCurrentHeading(){
    return HEADING;
}
void initPositionController(int initialHeading){

    INITIAL_HEADING = initialHeading;
    initMap(&map,1);
}

void updateRobotPosition(int distance){
    int dx = distance * sin(HEADING-INITIAL_HEADING);
    int dy = distance * cos(HEADING-INITIAL_HEADING);
    POS_X += dx;
    POS_Y += dy;
    current_square_x = CalcSquareX;
    current_square_y = CalcSquareY;
    
}
void updateMap(int x,int y,char value){
    insertIntoMap(&map,x,y,value);
}

void initArray(struct Array *a, size_t initialSize) {
  a->array = (char *)malloc(initialSize * sizeof(int));
  a->used = 0;
  a->size = initialSize;
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
    m->height = 1;
    m->width = sizeof(struct Array);
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



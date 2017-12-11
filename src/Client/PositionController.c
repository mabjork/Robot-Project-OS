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
    size_t longest_row;
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
/*
void updateRobotPosition(int distance){
    int dx = distance * sin(HEADING-INITIAL_HEADING);
    int dy = distance * cos(HEADING-INITIAL_HEADING);
    POS_X += dx;
    POS_Y += dy;
    current_square_x = CalcSquareX;
    current_square_y = CalcSquareY;
    
}
*/
void updateMap(struct Map *map,int x,int y,char value){
    struct Array *rows = map->rows;
    struct Array row = *(rows + y);
    row.array[x] = value;
}

void initArray(struct Array *a, size_t initialSize) {
  a->array = (char *)malloc(initialSize * sizeof(int));
  a->used = 0;
  a->size = initialSize;
  
  for(int i = 0; i<initialSize;i++){
        a->array[i] = 'O';
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
    m->height = 1;
    m->width = 1;
    struct Array row;
    initArray(&row,m->width);
    m->rows[0] = row;
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
        row.array[i] = 'O';
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
        row.array[row.size-1] = 'O';
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
        
        row.array[0] = 'O';

        *(rows + i) = row;
    }
    
    
}

int main(int argc, char const *argv[]) {
    struct Map map;
    initMap(&map , 1);
    printMatrix(&map);
    addCol(&map);
    printMatrix(&map);
    addRow(&map);
    struct Array * rows = map.rows;
    struct Array row = rows[map.height-1];
    printf("Last row size : %i\n",row.size);
    addRow(&map);
    rows = map.rows;
    row = rows[map.height-1];
    printf("Last row size : %i\n",row.size);
    addRow(&map);
    rows = map.rows;
    row = rows[map.height-1];
    printf("Last row size : %i\n",row.size);
    addRow(&map);
    rows = map.rows;
    row = rows[map.height-1];
    printf("Last row size : %i\n",row.size);
    printMatrix(&map);
    addColLower(&map);
    addColLower(&map);
    printMatrix(&map);
    addRowLower2(&map);
    rows = map.rows;
    row = rows[map.height-1];
    printf("Last row size : %i\n",row.size);
    printMatrix(&map);
    updateMap(&map,0,0,'X');
    printMatrix(&map);
    updateMap(&map,1,1,'H');
    printMatrix(&map);
    addRow(&map);
    addColLower(&map);
    addColLower(&map);
    printMatrix(&map);
}
void printMatrix(struct Map *m){
    struct Array row;
    struct Array *rows = m-> rows;
    printf("Map height %i\n",m->height);
    
    for(int i = 0;i<m->height;i++){
        row = rows[i];
        printf("Row size = %i\n",row.size);
        for(int j = 0;j < m->width; j++){

            printf("%c",row.array[j]);
        }
        printf("\n");
    }
    printf("\n");
    
}



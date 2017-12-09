//#include <stdio.h>

int main(){
   /* 2D array declaration*/
   int disp[3][3];
   /*Counter variables for the loop*/
   int i, j;
   for(i=0; i<3; i++) {
      for(j=0;j<3;j++) {
         disp[i][j] = '-';
      }
   }

   //Swap an element
   disp[1][1] == 'R';

   //Displaying array elements
   printf("Map:\n");
   for(i=0; i<3; i++) {
      for(j=0;j<3;j++) {
         printf("%c ", disp[i][j]);
         if(j==2){
            printf("\n");
         }
      }
   }
   return 0;
}


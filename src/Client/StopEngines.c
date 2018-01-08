
#include "./headers/EngineController.h"


int main(int argc, char const *argv[]) {
    printf( "LOL this should work\n" );
    if ( ev3_init() == -1 ) return ( 1 );
    printf("Ev3 initiated\n");
    initEngines();
    printf("Engines initiated\n");
    //discoverEngines();
    stopEngines();
    return 0;

}
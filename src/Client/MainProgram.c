

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "./headers/EngineController.h"

#define SERV_ADDR   "dc:53:60:ad:61:90"     /* Whatever the address of the server is */
#define TEAM_ID     1                       /* Your team ID */

#define MSG_ACK     0
#define MSG_START    1
#define MSG_STOP   2
#define MSG_KICK    3
#define MSG_POSITION 4
#define MSG_MAPDATA 	5
#define MSG_MAPDONE 6

int main(int argc, char const *argv[]) {
    int engines[2] = {1,2};
    if ( ev3_init() == -1 ) exit(0);
    if (initEngines()== true){
        runEngines(&engines,1000);
    }


}

void startDiscovery(){
    setEngineMode(0);

}

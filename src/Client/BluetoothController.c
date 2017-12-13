//
// Created by marius on 11/19/17.
//

#include "headers/BluetoothController.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#define SERV_ADDR   "dc:53:60:ad:61:90"     /* Whatever the address of the server is */
#define TEAM_ID     	14                  /* Your team ID */

#define MSG_ACK     	0
#define MSG_START    	1
#define MSG_STOP   		2
#define MSG_KICK    	3
#define MSG_POSITION 	4
#define MSG_MAPDATA 	5
#define MSG_MAPDONE 	6
#define MSG_OBSTACLE	7
#define MSG_CUSTOM		8

int read_from_server (int sock, char *buffer, size_t maxSize) {
    int bytes_read = read (sock, buffer, maxSize);

    if (bytes_read <= 0) {
        fprintf (stderr, "Server unexpectedly closed connection...\n");
        close (s);
        exit (EXIT_FAILURE);
    }

    printf ("[DEBUG] received %d bytes\n", bytes_read);

    return bytes_read;
}
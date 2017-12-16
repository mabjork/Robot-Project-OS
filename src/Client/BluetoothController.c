#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include <math.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "headers/BluetoothController.h"

#define SERV_ADDR  "9c:ad:97:b1:a7:d2" /*Halvor PC BT*/ /* 38:ca:da:e9:90:6c Halvor Iphone BT */ 
/* ROBOT BT "00:17:e9:f5:c9:dd" */ /*OS SERVER "dc:53:60:ad:61:90"*/

#define TEAM_ID 14

#define MSG_ACK 0
#define MSG_START 1
#define MSG_STOP 2
#define MSG_KICK 3
#define MSG_POSITION 4
#define MSG_MAPDATA 5
#define MSG_MAPDONE 6
#define MSG_OBSTACLE 7
#define Sleep( msec ) usleep(( msec ) * 1000 )

int s;
uint16_t msgId = 0;
void btcommunication() {
  struct sockaddr_rc addr = { 0 };
  int status;

  /* allocate a socket */
  s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

  /* set the connection parameters (who to connect to) */
  addr.rc_family = AF_BLUETOOTH;
  addr.rc_channel = (uint8_t) 1;
  str2ba (SERV_ADDR, &addr.rc_bdaddr);

  /* connect to server */
  status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

  /* if connected */
  if( status == 0 ) {
    char string[58];

    /* Wait for START message */
    read_from_server (s, string, 9);
    if (string[4] == MSG_START) {
      printf ("Received start message!\n");
    }

    //call some function tocommunicate here, like robot(); in robotclient.c
    positionprint();
    init();
    startDiscovery(); 
    stopmessage(); // Has to be added to startDiscovery, it's never called now
    
    //close (s);

    sleep (5);

  } else {
    fprintf (stderr, "Failed to connect to server...\n");
    sleep (2);
    exit (EXIT_FAILURE);
  }

  //close(s);
  return 0;
}

void positionprint () {
  char string[58];
  char type;
  int x1, x2, y1, y2;
  printf ("I'm navigating...\n");

  srand(time(NULL));
  /* Send 1 position message */
  int i, j;
  for (i=0; i<1; i++){
    *((uint16_t *) string) = msgId++;
    string[2] = TEAM_ID;
    string[3] = 0xFF;
    string[4] = MSG_POSITION;
    string[5] = i;          /* x */
    string[6] = 0x00;
    string[7] = i;              /* y */
    string[8]= 0x00;
    write(s, string, 9);
    Sleep( 1000 );
  }
}

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

void stopmessage() {
  char string[58];
  char type;
  int x1, x2, y1, y2;
  printf("I'm waiting for the stop message");
  while(1){
    //Wait for stop message
    read_from_server (s, string, 58);
    type = string[4];
    if (type ==MSG_STOP){
      return;
    }
  }
}

//void write_to_server (struct team *t, const char *buf, size_t size) {
//    write (t->sock, buf, size);
//}
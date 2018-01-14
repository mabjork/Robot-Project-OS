#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include <math.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include "headers/BluetoothController.h"
#include "headers/PositionController.h"

#define SERV_ADDR  "9c:ad:97:b1:a7:d2" /*Halvor PC BT*/ /* 38:ca:da:e9:90:6c Halvor Iphone BT */ 
/* ROBOT BT "00:17:e9:f5:c9:dd" */ /*OS SERVER "dc:53:60:ad:61:90"*/ /*BT SERVER "00:1a:7d:da:71:06*/
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
bool is_moving = false;
uint16_t msgId = 0;

/* START BLUETOOTH CONNECTION */
int bt_connect() {
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
  if( status != 0 ) {
    printf("Error: %d\n", errno);
    }
  return status;
}

/* SEND ACK MESSAGE */
ssize_t bt_ack(uint16_t ackId, uint8_t dest, uint8_t statusCode){
    char string[58];
    printf("Sending ACK\n");
    *((uint16_t *) string) = msgId++;
    string[2] = TEAM_ID;
    string[3] = dest;
    string[4] = MSG_ACK;
    *((uint16_t *) string+5) = ackId;
    string[7] = statusCode;
    return write(s, string, 8);
}

/* WAIT FOR START MESSAGE */
int bt_wait_startmsg(char * msg){
    if (msg[4] == MSG_START) {
        printf ("Received start message!\n");
        is_moving = true;
        /* Start Driving */
        startDiscovery();
        return 0;
    } else {
        return -1;
    }
}

/* WAIT FOR STOP MESSAGE */
int bt_wait_stopmsg(char * msg) {
    if (msg[4] == MSG_STOP) {
        printf ("Received Stop message!\n");
        bt_close();
        stopEngines();
        //position_stop(); STOP SENDING POSITION OR SOMETHING
        return 0;
    } else {
        return -1;
    }
}

/* WAIT FOR KICK MESSAGE */
int bt_wait_kick(char * msg){

    if (msg[4] == MSG_KICK) {
        printf ("Received Kick message!\n");
        bt_close();
        stopEngines();
        //position_stop(); STOP SENDING POSITION OR SOMETHING
        return 0;
    } else {
        return -1;
    }
}

/* Send a POSITION message to the server */
ssize_t bt_position(){
    char string[58];
    float x, y;
    int heading;
    int16_t x1, y1;
    //get_position_and_heading(&x, &y, &heading); 
    x1 = (int16_t)(current_square_x); //- START_SQUARE_X) ;
    y1 = (int16_t)(current_square_y); //- START_SQUARE_Y);
    printf("Sending X: %d, Y:%d\n", x1, y1);
    *((uint16_t *) string) = msgId++;
    string[2] = TEAM_ID;
    string[3] = 0xFF;
    string[4] = MSG_POSITION;
    string[5] = (uint8_t)(x1);
    string[6] = (uint8_t)(x1>>8);
    string[7] = (uint8_t)(y1);
    string[8]= (uint8_t)(y1>>8);
    return write(s, string, 9);
    //Sleep( 1000 );
}

/* MISSING */
/* MAPDATA */

/* MAPDONE */
void bt_mapdone(){
    char string[58];
	printf("Done sending map");
 	*((uint16_t *) string) = msgId++;
 	string[2] = TEAM_ID;
 	string[3] = 0xFF;
 	string[4] = MSG_MAPDONE;
 	write(s, string, 5);
}

/* OBSTACLE */ //Possible to drop val, and just use 0, if we only drop obstacles
ssize_t bt_obstacle(uint8_t val){
    char string[58];
    float x, y;
    //int heading;
    int16_t x1, y1;
    //get_position_and_heading(&x, &y, &heading); 
    x1 = (int16_t)(current_square_x - START_SQUARE_X) ;
    y1 = (int16_t)(current_square_y - START_SQUARE_Y);
    printf("BT sending obstacle position\n");
    *((uint16_t *) string) = msgId++;
    string[2] = TEAM_ID;
    string[3] = 0xFF;
    string[4] = MSG_OBSTACLE;
    string[5] = val;
    string[6] = (uint8_t)(x1);
    string[7] = (uint8_t)(x1>>8);
    string[8] = (uint8_t)(y1);
    string[9] = (uint8_t)(y1>>8);
    return write(s, string, 10);
}

/* READ FROM SERVER */
int read_from_server (int sock, char *buffer, size_t maxSize) {
  int bytes_read = read (sock, buffer, maxSize);
  if (bytes_read <= 0) {
    fprintf (stderr, "Server unexpectedly closed connection...\n");
    bt_close (s);
    exit (EXIT_FAILURE);
  }
  printf ("[DEBUG] received %d bytes\n", bytes_read);
  return bytes_read;
}

/* CHECK MESSAGE */
int bt_check(){
    ssize_t nbyte;
    char msg[58]; 

    /* Read from server */
    //OBS: Check if this part works, might have to change read_from_server to read
    nbyte = read_from_server (s, msg, 9);
    printf("BT RECIEVED message type: %d of size %d\n", (int)msg[4], nbyte);
    
    if(nbyte==-1){
        fprintf (stderr, "Server unexpectedly closed connection...\n");
        bt_close (s);
        exit (EXIT_FAILURE);

    }
    else if(nbyte==0){
        printf ("BT: Nothing to read\n");
        return 0;
    }    

    else{
        switch(msg[4]) {
            //case MSG_ACK :
            //    bt_ack(msg);
            //    break;
            case MSG_START :
                bt_wait_startmsg(msg);
                break;
            case MSG_STOP :
                bt_wait_stopmsg(msg);
                break;
            case MSG_KICK :
                bt_wait_kick(msg);
                break;
            //case MSG_BALL :
            //    bt_recv_ball(msg);
            //    break;
            default :
                printf("BT: Invalid message\n");
                break;
        }
    }
    return nbyte;
}

//void write_to_server (struct team *t, const char *buf, size_t size) {
//    write (t->sock, buf, size);
//}

/*
void get_position_and_heading(float * x, float *y, int * heading){
    pthread_mutex_lock(&position_mutex);
    *x = POS_X;
    *y = POS_Y;
    *heading = HEADING;
    pthread_mutex_unlock(&position_mutex);
}
*/

int bt_close(){
    bt_stop();
    return close(s);
}

/* THREAD */
bool bt_terminate = false;

void *bt_send(){
    while(!bt_terminate){
        if(is_moving)
            bt_position();
        sleep(2);
    }
    return NULL;
}

pthread_t t;

void bt_transmit(){
    bt_terminate = false;
    pthread_create(&t, NULL, bt_send, NULL);
}

void bt_stop(){
    bt_terminate = true;
    pthread_join(t, NULL);
}

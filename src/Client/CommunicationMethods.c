#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include <math.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#define SERV_ADDR "00:17:e9:f5:c9:dd" //"dc:53:60:ad:61:90"  /* Whatever the address of the server is */
#define TEAM_ID 14                       /* Your team ID */

#define MSG_ACK 0
#define MSG_START 1
#define MSG_STOP 2
#define MSG_KICK 3
#define MSG_POSITION 4
#define MSG_MAPDATA 5
#define MSG_MAPDONE 6
#define MSG_OBSTACLE 7
#define Sleep( msec ) usleep(( msec ) * 1000 )

/* Send an ACK message */
ssize_t bt_send_ack(uint16_t ackId, uint8_t dest, uint8_t statusCode){
    char string[58];
    printf("BT Sending ACK\n");
    // Remember to increment msgId
    *((uint16_t *) string) = msgId++;
    string[2] = TEAM_ID;
    string[3] = dest;
    string[4] = MSG_ACK;
    *((uint16_t *) string+5) = ackId;
    string[7] = statusCode;

    /* Return number of bytes written */
    return write(s, string, 8);
}


//POSITION messages must be sent by robots every 2 seconds. This message is used to advertise the expected position of the robot.
//The message is 9-bytes long:
void position(int x, int y){
	*((uint16_t *) string) = msgId++;
    string[2] = TEAM_ID;
    string[3] = 0xFF;
    string[4] = MSG_POSITION;
    string[5] = x;          /* x */
    string[6] = 0x00;
    string[7] = y;              /* y */
    string[8]= 0x00;
    write(s, string, 9);
    Sleep( 1000 );
}

/* Send a POSITION message to the server */
ssize_t bt_send_position(){
    char string[58];
    float x, y;
    int heading;
    int16_t x1, y1;

    get_position_and_heading(&x, &y, &heading); 
    x1 = (int16_t)x;
    y1 = (int16_t)y;
    printf("Sending X: %d, Y:%d\n", x1, y1);

    // Remember to increment msgId
    *((uint16_t *) string) = msgId++;
    string[2] = TEAM_ID;
    string[3] = 0xFF;
    string[4] = MSG_POSITION;
    // Little endian representation
    string[5] = (uint8_t)(x1);
    string[6] = (uint8_t)(x1>>8);
    string[7] = (uint8_t)(y1);
    string[8]= (uint8_t)(y1>>8);

    /* Return number of bytes written */
    return write(s, string, 9);
}

//After the entire map has been generated, the robot sends the server each 5x5 cm grid one pixel at a time.
void mapdata(int x, int y, int R, int G, int B){
	*((uint16_t *) string) = msgId++;
    string[2] = TEAM_ID;
    string[3] = 0xFF;
    string[4] = MSG_MAPDATA;
    string[5] = x1+10;          /* x */
    string[6] = 0x00;
    string[7] = j;              /* y */
    string[8]= 0x00;
    string[9]= 0;
    string[10]=254;
    string[11]= 0;
    write(s, string, 12);
    Sleep( 100 );



/*
	*((uint16_t *) string) = msgId++;
	//int ID; 
	int src = TEAM_ID;
	int dst = 0XFF;
	int type = MSG_MAPDATA;
	int x; //the x coordinate of the pixel. This field is a signed 16-bits little-endian integer.
	int y; //the y coordinate of the pixel. This field is a signed 16-bits little-endian integer.
	int R; //'R', 'G', and 'B' are the red, green, blue values of the pixel respectively, valued from 0 to 255.
	int G; 
	int B; 
*/
}

//These messages signal to the server that their map is finished.
void mapdone(){
	printf("Done sending map");
 	*((uint16_t *) string) = msgId++;
 	string[2] = TEAM_ID;
 	string[3] = 0xFF;
 	string[4] = MSG_MAPDONE;
 	write(s, string, 5);

	/**((uint16_t *) string) = msgId++;
	//int ID; 
	int src = 14;
	int dst = 0XFF;
	int type = MSG_MAPDONE;*/
}

//OBSTACLE messages must be sent when a robot picks up or drop an obstacle.
void obstacle(int act, int x, int y){ //act: 0 = dropped, 1 = picked up
	*((uint16_t *) string) = msgId++;
    string[2] = TEAM_ID;
    string[3] = 0xFF;
    string[4] = MSG_OBSTACLE;
    string[5] = act          
    string[6] = x;
    string[7] = 0x00;
    string[8] = y;
    string[9] = 0x00;    
    write(s, string, 12);
    Sleep( )

/*
	*((uint16_t *) string) = msgId++;
	//int ID; 
	int src = 14;
	int dst = 0XFF;
	int type = MSG_OBSTACLEMS;
	int act; //act is 0 if the robot dropped obstacle or 1 if it picked it up.
	int x;
	int y;
*/
}
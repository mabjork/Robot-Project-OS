#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/socket.h>
#include <math.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#define SERV_ADDR   "dc:53:60:ad:61:90"  /* Whatever the address of the server is */
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



//ACK messages are used to acknowledge the reception of messages. They are 8-byte long
void ack(){
	*((uint16_t *) string) = msgId++; //msg id
	//int id; //is a 2-byte number identifying the message (kind of like a sequence number). It is used when acknowledging messages
	int src = TEAM_ID; //team who sent the message
	int dst;  //team who should receive the message
	int type = MSG_ACK; //identifies the kind of message that is sent
	int idack; //ID of the message that is acknowledged
	int state; //state is a status code. 0 -> OK, 1 -> error. Other status codes may be used for acknowledging custom messages.
	
	//Messages sent by the server should not be acknowledged.
}

//START messages can only be used by the server. One is sent to each team when the game starts. If the robot disconnects and reconnects
//during the game, another START message will be sent to it right after it connects to the server. They are 8-byte long

//STOP messages are sent by server to every robot when the game ends. They are 5-bytes long

//KICK messages can only be sent by the server. This message is used to advertise that a robot got kicked out of the game. It is sent
//to every robot in the game. The message is 6-bytes long


//POSITION messages must be sent by robots every 2 seconds. This message is used to advertise the expected position of the robot.
//The message is 9-bytes long:
void position(){
	*((uint16_t *) string) = msgId++;
	//int ID; 
	int src = TEAM_ID;
	int dst = 0XFF;
	int type = MSG_POSITION;
	int x; //This field is a signed 16-bits little-endian integer.
	int y; //This field is a signed 16-bits little-endian integer.

}
//After the entire map has been generated, the robot sends the server each 5x5 cm grid one pixel at a time.
void mapdata(){
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

}

//These messages signal to the server that their map is finished.
void mapdone(){
	*((uint16_t *) string) = msgId++;
	//int ID; 
	int src = 14;
	int dst = 0XFF;
	int type = MSG_MAPDONE;
}

//OBSTACLE messages must be sent when a robot picks up or drop an obstacle.
void obstacle(){
	*((uint16_t *) string) = msgId++;
	//int ID; 
	int src = 14;
	int dst = 0XFF;
	int type = MSG_OBSTACLEMS;
	int act; //act is 0 if the robot dropped obstacle or 1 if it picked it up.
	int x;
	int y;

}

//possible to add custom messages...
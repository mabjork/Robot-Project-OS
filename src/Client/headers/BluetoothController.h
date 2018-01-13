#ifndef ROBOT_PROJECT_BLUETOOTHCONTROLLER_H
#define ROBOT_PROJECT_BLUETOOTHCONTROLLER_H

int bt_connect();
int bt_check();
int bt_close();

ssize_t bt_ack(uint16_t ackId, uint8_t dest, uint8_t statusCode);
ssize_t bt_position();
ssize_t bt_obstacle(uint8_t val);

int bt_wait_startmsg(char * msg);
int bt_wait_stopmsg(char * msg);
int bt_wait_kick(char * msg);

void bt_transmit();
void bt_stop();

int read_from_server (int sock, char *buffer, size_t maxSize); 


#endif //ROBOT_PROJECT_BLUETOOTHCONTROLLER_H

#ifndef ROBOT_PROJECT_BLUETOOTHCONTROLLER_H
#define ROBOT_PROJECT_BLUETOOTHCONTROLLER_H

void btcommunication();
void positionprint();
void stopmessage();
//void write_to_server();

int read_from_server (int sock, char *buffer, size_t maxSize); 


#endif //ROBOT_PROJECT_BLUETOOTHCONTROLLER_H

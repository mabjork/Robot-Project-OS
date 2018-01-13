#ifndef SENSORCONTROLLER_H   /* Include guard */
#define SENSORCONTROLLER_H

float last_gyro_read;

float getDistanceSensorValue();
float getCompassDegrees();
float getGyroDegrees();
void calibrateGyro();

#endif
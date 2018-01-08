#ifndef SENSORCONTROLLER_H   /* Include guard */
#define SENSORCONTROLLER_H

float getDistanceSensorValue();
float getCompassDegrees();
float getGyroDegrees();
void calibrateGyro();

int recognizeObject();


#endif
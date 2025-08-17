/*
 * sensor-data.c
 *
 *  Created on: Sep 30, 2024
 *      Author: armondor
 */
#include "sensor-data.h"


//float sensor_data_array[181];

//float ping_sensor_data[181];

void set_sensor_array(int location, float value) {
    sensor_data_array[location] = value;
}

void set_ping_array(int location, float value) {
    ping_sensor_array[location] = value;
}

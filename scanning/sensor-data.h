#ifndef SENSOR_DATA_H 
#define SENSOR_DATA_H 

float sensor_data_array[181];

float ping_sensor_array[181];

void set_sensor_array(int location, float value);

void set_ping_array(int location, float value);

#endif 

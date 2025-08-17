/*
 * object_scan.c
 *
 *  Created on: Dec 7, 2024
 *      Author: dawud
 */

#include "object_scan.h"

#define MAX_DIST 50 // cm
#define DZ_DIAMETER 115 //mm

//Creates a new object in the array, stores sensor distance and the angle the object was first detected at
void new_object(float distance, int start_angle)
{
    objects[num_objects].start_dist = distance;
    objects[num_objects].start_angle = start_angle;
    objects[num_objects].end_angle = 0;
    objects[num_objects].end_dist = 0;
    num_objects++;
}

// scans 180 degrees and records the IR distance at each angle. If IR value is within object range, record PING distance.
void preform_scan()
{
    //move servo to 0 deg
    servo_move(0);
    char message[100] = "";
    //declare angle
    int angle;
    //declare/initialize ir and ping vals 
    int IR_raw;
    float ping = 0;
    float real_distIR;
    //do a sweep from 0 degrees to 180 degrees
    for (angle = 0; angle <= 180; angle++)
    {
        //move to current angle and read the IR
        servo_move(angle);
        IR_raw = adc_IR_read();

        //find the real distance
        real_distIR = find_IR_distance(IR_raw);

        //if the converted distance is less than the max distance (50 cm), then run a ping.
        if (real_distIR < MAX_DIST)
        {
            ping = ping_read();
        }

        //populate sensor array with current distance at angle/ANGLE_STEP index
        set_sensor_array(angle, real_distIR);
        set_ping_array(angle, ping);

        //debug; not used in final build
        // sprintf(message, "Angle: %d IR: %0.2f Raw: %d Ping: %0.2f\n\r", angle,
        //         real_distIR, IR_raw, ping);
        //lcd_printf("%s", message);

        //   uart_sendStr(message);
    }
    //move servo back to zero
    servo_move(0);
}

// detects objects using the array of IR values
void detect_objects()
{
    //declare/initialize values
    int angle;
    char obj_scan_ongoing = 0;
    num_objects = 0;

    // clear objects array
    memset(objects, 0, sizeof(objects));

    // if an object is detected at angle 0, create a new object @ 0.
    if (sensor_data_array[0] <= MAX_DIST)
    {
        obj_scan_ongoing = 1;
        new_object(sensor_data_array[0], 0);
    }

    // iterate through the IR values at every angle
    for (angle = 0; angle < 181; angle++)
    {

        // if there IR is over MAX_DIST, end object or ignore
        if (sensor_data_array[angle] >= MAX_DIST)
        {
            if (obj_scan_ongoing)
            {
                objects[num_objects - 1].end_angle = angle - 1;
                objects[num_objects - 1].end_dist =
                        ping_sensor_array[angle - 1];

                // TODO: might want to switch to linear_width
                // when ending an object, if its angle width is < 3 degrees, assume its error and ignore it
                if ((objects[num_objects - 1].end_angle
                        - objects[num_objects - 1].start_angle) < 3)
                {
                    num_objects--;
                }
            }
            obj_scan_ongoing = 0;
        }
        // if not over MAX_DIST, its part of an object
        else
        {
            // if no object is currently being detected, start a new object
            if (obj_scan_ongoing == 0)
            {
                new_object(ping_sensor_array[angle], angle);
                obj_scan_ongoing = 1;
            }
        }

    }
    // if object sill ongoing, add last edge point at 180;
    if (obj_scan_ongoing == 1)
    {
        objects[num_objects - 1].end_angle = 180;
        objects[num_objects - 1].end_dist = ping_sensor_array[angle - 1];
    }
}

// print information of all objects. debug method.
void print_object_info()
{
    //Print header information from object array to putty
    char message[100];
    sprintf(message, "Object#     Angle   Start_DIST   End_DIST   Width\n\r");

    uart_sendStr(message);

    lcd_printf("%d", num_objects);

    //Print object information from object array to putty
    int i;
    for (i = 0; i < num_objects; i++)
    {
        // set_linear_width(&objects[i]);
        sprintf(message, "%d %d %.2f %.2f %2d\n\r", i + 1,
                objects[i].start_angle, objects[i].start_dist,
                objects[i].end_dist,
                (objects[i].end_angle - objects[i].start_angle));

        uart_sendStr(message);
    }
}

// preforms a 180 scan, detects objects, and records object edge points in obstacle array.
void scan_for_objects()
{
    //MAKE SURE IR(adc), PING, AND SERVO ARE INITIALIZED IN MAIN

    // reset obstacle edges
    memset(obstacles_arr, 0, sizeof(obstacles_arr));
    num_obstacles = 0;

    // scan field from 0 to 180 using IR and PING
    preform_scan();

    // find objects and print info
    detect_objects();
   // print_object_info();

    // create edge points for each detected object
    // stores edges in obstacle_arr
    int i;
    for (i = 0; i < num_objects; i++)
    {
        find_obstacle_location(&bot_location, objects[i].start_dist,
                               objects[i].start_angle);
        find_obstacle_location(&bot_location, objects[i].end_dist,
                               objects[i].end_angle);
    }

    // print obstacle edge points to gui
    char message[100];
    for (i = 0; i < num_obstacles; i++)
    {
        sprintf(message, "e(%d,%d)\n", obstacles_arr[i].x, obstacles_arr[i].y);
        uart_sendStr(message);
    }
}

//performs a scan at the delivery zone to recalibrate the bot's grid.
int delivery_zone_scan(position *bot)
{
    // face 90 degrees
    int cwDiff = calc_cw_degree_differential(bot->deg, 90);
    //rotate in the most efficient way
    if ((360 - cwDiff) > 180)
    {
        turn_clockwise_live_update(cwDiff, bot);
    }
    else
    {
        turn_counterclockwise_live_update(360 - cwDiff, bot);
    }
    //wait half a sec
    timer_waitMillis(500);

    //this block moves into the white tape as a way to calibrate the y.
    int white_tape_check = 0;
    while (!white_tape_check)
    {
        white_tape_check = move_forward(bot, 10, 0);
    }
    timer_waitMillis(500);

    //move back by ten cm so we have an established y to scan from
    move_backward_live_update(100, bot);
    timer_waitMillis(500);

    //scan then analyze scan data to look for two similar sized pillars that are a known distance apart
    // scan field from 0 to 180 using IR and PING
    preform_scan();
    detect_objects();

    // found obj and flag
    object_t found_obj;
    int obj_found = 0;

    // we have objects array of angles/distances
    int i;
    //iterate through scan data
    for (i = 0; i < num_objects; i++)
    {
        // reset obstacle edges
        memset(obstacles_arr, 0, sizeof(obstacles_arr));
        num_obstacles = 0;

        // detect edge points relative to where the bot thinks it is
        find_obstacle_location(&bot_location, objects[i].start_dist,
                               objects[i].start_angle);
        find_obstacle_location(&bot_location, objects[i].end_dist,
                               objects[i].end_angle);
        //extrapolate an object out of edges
        coord_t edge_1 = obstacles_arr[0];
        coord_t edge_2 = obstacles_arr[1];

        float diameter = find_resultant(edge_1, edge_2);


        //if we're within the margin of what the calibration object should be, then we're good!
        if ((diameter > DZ_DIAMETER - 50) && (diameter < DZ_DIAMETER + 50))
        {
            found_obj = objects[i];
            obj_found = 1;
            break;
        }
    }

    // reset obstacle edges
    memset(obstacles_arr, 0, sizeof(obstacles_arr));
    num_obstacles = 0;

    //if we did find the object, then we can calibrate
    if (obj_found)
    {
        float angle1 = (float) found_obj.start_angle;
        float angle2 = (float) found_obj.end_angle;

        // x diff for servo same as for bot if at 90 degrees
        int x_diff_1 = round(
               found_obj.start_dist * cos((angle1) * (M_PI / 180.0)));

        int x_diff_2 = round(
                found_obj.end_dist * cos((angle2) * (M_PI / 180.0)));

        int avg_diff = (x_diff_1 + x_diff_2) / 2;

        // x pos of x-most pillar's outer edge is at 1830
        bot->x = 1830 - avg_diff;
        return 1;
    }
    else
    {
        return 0;
    }
}


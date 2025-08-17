/*
 * obstacle_detection.c
 *
 *  Created on: Dec 2, 2024
 *      Author: dawud
 */

#include "obstacle_detection.h"

/*
 * "bot" refers to middle of cybot
 *
 * a = ping_dist
 * b = bot_to_obj = sqrt(a^2 + c^2 - (2ac * cos(B))
 * c = bot_to_servo
 *
 * angle A = bot_to_obj_angle = acos((b^2 + c^2 - a^2) / (2bc))
 * angle B = 90 + servo_bot_angle
 *
 * point A = bot_pos
 * point B = servo_pos: x = bot.x + c * cos(bot.deg) ; y = bot.y + c * sin(bot.deg)
 * point C = obj_pos: x = servo.x + a * cos(servo_grid) ; y = servo.y + a * sin(servo_grid)
 */

// finds the angle of servo relative to grid mapping
int find_servo_grid_angle(position *bot_pos, int servo_bot_angle)
{
    //declare return value
    int servo_grid_angle;

    // servo to grid angle is equal to bots angle if at 90, otherwise add the difference
    if (servo_bot_angle > 90)
    {
        servo_grid_angle = bot_pos->deg + (servo_bot_angle - 90);
        // wrap around if angle > 360
        if (servo_grid_angle >= 360)
        {
            servo_grid_angle -= 360;
        }
    }
    else
    {
        servo_grid_angle = bot_pos->deg - (90 - servo_bot_angle);
        // wrap around if angle < 0
        if (servo_grid_angle < 0)
        {
            servo_grid_angle += 360;
        }
    }
    return servo_grid_angle;
}

// finds the grid coord of the servo
// represents vector point B
coord_t find_servo_pos(position *bot_pos)
{
    //declare return value
    coord_t servo_pos;
    //find the bot's absolute degree and convert to radians
    double bot_angle_rad = (double) bot_pos->deg * (M_PI / 180.0);
    //servo position is calculated using some trig.
    servo_pos.x = round(bot_pos->x + (CENTER_TO_SERVO * cos(bot_angle_rad)));
    servo_pos.y = round(bot_pos->y + (CENTER_TO_SERVO * sin(bot_angle_rad)));
    //return the position.
    return servo_pos;
}

// finds bot-to-object vector by using the scanner-to-object and bot-to-scanner vectors
// servo_bot_angle is the angle relative to the bot
// represents vector point C
void find_obstacle_location(position *bot_pos, int ping_dist,
                            int servo_bot_angle)
{
    //finds the absolute angle of the servo.
    int servo_grid_angle = find_servo_grid_angle(bot_pos, servo_bot_angle);

    //convert to radians
    double servo_grid_rad = (double) servo_grid_angle * (M_PI / 180.0);

    //finds the location of the servo.
    coord_t servo_pos = find_servo_pos(bot_pos);

    //finds the position of an obstacle using the servo position and some trig using the ping.
    coord_t obstacle_pos;
    obstacle_pos.x = round(servo_pos.x + ((double)ping_dist * cos(servo_grid_rad)));
    obstacle_pos.y = round(servo_pos.y + ((double)ping_dist * sin(servo_grid_rad)));

    //throw position into the edges array
    obstacles_arr[num_obstacles] = obstacle_pos;
    //increment number of edges found
    num_obstacles++;
}

// finds the distance from the center of the bot to the obstacle point in mm
// represents side b
int find_bot_obstacle_dist(int ping_dist, int servo_bot_angle)
{
    //declare the distance and angle
    int dist;
    int angle_B_deg;

    // find the b angle, which is relative to the servo's angle
    if (servo_bot_angle > 90)
    {
        angle_B_deg = servo_bot_angle + 90;
    }
    else
    {
        angle_B_deg = 90 + (180 - servo_bot_angle);
    }

    //convert to radians
    double angle_B_rad = (double) (angle_B_deg) * (M_PI / 180.0);

    //use pythagorean and trig to find the distance from the bot to the point found from the ping distance.
    dist = round(
            sqrt(pow(ping_dist, 2) + pow(CENTER_TO_SERVO, 2)
                    - (2 * (double) ping_dist * CENTER_TO_SERVO
                            * cos(angle_B_rad))));
    //return the value
    return dist;
}

// finds the angle between the bots direction and the obstacle
// represents angle A
int find_angle_to_obstacle(position *bot_pos, coord_t obstacle_pos,
                           int ping_dist, int servo_bot_angle)
{
    //declare value
    double bot_obs_angle_rad;

    //find the distance from the bot to the obstacle
    int bot_obs_dist = find_bot_obstacle_dist(ping_dist, servo_bot_angle);

    //find the angle from the bot to the obstacle by finding the arccosine.
    bot_obs_angle_rad = acos(
            (double) (pow(bot_obs_dist, 2) + pow(CENTER_TO_SERVO, 2)
                    - pow(ping_dist, 2))
                    / (double) (2 * bot_obs_dist * CENTER_TO_SERVO));

    // if obstacle is scanned on the left side of the bot
    // then angle should be negative indicating to move counterClockwise
    if (servo_bot_angle > 90)
    {
        bot_obs_angle_rad *= -1;
    }

    //return the value, converted to degrees.
    return (round(bot_obs_angle_rad * (M_PI * 180)));
}

//finds the angle from the current position to a coordinate.
int find_angle_to_obstacle_coord(position *bot_pos, coord_t obstacle_pos)
{
    float y_diff = (float)(bot_pos->y - obstacle_pos.y);
    float x_diff = (float)(bot_pos->x - obstacle_pos.x);
    float angle = round(atan(y_diff / x_diff) * (180.0 / M_PI));
    return (int)angle;
}

// used to calibrate the cliff sensors, lcd prints the value of each cliff sensor
void cliff_calibrate()
{
    //init the lcd screen
    lcd_init();
    //allocate the oi
    oi_t *sensor = oi_alloc();
    oi_init(sensor);
    //get values and display them.
    while (1)
    {
        int cliff_FL = getCliffLeftFront();
        int cliff_L = getCliffLeft();
        int cliff_FR = getCliffRightFront();
        int cliff_R = getCliffRight();
        lcd_printf("FrontL: %d \nLeft: %d \nFrontR: %d \nRight: %d", cliff_FL,
                   cliff_L, cliff_FR, cliff_R);
        timer_waitMillis(3000);
    }
}

//finds the resultant betweens two points using the pythagorean theorem.
float find_resultant(coord_t a, coord_t b)
{

    return (float) sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));
}


    // find object coord, with respect to the bot servo ~70mm away
void create_nono(position* bot, int invis) {
    if (invis == 3 || invis == 19 || invis == 21 || invis == 23)
    {
        // mark position on right
        find_obstacle_location(bot, 70, 45);
    }
    else if (invis == 1 || invis == 9 || invis == 11 || invis == 15)
    {
        // mark position on left
        find_obstacle_location(bot, 70, 135);
    }
    if (invis == 13 || invis == 2)
    {
        // mark position in front
        find_obstacle_location(bot, 70, 90);
    }

    // when an object is hit, creates a position variable at that location that we will use to avoid going back to that spot (i.e. dont go back where the object is)
    nono_zone.x = obstacles_arr[num_obstacles - 1].x;
    nono_zone.y = obstacles_arr[num_obstacles - 1].y;

    //send data to the gui
    char message[30];
    sprintf(message, "e(%d,%d)\n", nono_zone.x, nono_zone.y);
    uart_sendStr(message);
}





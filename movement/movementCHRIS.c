/*
 * movement.c
 *
 * Movement API for the CyBot.
 *
 *  Created on: Sep 12, 2024
 *      Author: itschris
 */

#include "movementCHRIS.h"

#define WHITEL 2750 //The minimum value to detect white tape
#define WHITEFL 2600 //The minimum value to detect white tape
#define WHITER 2600 //The minimum value to detect white tape
#define WHITEFR 2750 //The minimum value to detect white tape
#define BLACKL 200 //the maximum value to detect the black felt
#define BLACKFL 200 //the maximum value to detect the black felt
#define BLACKR 200 //the maximum value to detect the black felt
#define BLACKFR 200 //the maximum value to detect the black felt

// invis sensor codes
#define NORM 0 //nothing was detected on any scanner (woo!)
#define BUMPL 1 //Left bumper was hit (object on the left side of the bot)
#define BUMPLR 2 //Left and Right bumper were hit (object right infront of the bot)
#define BUMPR 3 //Right bumper was hit (object on the right side of the bot)
#define LFWHITE 4 //Left Front cliff detects white tape
#define LFWHITE_LWHITE 5 //Left Front cliff detects WHITE tape and Left cliff detects WHITE tape (edge is diagonal across the left side of the bot, oh no!)
#define LFWHITE_LBLACK 6 //Left Front cliff detects WHITE tape and Left cliff detects BLACK tape (I believe it is highly unlikely that the cliff sensors ever pick up on the edge and a hole at the same time but it is possible)
#define LFWHITE_RFWHITE 7 //Left Front cliff detects WHITE tape and Right Front cliff detects WHITE tape (front of the bot would be over the white tape, big no no)
#define LFWHITE_RFBLACK 8 //Left Front cliff detect WHITE tape and Right Front cliff detects BLACK tape (edge case)
#define LFBLACK 9 //Left Front cliff detects BLACK tape (hole is diagonal across the left front side of the bot)
#define LFBLACK_LWHITE 10 //Left Front cliff detects BLACK tape and Left cliff detects WHITE tape (edge case)
#define LFBLACK_LBLACK 11 //Left Front cliff detects BLACK tape and Left cliff detects BLACK tape (hole is diagonal across the left side of the bot)
#define LFBLACK_RFWHITE 12 //Left Front cliff detects BLACK tape and Right Front cliff detects WHITE tape (edge case)
#define LFBLACK_RFBLACK 13 //Left Front cliff detects BLACK tape and Right Front cliff detects BLACK tape (hole is right infront of the bot)
#define LWHITE 14 //Left cliff detects WHITE tape (edge is on the left side of the bot)
#define LBLACK 15 //Left cliff detects BLACK tape (hole is on the left side of the bot)
#define RFWHITE 16 //Right Front cliff detects WHITE tape (right front detects the edge)
#define RFWHITE_RWHITE 17 //Right Front cliff detects WHITE tape and Right cliff detects WHITE tape (edge is diagonal across the right side of the bot)
#define RFWHITE_RBLACK 18 //Right Front cliff detects WHITE tape and Right cliff detects BLACK tape (edge case)
#define RFBLACK 19 //Right Front cliff detects BLACK tape (right front detects a hole)
#define RFBLACK_RWHITE 20 //Right Front cliff detects BLACK tape and Right cliff detects WHITE tape (edge case)
#define RFBLACK_RBLACK 21 //Right Front cliff detects BLACK tape and Right cliff detects BLACK tape (hole is diagonal across the right side of the bot)
#define RWHITE 22 //Right cliff detects WHITE tape (edge is on the right side of the bot)
#define RBLACK 23 //Right clif detects BLACK tape (edge is on the right side of the bot)

// increments the amount of times an object is hit in a row
int hit_increment;

// a common method for helping the bot move around none border obstacles, initially made specifically for managing holes but now used for holes and bumps
void simpleHoleAvoid(position *current, int angle_turn)
{
    // remaining distance not accounted for.
    int remainder = getDistance();
    update_bot_pos(current, remainder);

    // move back 100mm
    move_backward_live_update(100, current);

    if (angle_turn < 0)
    {
        turn_clockwise_live_update(abs(angle_turn), current);
    }
    else
    {
        turn_counterclockwise_live_update(angle_turn, current);
    }
}

// if the cliff sensors pick up the edge this method will be run, in reasonable scenarios where the bot hits the edge fully turning around should get the bot back on course
void simpleEdgeAvoid(position *current)
{
    oi_setWheels(0, 0);
    int prev_angle = current->prev_deg;
    int curr_angle = current->deg;

    // remaining distance not accounted for.
    int remainder = getDistance();
    update_bot_pos(current, remainder);

    // move back 50mm
    move_backward_live_update(50, current);

    if (prev_angle > curr_angle) //before running into bump bot turned towards the right, thus turn towards the left
    {
        turn_counterclockwise_live_update(180, current);
    }
    else if (current->prev_deg < current->deg) //before running into bump bot turned towards its left, thus turn towards the right
    {
        turn_clockwise_live_update(180, current);
    }

    // set flag
    obstacle_hit = 1;
}

// check all sensors that pick up obstacles the scanners cant, determine exactly what happened and return an int code to send exactly what happened to other parts of the program
int checkInvis()
{
    int cliffLeft = getCliffLeft();
    int cliffLeftFront = getCliffLeftFront();
    int cliffRight = getCliffRight();
    int cliffRightFront = getCliffRightFront();
    int leftBump = getLeftBump();
    int rightBump = getRightBump();

    // LEFT BUMPER CHECK
    if (leftBump)
    {
        oi_setWheels(0, 0);
        // check the right bumper to know if the item that got bumped is right in front of the bot or just somewhere to the left
        if (rightBump == 1)
        {
            return BUMPLR;
        }
        return BUMPL;
    }

    // RIGHT BUMPER CHECK
    if (rightBump)
    {
        oi_setWheels(0, 0);
        return BUMPR;
    }

    // FRONT LEFT CLIFF SENSOR CHECK
    if (cliffLeftFront > WHITEFL)
    {
        oi_setWheels(0, 0);
        // reason for internal if statements: Front left could detect a cliff with Front right and the Left sensors, so if Front Left goes off check those two as well to get an accurate determination of where the cliff is
        if (cliffLeft > WHITEL)
        {
            return LFWHITE_LWHITE; //Left front and Left detect white
        }
        else if (cliffLeft < BLACKL)
        {
            return LFWHITE_LBLACK; //Left front detects white and Left detects black
        }
        else if (cliffRightFront > WHITEFR)
        {
            return LFWHITE_RFWHITE; //Left front and Right front detects white
        }
        else if (cliffRightFront < BLACKFR)
        {
            return LFWHITE_RFBLACK; //Left front detects white and Right front detects black
        }
        return LFWHITE; //Left front detects white
    }
    else if (cliffLeftFront < BLACKFL)
    {
        oi_setWheels(0, 0);
        if (cliffLeft > WHITEL)
        {
            return LFBLACK_LWHITE; //Left front detects black and Left detects white
        }
        else if (cliffLeft < BLACKL)
        {
            return LFBLACK_LBLACK; //Left front and Left detect black
        }
        else if (cliffRightFront > WHITEFR)
        {
            return LFBLACK_RFWHITE; //Left front detects black and Right front detects white
        }
        else if (cliffRightFront < BLACKFR)
        {
            return LFBLACK_RFBLACK; //Left front and Right front detect black
        }
        return LFBLACK; //Left front detects black
    }

    // LEFT CLIFF SENSOR CHECK
    if (cliffLeft > WHITEL)
    {
        oi_setWheels(0, 0);
        return LWHITE; //Left detects white
    }
    else if (cliffLeft < BLACKL)
    {
        oi_setWheels(0, 0);
        return LBLACK; //Left detects black
    }

    // FRONT RIGHT CLIFF CHECK
    if (cliffRightFront > WHITEFR)
    {
        oi_setWheels(0, 0);
        if (cliffRight > WHITER)
        {
            return RFWHITE_RWHITE; //Right front and Right detects white
        }
        else if (cliffRight < BLACKR)
        {
            return RFWHITE_RBLACK; //Right front detects white and Right detects black
        }
        return RFWHITE; //Right front detects white
    }
    else if (cliffRightFront < BLACKFR)
    {
        oi_setWheels(0, 0);
        if (cliffRight > WHITER)
        {
            return RFBLACK_RWHITE; //Right front detects black and Right detects white
        }
        else if (cliffRight < BLACKR)
        {
            return RFBLACK_RBLACK; //Right front and Right detects black
        }
        return RFBLACK; //Right front detects black
    }

    // RIGHT CLIFF SENSOR CHECK
    if (cliffRight > WHITER)
    {
        oi_setWheels(0, 0);
        return RWHITE; //Right detects white
    }
    else if (cliffRight < BLACKR)
    {
        oi_setWheels(0, 0);
        return RBLACK; //Right detects black
    }

    return NORM; //Nothing detected by any sensors
}

//invis acquired from invisCheck() above
int invisReact(position *bot)
{
    int invis = checkInvis();
    if (invis != 0)
    {
        // create nono point to avoid
        create_nono(bot, invis);
    }


    switch (invis)
    {
    case 0:
        break;
    case 1: //left bumper
        simpleHoleAvoid(bot, -45);
        break;
    case 2: //left and right bumper
        if (bot->deg > 45)
        {
            simpleHoleAvoid(bot, 45);
        }
        else
        {
            simpleHoleAvoid(bot, -45);
        }
        break;
    case 3: //right bumper
        simpleHoleAvoid(bot, 45);
        break;
    case 4: //left front: white
        simpleEdgeAvoid(bot);
        break;
    case 5: //left front: white, left: white
        simpleEdgeAvoid(bot);
        break;
    case 6: //left front: white, left: black
        simpleEdgeAvoid(bot);
        break;
    case 7: //left front: white, right front: white
        simpleEdgeAvoid(bot);
        break;
    case 8: //left front: white, right front: black
        simpleEdgeAvoid(bot);
        break;
    case 9: //left front: black
        simpleHoleAvoid(bot, -45);
        break;
    case 10: //left front: black, left: white
        simpleEdgeAvoid(bot);
        break;
    case 11: //left front: black, left: black
        simpleHoleAvoid(bot, -45);
        break;
    case 12: //left front: black, right front: white
        simpleEdgeAvoid(bot);
        break;
    case 13: //left front: black, right front: black
        simpleHoleAvoid(bot, 45);
        break;
    case 14: //left: white
        simpleEdgeAvoid(bot);
        break;
    case 15: //left: black
        simpleHoleAvoid(bot, -45);
        break;
    case 16: //right front: white
        simpleEdgeAvoid(bot);
        break;
    case 17: //right front: white, right: white
        simpleEdgeAvoid(bot);
        break;
    case 18: //right front: white, right: black
        simpleEdgeAvoid(bot);
        break;
    case 19: //right front: black
        simpleHoleAvoid(bot, 45);
        break;
    case 20: //right front: black, right: white
        simpleEdgeAvoid(bot);
        break;
    case 21: //right front: black, right: black
        simpleHoleAvoid(bot, 45);
        break;
    case 22: //right: white
        simpleEdgeAvoid(bot);
        break;
    case 23: //right: black
        simpleHoleAvoid(bot, 45);
        break;
    }
    return invis;
}

// Moves the robot forward based on a given amount of millimeters, with collision detection.
// Returns 1 if obstacle (bump/cliff) was detected
int move_forward(position *bot_pos, int distance, char avoidence)
{
    //accumulator
    int sum = 0;

    // reset bump/cliff flag
    obstacle_hit = 0;

    // code returned by invisReact()
    int sensor_code;
    getDistance();

    //initialize timer, very important for the acceleration curve.
    //timer_init();

    //used as a temp var so we don't have to do multiple timer_getMillis() calls per loop.
    //int currentM;

    oi_setWheels(150, 150);
    while (sum < distance)
    {
        if (avoidence)
        {
            sensor_code = invisReact(bot_pos);
            if (sensor_code != 0)
            {
                obstacle_hit = 1;
                return 1;
            }
        }
        else
        {
            sensor_code = checkInvis();
            if (sensor_code != 0)
            {
                obstacle_hit = 1;
                oi_setWheels(0, 0);
                return 1;
            }
        }

        sum += getDistance(); // new io function only pulling distance packet

        //currentM = timer_getMillis();
        //so essentially this is just a really simple trapezoidal acceleration curve that accelerates @ 1 m/s/s until 150 ms using ternary operators.
//        oi_setWheels(currentM > 150 ? 150 : currentM,
//                     currentM > 150 ? 150 : currentM);
    }
    oi_setWheels(0, 0);
    // lcd_printf("Sum: %d", sum);
    return 0;
}

// moves the bot forward a specified distance (in mm) and updates the bot's coordinate position
// if distance = 10, has +10mm overshoot
// distance 300 gives roughly perfect result (on bot 0)
// < 300 overshoots by < 10mm, > 300 undershoots by < 10mm
void move_forward_live_update(int distance, position *current)
{
    int i;
    int break_check = 0;
    int error_dist = (double) distance * wheel_calibrate_value;
    int distance_loop = error_dist / 50;
    int remainder = error_dist % 50;
    for (i = 0; i < distance_loop; i++)
    {
        break_check = move_forward(current, 50, 1);

        // if obstacle detected, end this function
        if (break_check)
        {
            return;
        }
        // update bot location by a dist of 50
        update_bot_pos(current, 50);
    }

    // bot 3 has almost no error
    // bot 0 has error * (1/30.0)
    double error_count = (double) distance * 0;

    break_check = move_forward(current, remainder + error_count, 1);
    update_bot_pos(current, remainder);

    // if obstacle detected, end this function
    if (break_check)
    {
        return;
    }

    // reset how much the obstacle was hit
    hit_increment = 0;
}

// Same as move_forward_live_update above, but will not avoid objects
void move_forward_live_update_no_avoidance(int distance, position *current)
{
    int i;
    int break_check = 0;
    int error_dist = (double) distance * wheel_calibrate_value;
    int distance_loop = error_dist / 50;
    int remainder = error_dist % 50;

    for (i = 0; i < distance_loop; i++)
    {
        break_check = move_forward(current, 50, 0);

        // if obstacle detected, end this function
        if (break_check)
        {
            return;
        }
        // update bot location by a dist of 50
        update_bot_pos(current, 50);
    }

    // bot 3 has almost no error
    // bot 0 has error * (1/30.0)
    double error_count = (double) distance * 0;

    break_check = move_forward(current, remainder + error_count, 0);
    update_bot_pos(current, (remainder + error_count));

    // if obstacle detected, end this function
    if (break_check)
    {
        return;
    }
}

// turns the bot a specific degree clockwise (in degrees), updates the bots local quadrant position
void turn_clockwise_live_update(int degrees, position *current)
{
    //accumulator
    double angleSum = 0;

    //REMEMBER: oi_setWheels(); has parameters of (RIGHTWHEEL,LEFTWHEEL);!
    oi_setWheels(-50, 50);

    //we're using a 2-phase function here. the wheels will go at 50mm/s until the last 15 degrees, then go 15 mm/s.
    while (angleSum < degrees - 2)
    {
        //clockwise is given in a negative value so we subtract it from the sum to make it add.
        double angle_change;
        angle_change = get_angle_degrees();
        angleSum -= angle_change;
        //lcd_printf("Sum: %f", angleSum + 2);
    }
    oi_setWheels(0, 0);

    // update degree
    current->prev_deg = current->deg;
    current->deg -= degrees;

    // wrap angle around circle
    if (current->deg < 0)
    {
        current->deg += 360;
    }

    // update quad
    if (current->deg <= 90)
    {
        current->quadrant = 1;
    }
    else if (current->deg <= 180)
    {
        current->quadrant = 2;
    }
    else if (current->deg <= 270)
    {
        current->quadrant = 3;
    }
    else
    {
        current->quadrant = 4;
    }
}

// turns the bot a specific degree counter clockwise (in degrees), updates the bots local quadrant position
void turn_counterclockwise_live_update(int degrees, position *current)
{
    //accumulator
    double angleSum = 0;

    //REMEMBER: oi_setWheels(); has parameters of (RIGHTWHEEL,LEFTWHEEL);!
    oi_setWheels(50, -50);
    //we're using a 2-phase function here. the wheels will go at 50mm/s until the last 15 degrees, then go 15 mm/s.
    while (angleSum < degrees - 2)
    {
        //counterclockwise is given positive, so we can just add it to the sum.
        double angle_change;
        angle_change = get_angle_degrees();
        angleSum += angle_change;
        //lcd_printf("Sum: %f", angleSum + 2);
    }
    //done, set wheels to 0
    oi_setWheels(0, 0);

    // update degree
    current->prev_deg = current->deg;
    current->deg += degrees;

    // wrap angle around circle
    if (current->deg >= 360)
    {
        current->deg -= 360;
    }

    // update quad
    if (current->deg <= 90)
    {
        current->quadrant = 1;
    }
    else if (current->deg <= 180)
    {
        current->quadrant = 2;
    }
    else if (current->deg <= 270)
    {
        current->quadrant = 3;
    }
    else
    {
        current->quadrant = 4;
    }
}

// moves the bot backwards a specified distance (in mm) and updates the bots coordinate position
void move_backward_live_update(int distance, position *current)
{
    int i;
    int error_dist = (double) distance * wheel_calibrate_value;
    int distance_loop = error_dist / 50;
    int remainder = error_dist % 50;
    for (i = 0; i < distance_loop; i++)
    {
        move_backward(current, 50);
        // update the bots location by 50
        update_bot_pos(current, -50);
    }

    // bot 0 has error * (1/30.0)
    // double error_count = (double) distance * (1 / 30.0);

    move_backward(current, remainder);
    update_bot_pos(current, -1 * remainder);
}

// Moves the robot backward based on a given amount of millimeters
void move_backward(position *current, int distance)
{
    //accumulator
    int sum = 0;

    //not *needed*, but good to have in case and makes sure our acceleration curve goes from 0 to 150.
    oi_setWheels(0, 0);

    oi_setWheels(-50, -50);
    while (sum < distance)
    {
        sum -= getDistance(); // new io function only pulling distance packet

        //currentM = timer_getMillis();
        //so essentially this is just a really simple trapezoidal acceleration curve that accelerates @ 1 m/s/s until 150 ms using ternary operators.
//        oi_setWheels(currentM > 150 ? 150 : currentM,
//                     currentM > 150 ? 150 : currentM);
//        lcd_clear();
//        lcd_setCursorPos(0, 2);
//        lcd_printf("Going %dmm...", distance);

    }
    int reset = getDistance();
    oi_setWheels(0, 0);
    //lcd_printf("Sum: %d", sum);
}

// updates the bots coord based on its current angle and the given distance.
void update_bot_pos(position *current, int distance)
{
    switch (current->quadrant)
    {
    case 1:
        current->x += (int) (round(
                distance * cos(((float) current->deg * M_PI) / 180.0f)));
        current->y += (int) (round(
                distance * sin(((float) current->deg * M_PI) / 180.0f)));
        break;
    case 2:
        current->x -=
                (int) (round(
                        distance
                                * cos(((180.0 - (float) current->deg) * M_PI)
                                        / 180.0f)));
        current->y +=
                (int) (round(
                        distance
                                * sin(((180.0 - (float) current->deg) * M_PI)
                                        / 180.0f)));
        break;
    case 3:
        current->x -=
                (int) (round(
                        distance
                                * cos((((float) current->deg - 180.0) * M_PI)
                                        / 180.0f)));
        current->y -=
                (int) (round(
                        distance
                                * sin((((float) current->deg - 180.0) * M_PI)
                                        / 180.0f)));
        break;
    case 4:
        current->x +=
                (int) (round(
                        distance
                                * cos(((360.0 - (float) current->deg) * M_PI)
                                        / 180.0f)));
        current->y -=
                (int) (round(
                        distance
                                * sin(((360.0 - (float) current->deg) * M_PI)
                                        / 180.0f)));
        break;
    }

    char message[100];
    sprintf(message, "b(%d,%d)\n", current->x, current->y);
    uart_sendStr(message);
}


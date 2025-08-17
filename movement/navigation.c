/*
 *   navigation.c
 *   authored by chris
 *   https://www.youtube.com/watch?v=_rNUMlJ3P4k
 *   created 11/18/24
 */

#include "navigation.h"

//navigates from the current bot position to the destination in the quickest route possible.
void move_easiest(position *current, coord_t destination, oi_t *sensor)
{
    //this is just the pythagorean theorem to find a resultant vector rounded to nearest whole mm.
    int resultant = (int) (round(
            sqrt(pow((current->x - destination.x), 2)
                    + pow((current->y - destination.y), 2))));

    //takes the arccos of (delta x)/(resultant) to get the change in degrees from the x component, which is then converted to degrees
    int destination_angle = (round(
            (acos((destination.x - current->x) / (float) resultant) * 180.0f)
                    / M_PI));

    //going to be the differential in degree.
    int newDeg;

    //logic for how the degrees should increment / decrement.
    if (destination.x - current->x < 0)
    {
        if (destination.y - current->y < 0)
        {
            //in this case, both components are negative. rotation will be towards quadrant 3.
            newDeg = 180 + destination_angle;
            current->quadrant = 3;
        }
        else
        {
            //in this case, x component is negative, y component is positive. rotation will be towards quadrant 2.
            newDeg = 180 - destination_angle;
            current->quadrant = 2;
        }
    }
    else if (destination.x - current->x > 0)
    {
        if (destination.y - current->y < 0)
        {
            //in this case, x component is positive, y component is negative. rotation will be towards quadrant 4.
            newDeg = 360 - destination_angle;
            current->quadrant = 4;
        }
        else
        {
            //in this case, both components are positive. rotation will be towards quadrant 1.
            newDeg = 0 + destination_angle;
            current->quadrant = 1;
        }
    }

    //calculate the degree differential from the current direction to the new direction.
    int cwDiff = calc_cw_degree_differential(current->deg, newDeg);

    //checks for shortest path. if this condition is true, then CW rotation for cwDiff degrees is most efficient, otherwise CCW rotation for (360-cwDiff) degrees is most efficient.
    if ((360 - cwDiff) > 180)
    {
        //use if no pointer method
        //current->deg = turn_clockwise(sensor, cwDiff);
        turn_clockwise_live_update(cwDiff, current);
    }
    else
    {
        //use if no pointer method
        //current->deg = turn_counterclockwise(sensor, 360 - cwDiff);
        turn_counterclockwise_live_update(360 - cwDiff, current);
    }

    //move according to the resultant.
    move_forward_live_update(resultant, current);
//    below is what you should do if you don't pass through the position pointer into your movement method.
//    int realres = move_forward_send_travelled(sensor, resultant);
//    //as of right now, you could add the differential and be most of the way there. that's what we'll be doing for now, but i'd love invert the true vector to the true coordinate updates at some point.
//    current -> x += (current -> x - destination.x);
//    current -> y += (current -> y - destination.y);
}

//rotates the bot toward the destination in such a way that the path of the ideal resultant is at the 90 degree scan point.
void rotate_towards(position *current, coord_t destination)
{
    //this is just the pythagorean theorem to find a resultant vector rounded to nearest whole mm.
    double resultant = sqrt(
            pow((current->x - destination.x), 2)
                    + pow((current->y - destination.y), 2));

    //takes the arccos of (delta x)/(resultant) to get the change in degrees from the x component, which is then converted to degrees
    int destination_angle = round(
            (acos((double) (destination.x - current->x) / resultant) * 180.0)
                    / M_PI);

    //going to be the differential in degree.
    int newDeg;

    //logic for how the degrees should increment / decrement.
    if (destination.x - current->x < 0)
    {
        if (destination.y - current->y < 0)
        {
            //in this case, both components are negative. rotation will be towards quadrant 3.
            newDeg = 360 - destination_angle;
            current->quadrant = 3;
        }
        else
        {
            //in this case, x component is negative, y component is positive. rotation will be towards quadrant 2.
            newDeg = destination_angle;
            current->quadrant = 2;
        }
    }
    else if (destination.x - current->x > 0)
    {
        if (destination.y - current->y < 0)
        {
            //in this case, x component is positive, y component is negative. rotation will be towards quadrant 4.
            newDeg = 360 - destination_angle;
            current->quadrant = 4;
        }
        else
        {
            //in this case, both components are positive. rotation will be towards quadrant 1.
            newDeg = 0 + destination_angle;
            current->quadrant = 1;
        }
    }

    //calculate the degree differential from the current direction to the new direction.
    int cwDiff = calc_cw_degree_differential(current->deg, newDeg);

    //debug, not used in final build
    // char message[100];
    // sprintf(message, "Rotating from %d to %d. At (%d, %d)\n\r", current->deg,
    //         newDeg, current->x, current->y);
    // uart_sendStr(message);

    //checks for shortest path. if this condition is true, then CW rotation for cwDiff degrees is most efficient, otherwise CCW rotation for (360-cwDiff) degrees is most efficient.
    if ((360 - cwDiff) > 180)
    {
        turn_clockwise_live_update(cwDiff, current);
    }
    else
    {
        turn_counterclockwise_live_update(360 - cwDiff, current);
    }
}

//returns the amount of degrees needed to rotate clockwise from one point to another.
int calc_cw_degree_differential(int start, int end)
{
    // if start is greater then end, return that value, else it will be negative so add 360
    if ((start - end) > 0)
    {
        return start - end;
    }
    else
    {
        return (start - end) + 360;
    }
}

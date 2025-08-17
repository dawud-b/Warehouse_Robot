/*
 * main.c
 *
 *  Created on: Nov 25, 2024
 *      Author: dawud
 */

#include "navigation.h"
#include "grid.h"
#include "lcd.h"
#include "adc.h"
#include <Math.h>
#include "open_interface.h"
#include "movementCHRIS.h"
#include "Timer.h"
#include "uart.h"
#include "sensor-data.h"
#include "servo.h"
#include "obstacle_detection.h"
#include "object_scan.h"
#include "messynavigation.h"

extern volatile int obstacle_hit;

#define BOT_DIAMETER 350 // bot diameter = 350mm, add ~20mm for error

void main()
{
    //    right_calibration_value = 343670; //bot 1318-04
    //    left_calibration_value = 315430;

    //    right_calibration_value = 344070; //bot 2041-07
    //    left_calibration_value = 316080;

    right_calibration_value = 344420; //bot 2041-03
    left_calibration_value = 314710;

//    right_calibration_value = 345140; //bot 2041-01
//    left_calibration_value = 317150;

//    right_calibration_value = 343640; //bot 2041-14
//    left_calibration_value = 314750;

//    right_calibration_value = 344230; //bot 2041-04
//    left_calibration_value = 317730;

//    right_calibration_value = 344720; //bot 2041-00
//    left_calibration_value = 317190;

    // ----------------  CALIBRATION FUNCTIONS -----------------------
    // calibrate_IR();
    //calibrate_servo();
    // cliff_calibrate();

    // bot 03
    wheel_calibrate_value = 0.80;

    // initialization
    timer_init();
    uart_init();
    lcd_init();
    oi_t *sensor = oi_alloc();
    oi_init(sensor);
    uart_interrupt_init();
    adc_init();
    ping_init();
    servo_init();
    grid_init();
    messy_nav_init();

    // set bot starting position
    bot_location.x = charging_station.x; // bot starts at charging station
    bot_location.y = charging_station.y;
    bot_location.deg = 90; // bot starts facing 90 degrees
    bot_location.quadrant = 1;

    char message[100];

    char uart_fl = 0;
    //uart_sendStr("Ready and waiting for signal\n\r");
    //first, stay in loop until bot receives start byte
    while (uart_fl != 's')
    {
        uart_fl = uart_receive();
    }
    //next, navigate from charging dock to delivery station
    //uart_sendStr("Going to delivery zone\n\r");

    // reset flag
    obstacle_hit = 0;

    // must get wihtin 90 of the delivery zone
    while (abs(bot_location.x - delivery_zone.x) > 500
            || abs(bot_location.y - delivery_zone.y) > 500)
    {
//        sprintf(message, "diff x: %d, diff y: %d",
//                (bot_location.x - delivery_zone.y),
//                (bot_location.y - delivery_zone.y));
//        uart_sendStr(message);

        if (!obstacle_hit)
        {
            nav_to_by_max_thirty_edge_coords(&bot_location, delivery_zone, 300);
        }
        else
        {
            // runs hit_move using nono_zone
            int move_again = 1;
            // if no good gaps found it needs to fun hit_move again
            while (move_again)
            {
                move_again = hit_move(&bot_location, nono_zone);
            }
            // create dummy point to avoid obstacle hit
            // adds dummy destination right in front of the bot (90 degrees), 500mm away
            memset(obstacles_arr, 0, sizeof(obstacles_arr));
            num_obstacles = 0;
            find_obstacle_location(&bot_location, 500, 90);
            coord_t dummy;
            dummy.x = obstacles_arr[0].x;
            dummy.y = obstacles_arr[0].y;
            // move to the dummy hopefully clearing any objects
            nav_to_by_max_thirty_edge_coords(&bot_location, dummy, 150);
        }
    }

    sprintf(message, "Im at delivery zone: (%d, %d) deg: %d\n\r",
            bot_location.x, bot_location.y, bot_location.deg);
    //  uart_sendStr(message);

    bot_location.y = (int) (GRID_Y - CENTER_TO_SERVO);
    // arrived at delivery zone, now calibrate
    int calibrated = delivery_zone_scan(&bot_location);
    sprintf(message, "I think im at (%d, %d) deg: %d\n\r", bot_location.x,
            bot_location.y, bot_location.deg);
    // uart_sendStr(message);
    if (calibrated == 0)
    {
        while (1)
        {
            playSong(0);
        }
    }

    // Second Part
    playSong(0);
    uart_sendStr("a");

    uart_fl = 0;
    while (uart_fl == 0)
    {
        uart_fl = uart_receive();
    }
    int depot;
    depot = uart_fl - 48;
    coord_t destination;
    int tile = 610;
    switch (depot)
    {
    case 1:
        destination.x = (int) round(.5 * tile);
        destination.y = (int) round(2.5 * tile);
        break;
    case 2:
        destination.x = (int) round(4.5 * tile);
        destination.y = (int) round(.5 * tile);
        break;
    case 3:
        destination.x = (int) round(6.5 * tile);
        destination.y = (int) round(1.5 * tile);
        break;
    case 4:
        destination.x = (int) round(5.5 * tile);
        destination.y = (int) round(3.5 * tile);
        break;
    }
    // reset flag
    obstacle_hit = 0;

    // must get wihtin 500mm of the depot
    while (abs(bot_location.x - destination.x) > 500
            || abs(bot_location.y - destination.y) > 500)
    {
        // for reading from putty
        //        sprintf(message, "diff x: %d, diff y: %d",
        //                (bot_location.x - delivery_zone.y),
        //                (bot_location.y - delivery_zone.y));
        //        uart_sendStr(message);

        if (!obstacle_hit)
        {
            nav_to_by_max_thirty_edge_coords(&bot_location, destination, 300);
        }
        else
        {
            // runs hit_move using nono_zone
            int move_again = 1;
            // if no good gaps found it needs to fun hit_move again
            while (move_again)
            {
                move_again = hit_move(&bot_location, nono_zone);
            }
            // create dummy point to avoid obstacle hit
            // adds dummy destination right in front of the bot (90 degrees), 500mm away
            memset(obstacles_arr, 0, sizeof(obstacles_arr));
            num_obstacles = 0;
            find_obstacle_location(&bot_location, 500, 90);
            coord_t dummy;
            dummy.x = obstacles_arr[0].x;
            dummy.y = obstacles_arr[0].y;
            // move to the dummy hopefully clearing any objects
            nav_to_by_max_thirty_edge_coords(&bot_location, dummy, 150);
        }
        char get_byte = uart_receive_nonblocking();
        if (get_byte == 's')
        {
            while (1)
            {

            }
        }
    }

    playSong(0);
    while (uart_fl != 's')
    {
        uart_fl = uart_receive();
    }

    // reset flag
    obstacle_hit = 0;

    // must get wihtin 400mm of the delivery zone
    while (abs(bot_location.x - delivery_zone.x) > 400
            || abs(bot_location.y - delivery_zone.y) > 400)
    {
        // for reading from putty
        //        sprintf(message, "diff x: %d, diff y: %d",
        //                (bot_location.x - delivery_zone.y),
        //                (bot_location.y - delivery_zone.y));
        //        uart_sendStr(message);

        if (!obstacle_hit)
        {
            nav_to_by_max_thirty_edge_coords(&bot_location, delivery_zone, 300);
        }
        else
        {
            // runs hit_move using nono_zone
            int move_again = 1;
            // if no good gaps found it needs to fun hit_move again
            while (move_again)
            {
                move_again = hit_move(&bot_location, nono_zone);
            }
            // create dummy point to avoid obstacle hit
            // adds dummy destination right in front of the bot (90 degrees), 500mm away
            memset(obstacles_arr, 0, sizeof(obstacles_arr));
            num_obstacles = 0;
            find_obstacle_location(&bot_location, 500, 90);
            coord_t dummy;
            dummy.x = obstacles_arr[0].x;
            dummy.y = obstacles_arr[0].y;
            // move to the dummy hopefully clearing any objects
            nav_to_by_max_thirty_edge_coords(&bot_location, dummy, 150);
        }
        char get_byte = uart_receive_nonblocking();
        if (get_byte == 's')
        {
            while (1)
            {

            }
        }
    }

    uart_sendStr("f");
    playSong(0);

//      sprintf(message, "Im at delivery zone: (%d, %d) deg: %d\n\r",
//              bot_location.x, bot_location.y, bot_location.deg);
//      uart_sendStr(message);
//
//      bot_location.y = (int) (GRID_Y - CENTER_TO_SERVO);
//      // arrived at delivery zone, now calibrate
//      int calibrated = delivery_zone_scan(&bot_location);
//      sprintf(message, "I think im at (%d, %d) deg: %d\n\r", bot_location.x,
//              bot_location.y, bot_location.deg);
//      uart_sendStr(message);
//      if (calibrated == 0)
//      {
//          while (1)
//          {
//              playSong(0);
//          }
//      }

    // ------------------------------- Tests -------------------------------------

    // auto-move bot to item coord test
//    set_item("book", 600, 1205);
//
//    set_item("bag", 1190, 595);
//
//    item_t book = get_item("book");
//    item_t bag = get_item("bag");
//
//    move_easiest(&bot_location, book.coord, sensor);
//
//    timer_waitMillis(3000);
//
//    move_easiest(&bot_location, bag.coord, sensor);

    // scan and mark object edges test
//    scan_for_objects();

//    // messy navigation test
//   coord_t destination;
//   destination.x = 500;
//   destination.y = 500;
//   nav_to_by_max_thirty_edge_coords(&bot_location, destination);

    // movement
    // move_forward_live_update(300, &bot_location);
//    move_backward_live_update(200, &bot_location);
//    sprintf(message, "Im at delivery zone: (%d, %d) deg: %d\n\r",
//            bot_location.x, bot_location.y, bot_location.deg);
//    uart_sendStr(message);
//    timer_waitMillis(5000);
//
//    move_backward_live_update(100, &bot_location);
//    sprintf(message, "Im at delivery zone: (%d, %d) deg: %d\n\r",
//                bot_location.x, bot_location.y, bot_location.deg);
//        uart_sendStr(message);
//    timer_waitMillis(5000);
//
//    move_backward_live_update(50, &bot_location);
//    timer_waitMillis(5000);

    oi_free(sensor);
    while (1)
    {
    };
}


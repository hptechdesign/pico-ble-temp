/**
 * @file serial.h
 * @author Huw Price
 * @brief Serial port functions for ecu_display and sensor_spoofer
 * @date 2023-04-06
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _SERIAL_H
#define _SERIAL_H

/* Includes */
#include "sensor.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* Public macros and typedefs */
#define CONTROL_FRAME_SIZE 11
#define SENSOR_FRAME_SIZE 33

/* Public typedefs and enums*/
typedef enum {
    mode_startup,
    mode_select_port,
    mode_ascii,
    mode_stream_data
} serial_modes_t;

typedef enum {             //  Units:
    fire_angle_delimit,    // N/A
    fire_angle_x10,        // Degrees
    fire_angle_x1,         // Degrees
    injector_duty_delimit, // N/A
    injector_duty_x10,     // ms
    injector_duty_x1,      // ms
    peak_hold_delimit,     // N/A
    peak_hold_x1,          // ms
    peak_hold_x0_1,        // ms
    control_crc_byte1,     // N/A
    control_crc_byte2      // N/A
} control_data_byte_t;

typedef enum {
    crank_rpm_delimit       =   0,
    crank_rpm_x1000         =   1,
    crank_rpm_x100          =   2,        
    crank_rpm_x10           =   3,
    map_delimit             =   4,
    manifold_pressure_x1000 =   5,
    manifold_pressure_x100  =   6,
    manifold_pressure_x10   =   7,
    temperature_delimit_a   =   8,
    temperature_a_delimit   =   9,
    temperature_a_x100      =  10,
    temperature_a_x10       =  11,
    temperature_a_x1        =  12,
    temperature_delimit_b   =  13,
    temperature_b_delimit   =  14,
    temperature_b_x100      =  15,
    temperature_b_x10       =  16,
    temperature_b_x1        =  17,
    oil_pressure_delimit    =  18,
    oil_pressure_x1000      =  19,
    oil_pressure_x100       =  20,
    oil_pressure_x10        =  21,
    fuel_pressure_delimit   =  22,
    fuel_pressure_x1000     =  23,
    fuel_pressure_x100      =  24,
    fuel_pressure_x10       =  25,
    intake_airflow_delimit  =  26,
    intake_airflow_res1     =  27,
    intake_airflow_res2     =  28,
    intake_airflow_res3     =  29,
    intake_airflow_res4     =  30,
    sensor_crc_byte1        =  31,
    sensor_crc_byte2        =  32
} sensor_data_byte_t;

extern int ser_payloadsSent;
extern uint16_t crc_bytes;
/* Public function prototypes */
int serial_init(void);

// rpi_ecu_display:
void serial_getSensorData(sensor_data_t * rx_buffer);
// ecu_sensor_spoofer:
void serial_sendSensorPacket(void);

/* Ascii text string functions*/
void serial_puts(char * msg);


#endif //_SERIAL_H
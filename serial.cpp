/**
 * @file serial.c
 * @author Huw Price
 * @brief Serial port functions for ecu_display and sensor_spoofer
 * @date 2023-04-06
 *
 * @copyright Copyright (c) 2023
 *
 */

/* Includes */

#include "serial.h"
#include "sensor.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "crc.h"
#include "stdio.h"

#define ECU_SENSOR_SPOOFER
#define uart0   ((uart_inst_t *)uart0_hw)


/* Private macros and typedefs */
typedef unsigned char BYTE;

// Private functions

void serial_encapsulateSensorData(void);
void serial_SendBuf(BYTE * buf, int bufsize);

// Static variables
static int winPort = -1;     // windows COM port is always equal to (port+1)
static int port    = -1;     /* port 0 = /dev/ttyS0 (COM1 on windows) */
static int bdrate  = 115200; /* 115200 baud */
static char mode[] = {'8', 'N', '1', 0};

// Buffers for the ascii functions
static char msg[64];
static char str[2][512];
static int t = 0, r = 0;
static char buf[4096];

// Buffers for control & sensor data packets
// this buffer used in ECU_SENSOR_SPOOFER -- ifdef them out at some point
static BYTE tx_sensor_data[SENSOR_FRAME_SIZE];
static BYTE rx_control_data[CONTROL_FRAME_SIZE];

// this buffer used in RPI_ECU_DISPLAY
static BYTE rx_sensor_data[SENSOR_FRAME_SIZE];
static BYTE tx_control_data[CONTROL_FRAME_SIZE];

static serial_modes_t serial_mode;
int ser_payloadsSent          = 0;
uint16_t crc_bytes=0;

/**
 * @brief serial_sendSensorPacket
 *
 */

void serial_sendSensorPacket(void)
{
    if(serial_mode == mode_stream_data) {
        serial_encapsulateSensorData();
        // check this is actually polled tx - I've assumed it is
        serial_SendBuf(&tx_sensor_data[0], SENSOR_FRAME_SIZE);
        ser_payloadsSent++;
    }
}

/**
 * @brief serial_sendSensorPacket
 *
 * @param sensor_packet
 */

void serial_encapsulateSensorData(void)
{
    if(1) // consider checking if tx is in progress, e.g. if using interrupt
          // driven tx
    {   
        tx_sensor_data[crank_rpm_delimit] = 'S';
        tx_sensor_data[crank_rpm_x1000]   = (BYTE)(sensor_getCrankRpm() / 1000);
        tx_sensor_data[crank_rpm_x100] =
            (BYTE)((sensor_getCrankRpm() % 1000) / 100);
        tx_sensor_data[crank_rpm_x10] =
            (BYTE)((sensor_getCrankRpm() % 100) / 10);
        tx_sensor_data[map_delimit] = 'M';
        tx_sensor_data[manifold_pressure_x1000] =
            (BYTE)(sensor_getManifoldPressure() / 1000);
        tx_sensor_data[manifold_pressure_x100] =
            (BYTE)((sensor_getManifoldPressure() % 1000) / 100);
        tx_sensor_data[manifold_pressure_x10] =
            (BYTE)((sensor_getManifoldPressure() % 100) / 10);
        tx_sensor_data[temperature_delimit_a] = 'T';
        tx_sensor_data[temperature_a_delimit] = 'A';
        tx_sensor_data[temperature_a_x100] =
            (BYTE)((sensor_getTemperatureA() % 1000) / 100);
        tx_sensor_data[temperature_a_x10] =
            (BYTE)((sensor_getTemperatureA() % 100) / 10);
        tx_sensor_data[temperature_a_x1] =
            (BYTE)((sensor_getTemperatureA() % 10));
        tx_sensor_data[temperature_delimit_b] = 'T';
        tx_sensor_data[temperature_b_delimit] = 'B';
        tx_sensor_data[temperature_b_x100] =
            (BYTE)((sensor_getTemperatureB() % 1000) / 100);
        tx_sensor_data[temperature_b_x10] =
            (BYTE)((sensor_getTemperatureB() % 100) / 10);
        tx_sensor_data[temperature_b_x1] =
            (BYTE)((sensor_getTemperatureB() % 10));
        tx_sensor_data[oil_pressure_delimit] = 'P';
        tx_sensor_data[oil_pressure_x1000] =
            (BYTE)(sensor_getOilPressure() / 1000);
        tx_sensor_data[oil_pressure_x100] =
            (BYTE)((sensor_getOilPressure() % 1000) / 100);
        tx_sensor_data[oil_pressure_x10] =
            (BYTE)((sensor_getOilPressure() % 100) / 10);
        tx_sensor_data[fuel_pressure_delimit] = 'D';
        tx_sensor_data[fuel_pressure_x1000] =
            (BYTE)(sensor_getFuelPressure() / 1000);
        tx_sensor_data[fuel_pressure_x100] =
            (BYTE)((sensor_getFuelPressure() % 1000) / 100);
        tx_sensor_data[fuel_pressure_x10] =
            (BYTE)((sensor_getFuelPressure() % 100) / 10);
        tx_sensor_data[intake_airflow_delimit] = 'F';
        tx_sensor_data[intake_airflow_res1]    = (BYTE)0;
        tx_sensor_data[intake_airflow_res2]    = (BYTE)0;
        tx_sensor_data[intake_airflow_res3]    = (BYTE)0;
        tx_sensor_data[intake_airflow_res4]    = (BYTE)0;
        crc_bytes = crcFast(&tx_sensor_data[0], (size_t) 31);
        char buf[32];
        snprintf(&buf[0], 32, "\n\rCRC: %d",crc_bytes);
        printf(buf);
        tx_sensor_data[sensor_crc_byte1]       = (BYTE) ((crc_bytes>>8) & 0xFF);
        snprintf(&buf[0], 32, "\n\rBYTE 1: %d",(BYTE) ((crc_bytes>>8) & 0xFF));
        printf(buf);
        tx_sensor_data[sensor_crc_byte2]       = (BYTE) (crc_bytes & 0xFF);
        snprintf(&buf[0], 32, "\n\rBYTE 2: %d",(BYTE) (crc_bytes & 0xFF));
        printf(buf);
    }
}

/**
 * @brief serial_getControlData
 *
 * @return struct control_packet_t
 */

control_data_t serial_getControlData(void)
{
    if(serial_mode == mode_stream_data) {
        // sensor_decodeControlPacket();
        // read the next CONTROL_FRAME_SIZE bytes and return as control packet
    }
}


/**
 * @brief serial_init
 *
 *  Initialise serial port, user selects ascii or stream mode
 *
 * @return int stream mode
 */

int serial_init(void)
{
    // Initialise UART 0
    uart_init(uart0, 115200);
 
    // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);
 
    uart_puts(uart0, "Pico-ecu-sensormcu is awake!");

    snprintf(msg, sizeof(msg), "\n\rOpened port: COM%d\n", (port + 1));
    serial_puts(msg);

    serial_mode = mode_stream_data;
    printf("\nRunning in sensor sensor mode.");
    crcInit();
    return (serial_mode);
}


/**
 * @brief serial_puts
 *
 * @param msg
 */
void serial_puts(char * msg)
{
    uart_puts(uart0, msg);
}

void serial_SendBuf(BYTE * buf, int bufSize)
{
    for (int i=0; i<bufSize; i++)
    {
        uart_putc_raw(uart0, (char) *buf++);
    }
}


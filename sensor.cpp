/**
 * @file sensor.c
 * @author Huw Price
 * @brief
 * Sensor values can be spoofed by calling sensor_generateData().
 * Serial module uses the sensor_setXxx() commands to store the data in private
 * structs.
 * Application software uses the sensor_getXxxx() commands to retrieve
 * stored data.
 * @date 2023-04-06
 *
 * @copyright Copyright (c) 2023
 *
 */

/* Includes */
#include "sensor.h"
#include "serial.h"
#include <stdlib.h>

/* Private macros */
#define SPOOF_MAX_STEP_PERCENT 2
#define SPOOF_CRANK_RPM_MEDIAN 2100
#define SPOOF_MANIFOLD_PRESSURE_MEDIAN 1000
#define SPOOF_TEMPERATURE_A_MEDIAN 98
#define SPOOF_TEMPERATURE_B_MEDIAN 99
#define SPOOF_OIL_PRESSURE_MEDIAN 2500
#define SPOOF_FUEL_PRESSURE_MEDIAN 2200

/* Private function prototypes */

// ecu_display functions
void sensor_getData(void);

// sensor_spoofer functions
int sensor_spoofNext(int current, int target, int maxPercentDiff);
void sensor_generateData(void);

/* Global variables */

/* Static variables */

static control_data_t control_data = {
    .fire_angle_deg = 0, .injector_duty_ms = 0, .peak_hold_ms = 0};
static sensor_data_t sensor_data = {.crank_rpm              = 0,
                                    .manifold_pressure_mbar = 0,
                                    .temperature_a_degC     = 0,
                                    .temperature_b_degC     = 0,
                                    .oil_pressure_mbar      = 0,
                                    .fuel_pressure_bar      = 0};

/* Private functions */

/**
 * @brief sensor_spoofNext generates the next sensor through a combination of a
 * random number generator and whether the current value is above or below the
 * target value. Step size is limited by maxPercentDiff.
 *
 * @param current
 * @param target
 * @param maxPercentDiff
 * @return int
 */
int sensor_spoofNext(int current, int target, int maxPercentDiff)
{
    int r = 0;
    // limit the range of random next values
    r = rand() % target / (100 / maxPercentDiff);
    // get the next sensor value
    current > target ? (current -= r) : (current += r);
    return current;
}

/* Public functions */

/**
 * @brief sensor_generateData this function is used in the ecu_sensor_spoofer to
 * generate the next full set of sensor values
 *
 */
void sensor_generateData(void)
{

    sensor_setCrankRpm(sensor_spoofNext(
        sensor_getCrankRpm(), SPOOF_CRANK_RPM_MEDIAN, SPOOF_MAX_STEP_PERCENT));

    sensor_setManifoldPressure(sensor_spoofNext(sensor_getManifoldPressure(),
                                                SPOOF_MANIFOLD_PRESSURE_MEDIAN,
                                                SPOOF_MAX_STEP_PERCENT));

    sensor_setTemperatureA(sensor_spoofNext(sensor_getTemperatureA(),
                                            SPOOF_TEMPERATURE_A_MEDIAN,
                                            SPOOF_MAX_STEP_PERCENT));

    sensor_setTemperatureB(sensor_spoofNext(sensor_getTemperatureB(),
                                            SPOOF_TEMPERATURE_B_MEDIAN,
                                            SPOOF_MAX_STEP_PERCENT));

    sensor_setOilPressure(sensor_spoofNext(sensor_getOilPressure(),
                                           SPOOF_OIL_PRESSURE_MEDIAN,
                                           SPOOF_MAX_STEP_PERCENT));

    sensor_setFuelPressure(sensor_spoofNext(sensor_getFuelPressure(),
                                            SPOOF_FUEL_PRESSURE_MEDIAN,
                                            SPOOF_MAX_STEP_PERCENT));
}

/**
 * @brief sensor_fillBufWithCurrentData fills the given text
 * buffer with the current sensor data for printing to the terminal while
 * the ecu_sensor-spoofer runs
 *
 * @param buf
 */
void sensor_fillBufWithCurrentData(char * buf)
{
    snprintf(buf, 512, "\n\rPayload: [%6d]\n\r\
        Crank_rpm              = %4d\n\r\
        Manifold_pressure_mbar = %4d\n\r\
        Temperature_a_degC     = %4d\n\r\
        Temperature_b_degC     = %4d\n\r\
        Oil_pressure_mbar      = %4d\n\r\
        Fuel_pressure_bar      = %4d\n\r",
             ser_payloadsSent, sensor_data.crank_rpm,
             sensor_data.manifold_pressure_mbar, sensor_data.temperature_a_degC,
             sensor_data.temperature_b_degC, sensor_data.oil_pressure_mbar,
             sensor_data.fuel_pressure_bar);
}

/**
 * @brief sensor_getData this function is used in rpi_ecu_display to read the
 * next packet of sensor data from the serial rx buffer
 *
 */
void sensor_getData(void)
{
    serial_getSensorData(&sensor_data);
}

/* Getters */

uint16_t sensor_getCrankRpm(void)
{
    return sensor_data.crank_rpm;
}

uint16_t sensor_getManifoldPressure(void)
{
    return sensor_data.manifold_pressure_mbar;
}

uint16_t sensor_getTemperatureA(void)
{
    return sensor_data.temperature_a_degC;
}

uint16_t sensor_getTemperatureB(void)
{
    return sensor_data.temperature_b_degC;
}

uint16_t sensor_getOilPressure(void)
{
    return sensor_data.oil_pressure_mbar;
}

uint16_t sensor_getFuelPressure(void)
{
    return sensor_data.fuel_pressure_bar;
}

/* Setters */

void sensor_setCrankRpm(uint16_t rpm)
{
    sensor_data.crank_rpm = rpm;
}

void sensor_setManifoldPressure(uint16_t pressure_mbar)
{
    sensor_data.manifold_pressure_mbar = pressure_mbar;
}

void sensor_setTemperatureA(uint16_t temperature_degC)
{
    sensor_data.temperature_a_degC = temperature_degC;
}

void sensor_setTemperatureB(uint16_t temperature_degC)
{
    sensor_data.temperature_b_degC = temperature_degC;
}

void sensor_setOilPressure(uint16_t pressure_mbar)
{
    sensor_data.oil_pressure_mbar = pressure_mbar;
}

void sensor_setFuelPressure(uint16_t pressure_bar)
{
    sensor_data.fuel_pressure_bar = pressure_bar;
}
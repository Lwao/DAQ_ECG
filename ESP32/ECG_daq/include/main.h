/**
 * @file main.h
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date October 9 2021
 */

#ifndef _MAIN_H_ 
#define _MAIN_H_

/*
 * Include section
 * --------------------
 * Importing all necessary libraries for the good maintenance of the code
 */

#ifndef C_POSIX_LIB_INCLUDED
    #define C_POSIX_LIB_INCLUDED
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <sys/unistd.h>
    #include <sys/stat.h>
#endif //C_POSIX_LIB_INCLUDED

#ifndef ESP_MANAGEMENT_LIBS_INCLUDED
    #define ESP_MANAGEMENT_LIBS_INCLUDED
    #include "esp_err.h" // error codes and helper functions
    #include "esp_log.h" // logging library
#endif //ESP_MANAGEMENT_LIBS_INCLUDED

#ifndef DRIVERS_INCLUDED
    #define DRIVERS_INCLUDED
    #include "driver/gpio.h"
    #include "driver/periph_ctrl.h"
    #include "driver/adc.h"
    #include "esp_adc_cal.h"
#endif //DRIVERS_INCLUDED

#ifndef ADC_LIB_INCLUDED
#define ADC_LIB_INCLUDED
#include <driver/adc.h>
#include "esp_adc_cal.h"
#endif

#ifndef FREERTOS_LIB_INCLUDED
    #define FREERTOS_LIB_INCLUDED
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "freertos/queue.h"
    #include "freertos/event_groups.h"
#endif //FREERTOS_LIB_INCLUDED


/*
 * Define section
 * --------------------
 * Definition of macros to be used globally in the code
 */


// pins
#define BTN_ON_OFF            GPIO_NUM_0 // gpio 0  - button
#define ESP_INTR_FLAG_DEFAULT 0

// buffers
#define BUFFER_LEN    1024
#define NUM_BUFFERS   30

// gpio
#define GPIO_INPUT_PIN_SEL1 (1ULL<<BTN_ON_OFF)  // | (1ULL<<ANOTHER_GPIO)

// event flags

#define BIT_(shift) (1<<shift)

enum events{SYSTEM_STARTED,
            ENABLE_ADC_READING,
            ENABLE_SIGNAL_PROCESSING,
            ENABLE_TRANSMISSION,
            BTN_ON_TASK,
            BTN_OFF_TASK};

// system macros
#define SAMPLING_TIME 1e-4

// log flags
#define APP_MAIN_TAG "app_main"
#define START_TAG    "start_task"
#define END_TAG      "end_task"
#define ADC_TAG      "adc_task"
#define DSP_TAG      "dsp_task"
#define TX_TAG       "tx_task"

/*
 * Global variable declaration section
 * --------------------
 * Initialize global variables to be used in any part of the code
 */

// config input pin - button (GPIO0 commanded by BOOT button)
gpio_config_t in_conf = {
    .intr_type    = GPIO_INTR_POSEDGE,   // interrupt on rising edge
    .mode         = GPIO_MODE_INPUT,     // set as input mode
    .pin_bit_mask = GPIO_INPUT_PIN_SEL1, // bit mask of pins to set (GPIO00)
    .pull_down_en = 1,                   // enable pull-down mode
    .pull_up_en   = 0,                   // disable pull-up mode
};

// adc variables
static esp_adc_cal_characteristics_t *adc_chars; 
static esp_adc_cal_value_t val_type;

// buffers
float adcBuffer[BUFFER_LEN];
float dspBuffer[BUFFER_LEN];
float txBuffer[BUFFER_LEN];

// freertos variables
TaskHandle_t xTaskSTARThandle; // handle to system chain to START
TaskHandle_t xTaskENDhandle; // handle to system chain to END
TaskHandle_t xTaskADChandle; // handle to ADC reading task
TaskHandle_t xTaskDSPhandle; // handle to digital signal processing task
TaskHandle_t xTaskTXhandle; // handle to data transmission task
TaskHandle_t xTaskOUThandle; // handle to ECG output task
QueueHandle_t xQueueDataADC; // data queue to store data read by ADC to be processed in DSP
QueueHandle_t xQueueDataDSP; // data queue to store data read processed by DSP to be send to TX
EventGroupHandle_t xEvents; // event group to manage tasks context

portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

/*
 * Function prototype section
 * --------------------
 * Initialize functions prototypes to later be defined
 */

/**
 * @brief Task to configure START of recording when receives command to it.
 *
 * @param pvParameters freeRTOS task parameters
 */
void vTaskSTART(void * pvParameters);

/**
 * @brief Task to configure END of recording when receives command to it.
 *
 * @param pvParameters freeRTOS task parameters
 */
void vTaskEND(void * pvParameters);

/**
 * @brief Task to read ECG data in analog domain via ADC.
 *
 * @param pvParameters freeRTOS task parameters
 */
void vTaskADC(void * pvParameters);

/**
 * @brief Task to apply digital signal processing algorithms to the incoming ECG, such as heart rate acquisition,
 *        pathology analysis, filtering etc.
 *
 * @param pvParameters freeRTOS task parameters
 */
void vTaskDSP(void * pvParameters);

/**
 * @brief Task to transmit processed data to visualization media.
 *
 * @param pvParameters freeRTOS task parameters
 */
void vTaskTX(void * pvParameters);


/**
 * @brief Interrupt service routine for button pressed (associated with BOOT button a.k.a GPIO0)
 */
static void IRAM_ATTR ISR_BTN();

void check_efuse(void);
void timer_config(int timer_idx, bool auto_reload, double tim
#endif //_MAIN_H_
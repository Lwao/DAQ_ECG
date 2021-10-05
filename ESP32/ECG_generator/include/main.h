/**
 * @file main.h
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date October 4 2021
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
    #include "esp_vfs_fat.h" // FAT filesystem support
#endif //ESP_MANAGEMENT_LIBS_INCLUDED

#ifndef DRIVERS_INCLUDED
    #define DRIVERS_INCLUDED
    #include "driver/gpio.h"
    #include "driver/periph_ctrl.h"
    #include "driver/ledc.h"
    #include "driver/dac.h"
#endif //DRIVERS_INCLUDED

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
#define BTN_CHANGE_PATHOLOGY  GPIO_NUM_0 // gpio 0  - button
#define ESP_INTR_FLAG_DEFAULT 0

// buffers
#define BUFFER_LEN    1024
#define NUM_BUFFERS   30

// gpio
#define GPIO_INPUT_PIN_SEL1 (1ULL<<BTN_CHANGE_PATHOLOGY)  // | (1ULL<<ANOTHER_GPIO)

// event flags

#define BIT_(shift) (1<<shift)

enum pathologies{NORMAL_RHYTHM, 
                SINUS_TACHYCARDIA,
                ATRIAL_FLUTTER,
                VENTRICULAR_TACHYCARDIA,
                VENTRICULAR_FLUTTER};

enum events{ENABLE_ECG_GENERATION,
            ENABLE_CHANGE_PATHOLOGY};

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

char inBuffer[BUFFER_LEN];
char outBuffer[BUFFER_LEN];

// configuration
float prev_x[4] = {0,0,0.1,0}; // previous x vector
float act_x[4]; // actual x vector
float dt=1e-4; // sampling period

// variables initialized to NORMAL_RHYTHM
const float C=1.35, beta=4;
float alpha[4] = {-0.024, 0.0216, -0.0012, 0.12};
float H=3, gammat=7;
float ecg;
const float min_=-1, max_=1;

// freertos variables
TaskHandle_t xTaskGENhandle; // handle to ECG generation task
TaskHandle_t xTaskOUThandle;  // handle to ECG output task
QueueHandle_t xQueueData;        // data queue for transfering generated data to DAC output
EventGroupHandle_t xPathologies; // event group to signalize which pathology must be simulated
EventGroupHandle_t xEvents;      // event group to manage tasks context

portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

/*
 * Function prototype section
 * --------------------
 * Initialize functions prototypes to later be defined
 */

/**
 * @brief Function responsible to change parameters in ECG signal generation so multiple pathologies can be simulated
 */
void change_pathology();

/**
 * @brief Task to generate ECG signal with Queiroz-Juárez et al., 2019 algorithm (https://doi.org/10.1038/s41598-019-55448-5).
 *
 * @param pvParameters freeRTOS task parameters
 */
void vTaskGenerateECG(void * pvParameters);

/**
 * @brief Task to output ECG signal previous buffered by generator task.
 *
 * @param pvParameters freeRTOS task parameters
 */
void vTaskOutputECG(void * pvParameters);


/**
 * @brief Interrupt service routine for button pressed (associated with BOOT button a.k.a GPIO0)
 */
static void IRAM_ATTR ISR_BTN();


#endif //_MAIN_H_
/**
 * @file main.c
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date October 9 2021
 */

#include "main.h"

/*
 * Main section
 * --------------------
 * Main function with general single time configuration in BOOT time
 */

void app_main(void)
{  
    BaseType_t xReturnedTask[5];

    // configure GPIO pins
    ESP_ERROR_CHECK(gpio_config(&in_conf));                           // initialize input pin as pathology changer
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT)); // install gpio isr service
    ESP_ERROR_CHECK(gpio_isr_handler_add(BTN_ON_OFF, ISR_BTN, NULL)); // hook isr handler for specific gpio pin
    
    // configure ADC
    //check_efuse();
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12)); // ADC 12-bit width
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC_CHANNEL_0, ADC_ATTEN_DB_0)); // ADC channel and attenuation
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, (uint32_t) 0, adc_chars);
    //print_char_val_type(val_type);

    // configure UART
    //ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
    //ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, 4, 5, 18, 19)); // Set UART pins(TX: IO4, RX: IO5, RTS: IO18, CTS: IO19)
    //ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, BUFFER_LEN, 0, 0, NULL, 0));
    
    // create queue/event groups
    xQueueDataADC = xQueueCreate(NUM_BUFFERS,BUFFER_LEN*sizeof(int)); 
    xQueueDataDSP = xQueueCreate(NUM_BUFFERS,BUFFER_LEN*sizeof(int)); 
    xEvents       = xEventGroupCreate();
    
    if((xQueueDataADC == NULL) || (xQueueDataDSP == NULL)){ // tests if queue creation fails
        ESP_LOGE("app_main", "Failed to create data queue.\n");
        while(1);
    }
    if((xEvents == NULL)){ // tests if event group creation fails
        ESP_LOGE("app_main", "Failed to create event group.\n");
        while(1);
    }

    // create tasks
    xReturnedTask[0] = xTaskCreatePinnedToCore(vTaskSTART, "taskSTART", configMINIMAL_STACK_SIZE+128, NULL, configMAX_PRIORITIES-1, &xTaskSTARThandle, PRO_CPU_NUM);
    xReturnedTask[1] = xTaskCreatePinnedToCore(vTaskEND,   "taskEND",   configMINIMAL_STACK_SIZE+1024, NULL, configMAX_PRIORITIES-1, &xTaskENDhandle,   PRO_CPU_NUM);
    xReturnedTask[2] = xTaskCreatePinnedToCore(vTaskADC,   "taskADC",   configMINIMAL_STACK_SIZE+1024, NULL, configMAX_PRIORITIES-2, &xTaskADChandle,   APP_CPU_NUM);
    xReturnedTask[3] = xTaskCreatePinnedToCore(vTaskDSP,   "taskDSP",   configMINIMAL_STACK_SIZE+1024, NULL, configMAX_PRIORITIES-2, &xTaskDSPhandle,   PRO_CPU_NUM);
    xReturnedTask[3] = xTaskCreatePinnedToCore(vTaskTX,    "taskTX",    configMINIMAL_STACK_SIZE+2014, NULL, configMAX_PRIORITIES-2, &xTaskTXhandle,    PRO_CPU_NUM);

    for(int itr=0; itr<5; itr++) // iterate over tasks 
    {
        if(xReturnedTask[itr] == pdFAIL){ // tests if task creation fails
            ESP_LOGE("app_main", "Failed to create task %d.\n", itr);
            while(1);
        }
    }

    // suspend tasks while not system started
    vTaskSuspend(xTaskENDhandle);
    vTaskSuspend(xTaskADChandle);
    vTaskSuspend(xTaskDSPhandle);
    vTaskSuspend(xTaskTXhandle);

    ESP_LOGI(APP_MAIN_TAG, "Successful BOOT!");
    vTaskDelete(NULL);
}

/*
 * freeRTOS section
 * --------------------
 * Declaration of freeRTOS tasks
 */

void vTaskSTART(void * pvParameters)
{
    while(1)
    {
        if(xEventGroupWaitBits(xEvents, BIT_(BTN_ON_TASK), pdFALSE, pdTRUE, portMAX_DELAY) & BIT_(BTN_ON_TASK))
        {
            //ESP_LOGI(START_TAG, "Start START.");

            // set flag informing that system started
            xEventGroupSetBits(xEvents, BIT_(SYSTEM_STARTED));
            xEventGroupClearBits(xEvents, BIT_(BTN_ON_TASK));

            // resume running time tasks
            vTaskResume(xTaskENDhandle);
            vTaskResume(xTaskADChandle);
            vTaskResume(xTaskDSPhandle);
            vTaskResume(xTaskTXhandle);

            // enable event flags to run tasks
            xEventGroupSetBits(xEvents, BIT_(ENABLE_ADC_READING));
            xEventGroupSetBits(xEvents, BIT_(ENABLE_SIGNAL_PROCESSING));
            xEventGroupSetBits(xEvents, BIT_(ENABLE_TRANSMISSION));

            //ESP_LOGI(START_TAG, "End START.");

            // locking task
            vTaskSuspend(xTaskSTARThandle);
        }else{vTaskDelay(1);}
    }
}

void vTaskEND(void * pvParameters)
{
    while(1)
    {
        if(xEventGroupWaitBits(xEvents, BIT_(BTN_OFF_TASK), pdFALSE, pdTRUE, portMAX_DELAY) & BIT_(BTN_OFF_TASK))
        {
            //ESP_LOGI(END_TAG, "Start END.");

            // clear flag informing that system stopped
            xEventGroupClearBits(xEvents, BIT_(SYSTEM_STARTED));
            xEventGroupClearBits(xEvents, BIT_(BTN_OFF_TASK));

            // disable event flags to run tasks
            xEventGroupClearBits(xEvents, BIT_(ENABLE_ADC_READING));
            xEventGroupClearBits(xEvents, BIT_(ENABLE_SIGNAL_PROCESSING));
            xEventGroupClearBits(xEvents, BIT_(ENABLE_TRANSMISSION));

            // suspend running time tasks and resume start task
            vTaskResume(xTaskSTARThandle);
            vTaskSuspend(xTaskADChandle);
            vTaskSuspend(xTaskDSPhandle);
            vTaskSuspend(xTaskTXhandle);

            //ESP_LOGI(END_TAG, "End END.");

            // locking task
            vTaskSuspend(xTaskENDhandle);
        }else{vTaskDelay(1);}
    }
}

void vTaskADC(void * pvParameters)
{
    uint32_t time;
    int i;
    while(1)
    {
        if(xEventGroupWaitBits(xEvents, BIT_(ENABLE_ADC_READING), pdFALSE, pdTRUE, portMAX_DELAY) & BIT_(ENABLE_ADC_READING))
        {
            //ESP_LOGI(ADC_TAG, "Start ADC.");
            time = esp_timer_get_time();
            for(i=0; i<BUFFER_LEN; i++) // adc read loop to fill buffer
            {
                //adcBuffer[i] = (float) esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_6), adc_chars) / (float) 1000; // voltage in mV to V
                //adcBuffer[i] = adc1_get_raw(ADC1_CHANNEL_6); // voltage in mV to V
                //printf("%d\n", adcBuffer[i]);
                adcBuffer[i] = (int) esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_6), adc_chars); // voltage in mV 
                while((esp_timer_get_time()-time) < (int32_t)(1e6*SAMPLING_TIME)); // hold loop until sampling time is reached
            }
            // enqueue data for next task
            xQueueSend(xQueueDataADC,&adcBuffer,portMAX_DELAY);
            //ESP_LOGI(ADC_TAG, "End ADC.");
            vTaskDelay(1);
        }else{vTaskDelay(1);}
    }
}

void vTaskDSP(void * pvParameters)
{
    //int i;
    while(1)
    {
        if(xEventGroupWaitBits(xEvents, BIT_(ENABLE_SIGNAL_PROCESSING), pdFALSE, pdTRUE, portMAX_DELAY) & BIT_(ENABLE_SIGNAL_PROCESSING))
        {
            while(xQueueDataADC!=NULL && xQueueReceive(xQueueDataADC, &dspBuffer, 0)==pdTRUE)
            {
                //ESP_LOGI(DSP_TAG, "Start DSP.");
                // enqueue data for next task
                xQueueSend(xQueueDataDSP,&dspBuffer,portMAX_DELAY);
                //ESP_LOGI(DSP_TAG, "End DSP.");
                vTaskDelay(1);
            }            
        }else{vTaskDelay(1);}
    }
}

void vTaskTX(void * pvParameters)
{
    int i;
    while(1)
    {
        if(xEventGroupWaitBits(xEvents, BIT_(ENABLE_TRANSMISSION), pdFALSE, pdTRUE, portMAX_DELAY) & BIT_(ENABLE_TRANSMISSION))
        {
            while(xQueueDataDSP!=NULL && xQueueReceive(xQueueDataDSP, &txBuffer, 0)==pdTRUE)
            {
                //ESP_LOGI(TX_TAG, "Start TX.");
                for(i=0; i<BUFFER_LEN; i++)
                {
                    printf("%d\n", txBuffer[i]);
                }
                vTaskDelay(1);
            }            
        }else{vTaskDelay(1);}
    }
}

/*
 * ISR section
 * --------------------
 * Declaration of interrupt service routines for push-buttons, timers, etc.
 */

static void IRAM_ATTR ISR_BTN()
{
    portENTER_CRITICAL_ISR(&spinlock);
    if(xEventGroupGetBitsFromISR(xEvents) & BIT_(SYSTEM_STARTED)){xEventGroupSetBits(xEvents, BIT_(BTN_OFF_TASK));} // recording started, so stop recording
    else{xEventGroupSetBits(xEvents, BIT_(BTN_ON_TASK));} // recording stopd, so start recording
    portEXIT_CRITICAL_ISR(&spinlock);
}

/*
 * Auxiliary functions section
 * --------------------
 * Declaration of auxiliary functions
 */

void check_efuse(void)
{
    //Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}
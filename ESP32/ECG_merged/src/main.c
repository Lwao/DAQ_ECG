/**
 * @file main.c
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date October 10 2021
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

    // create queue/event groups
    xQueueDataGEN = xQueueCreate(NUM_BUFFERS,BUFFER_LEN*sizeof(float)); 
    xQueueDataDSP = xQueueCreate(NUM_BUFFERS,BUFFER_LEN*sizeof(float)); 
    xEvents       = xEventGroupCreate();
    xPathologies  = xEventGroupCreate();
    
    if((xQueueDataGEN == NULL) || (xQueueDataDSP == NULL)){ // tests if queue creation fails
        ESP_LOGE("app_main", "Failed to create data queue.\n");
        while(1);
    }
    if((xEvents == NULL) || (xPathologies == NULL)){ // tests if event group creation fails
        ESP_LOGE("app_main", "Failed to create event group.\n");
        while(1);
    }

    // create tasks
    xReturnedTask[0] = xTaskCreatePinnedToCore(vTaskSTART, "taskSTART", configMINIMAL_STACK_SIZE+128, NULL, configMAX_PRIORITIES-1, &xTaskSTARThandle, PRO_CPU_NUM);
    xReturnedTask[1] = xTaskCreatePinnedToCore(vTaskEND,   "taskEND",   configMINIMAL_STACK_SIZE+1024, NULL, configMAX_PRIORITIES-1, &xTaskENDhandle,   PRO_CPU_NUM);
    xReturnedTask[2] = xTaskCreatePinnedToCore(vTaskGEN,   "taskGEN",   configMINIMAL_STACK_SIZE+4096, NULL, configMAX_PRIORITIES-2, &xTaskGENhandle,   APP_CPU_NUM);
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
    vTaskSuspend(xTaskGENhandle);
    vTaskSuspend(xTaskDSPhandle);
    vTaskSuspend(xTaskTXhandle);

    xEventGroupSetBits(xPathologies, BIT_(NORMAL_RHYTHM));


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
            vTaskResume(xTaskGENhandle);
            vTaskResume(xTaskDSPhandle);
            vTaskResume(xTaskTXhandle);

            // enable event flags to run tasks
            xEventGroupSetBits(xEvents, BIT_(ENABLE_ECG_GENERATION));
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
            xEventGroupClearBits(xEvents, BIT_(ENABLE_ECG_GENERATION));
            xEventGroupClearBits(xEvents, BIT_(ENABLE_SIGNAL_PROCESSING));
            xEventGroupClearBits(xEvents, BIT_(ENABLE_TRANSMISSION));

            // suspend running time tasks and resume start task
            vTaskResume(xTaskSTARThandle);
            vTaskSuspend(xTaskGENhandle);
            vTaskSuspend(xTaskDSPhandle);
            vTaskSuspend(xTaskTXhandle);

            //ESP_LOGI(END_TAG, "End END.");

            // locking task
            vTaskSuspend(xTaskENDhandle);
        }else{vTaskDelay(1);}
    }
}

void vTaskGEN(void * pvParameters)
{
    int i;
    while(1)
    {
         if(xEventGroupWaitBits(xEvents, BIT_(ENABLE_ECG_GENERATION), pdFALSE, pdTRUE, portMAX_DELAY) & BIT_(ENABLE_ECG_GENERATION))
        {
            if(xEventGroupGetBits(xEvents) & BIT_(ENABLE_CHANGE_PATHOLOGY))
            {
                change_pathology();
                xEventGroupClearBits(xEvents, BIT_(ENABLE_CHANGE_PATHOLOGY));
            }
            for(i=0; i<BUFFER_LEN; i++)
            {
                act_x[0] = gammat*(prev_x[0]-prev_x[1]-C*prev_x[0]*prev_x[1]-prev_x[0]*prev_x[1]*prev_x[1])*dt+prev_x[0];
                act_x[1] = gammat*(H*prev_x[0]-3*prev_x[1]+C*prev_x[0]*prev_x[1]+prev_x[0]*prev_x[1]*prev_x[1]+beta*(prev_x[3]-prev_x[1]))*dt+prev_x[1];
                act_x[2] = gammat*(prev_x[2]-prev_x[3]-C*prev_x[2]*prev_x[3]-prev_x[2]*prev_x[3]*prev_x[3])*dt+prev_x[2];
                act_x[3] = gammat*(H*prev_x[2]-3*prev_x[3]+C*prev_x[2]*prev_x[3]+prev_x[2]*prev_x[3]*prev_x[3]+2*beta*(prev_x[1]-prev_x[3]))*dt+prev_x[3];
                genBuffer[i] = alpha[0]*act_x[0]+alpha[1]*act_x[1]+alpha[2]*act_x[2]+alpha[3]*act_x[3];// + get_random(100);
                prev_x[0] = act_x[0];
                prev_x[1] = act_x[1];
                prev_x[2] = act_x[2];
                prev_x[3] = act_x[3];
            }
            // enqueue ecg data buffer
            xQueueSend(xQueueDataGEN,&genBuffer,portMAX_DELAY);
            vTaskDelay(1);
        } else{vTaskDelay(1);}
    }
}

void vTaskDSP(void * pvParameters)
{
    float thre=0.4;
    int i;
    float actualFilt, actualDFilt, actualDDFilt;
    float HR_temp;
    while(1)
    {
        if(xEventGroupWaitBits(xEvents, BIT_(ENABLE_SIGNAL_PROCESSING), pdFALSE, pdTRUE, portMAX_DELAY) & BIT_(ENABLE_SIGNAL_PROCESSING))
        {
            while(xQueueDataGEN!=NULL && xQueueReceive(xQueueDataGEN, &dspBuffer, 0)==pdTRUE)
            {
                //ESP_LOGI(DSP_TAG, "Start DSP.");

                for(i=0; i<BUFFER_LEN; i++)
                {
                    // MOVING AVERAGE FILTER
                    AVG_KERNEL[i % MOV_AVG_SIZE] = dspBuffer[i];
                    dspBuffer[i] = 0;
                    for(int j=0; j<MOV_AVG_SIZE; j++){dspBuffer[i] += AVG_KERNEL[j]/MOV_AVG_SIZE;}

                    // HEART RATE
                    if(dspBuffer[i]>thre)
                    {
                        actualFilt = dspBuffer[i];
                        actualDFilt = actualFilt-lastFilt;
                        if(actualDFilt>0){actualDFilt=1;}
                        else{actualDFilt=0;}
                        actualDDFilt = actualDFilt-lastDFilt;
                        if(actualDDFilt<0){actualDDFilt=1;}
                        else{actualDDFilt=0;}

                        if(actualDDFilt>lastDDFilt) // found peak, update heart rate
                        {
                            HR_temp = 60/(peak_count*dt);
                            if ((~isnan(HR_temp)) && (~isinf(HR_temp))){heart_rate = HR_temp;}
                            peak_count=0;
                        } else{peak_count++;}

                        lastFilt = actualFilt;
                        lastDFilt = actualDFilt;
                        lastDDFilt = actualDDFilt;
                    } else{peak_count++;}
                }
                
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
                    printf("%f\t%f\n", txBuffer[i], heart_rate);
                    //printf("%f\n", heart_rate);
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
    if(xEventGroupGetBitsFromISR(xEvents) & BIT_(SYSTEM_STARTED))
    {
        xEventGroupSetBits(xEvents, BIT_(BTN_OFF_TASK)); // recording started, so stop recording
        xEventGroupSetBits(xEvents, BIT_(ENABLE_CHANGE_PATHOLOGY)); // also change pathology for next session
    } 
    else{xEventGroupSetBits(xEvents, BIT_(BTN_ON_TASK));} // recording stopd, so start recording
    portEXIT_CRITICAL_ISR(&spinlock);
}

/*
 * Auxiliary functions section
 * --------------------
 * Declaration of auxiliary functions
 */

void change_pathology()
{
    // change pathology on button click, from previous to next in sequential order
    if(xEventGroupGetBits(xPathologies) & BIT_(NORMAL_RHYTHM))
    {
        xEventGroupClearBits(xPathologies, BIT_(NORMAL_RHYTHM));
        xEventGroupSetBits(xPathologies, BIT_(SINUS_TACHYCARDIA));

        alpha[0] = 0;
        alpha[1] = -0.1;
        alpha[2] = 0;
        alpha[3] = 0;
        H = 2.848;
        gammat = 21;

        ESP_LOGI("change_pathology", "SINUS_TACHYCARDIA");
    } else if(xEventGroupGetBits(xPathologies) & BIT_(SINUS_TACHYCARDIA))
    {
        xEventGroupClearBits(xPathologies, BIT_(SINUS_TACHYCARDIA));
        xEventGroupSetBits(xPathologies, BIT_(ATRIAL_FLUTTER));

        alpha[0] = -0.068;
        alpha[1] = 0.028;
        alpha[2] = -0.024;
        alpha[3] = 0.12;
        H = 1.52;
        gammat = 13;

        ESP_LOGI("change_pathology", "ATRIAL_FLUTTER");
    } else if(xEventGroupGetBits(xPathologies) & BIT_(ATRIAL_FLUTTER))
    {   
        xEventGroupClearBits(xPathologies, BIT_(ATRIAL_FLUTTER));
        xEventGroupSetBits(xPathologies, BIT_(VENTRICULAR_TACHYCARDIA));

        alpha[0] = 0;
        alpha[1] = 0;
        alpha[2] = 0;
        alpha[3] = -0.1;
        H = 2.178;
        gammat = 21;

        ESP_LOGI("change_pathology", "VENTRICULAR_TACHYCARDIA");
    } else if(xEventGroupGetBits(xPathologies) & BIT_(VENTRICULAR_TACHYCARDIA))
    {
        xEventGroupClearBits(xPathologies, BIT_(VENTRICULAR_TACHYCARDIA));
        xEventGroupSetBits(xPathologies, BIT_(VENTRICULAR_FLUTTER));

        alpha[0] = 0.1;
        alpha[1] = -0.02;
        alpha[2] = -0.01;
        alpha[3] = 0;
        H = 2.178;
        gammat = 13;

        ESP_LOGI("change_pathology", "VENTRICULAR_FLUTTER");
    } else if(xEventGroupGetBits(xPathologies) & BIT_(VENTRICULAR_FLUTTER))
    {
        xEventGroupClearBits(xPathologies, BIT_(VENTRICULAR_FLUTTER));
        xEventGroupSetBits(xPathologies, BIT_(NORMAL_RHYTHM));

        alpha[0] = -0.024;
        alpha[1] = 0.0216;
        alpha[2] = -0.0012;
        alpha[3] = 0.12;
        H = 3;
        gammat = 7;

        ESP_LOGI("change_pathology", "NORMAL_RHYTHM");
    } else
    {
        xEventGroupSetBits(xPathologies, BIT_(NORMAL_RHYTHM));

        alpha[0] = -0.024;
        alpha[1] = 0.0216;
        alpha[2] = -0.0012;
        alpha[3] = 0.12;
        H = 3;
        gammat = 7;

        ESP_LOGI("change_pathology", "NORMAL_RHYTHM");
    }
    prev_x[0] = 0;
    prev_x[1] = 0;
    prev_x[2] = 0.1;
    prev_x[3] = 0;
}

float get_random(float scaleFactor)
{
    float float_random;
    uint32_t int_random = esp_random();

    float_random = ((float) int_random/(pow(2,32)-1)); // number between [0,1]
    float_random = 2*float_random-1; // number between [-1,1]
    float_random /= scaleFactor; // number scaled down

    return float_random; 
}
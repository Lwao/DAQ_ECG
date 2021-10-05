/**
 * @file main.c
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date October 4 2021
 */

#include "main.h"

/*
 * Main section
 * --------------------
 * Main function with general single time configuration in BOOT time
 */


void app_main(void)
{  
    BaseType_t xReturnedTask[2];

    // configure gpio pins
    ESP_ERROR_CHECK(gpio_config(&in_conf));                               // initialize input pin as pathology changer
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT));     // install gpio isr service
    ESP_ERROR_CHECK(gpio_isr_handler_add(BTN_CHANGE_PATHOLOGY, ISR_BTN, NULL));  // hook isr handler for specific gpio pin

    // create queue/semaphores/event groups
    xQueueData        = xQueueCreate(NUM_BUFFERS,BUFFER_LEN*sizeof(float)); // 32 bits = 4 bytes
    xEvents           = xEventGroupCreate();
    xPathologies      = xEventGroupCreate();
    
    if(xQueueData == NULL){ // tests if queue creation fails
        ESP_LOGE("app_main", "Failed to create data queue.\n");
        while(1);
    }
    if((xEvents == NULL) || (xPathologies == NULL)){ // tests if event group creation fails
        ESP_LOGE("app_main", "Failed to create event groups.\n");
        while(1);
    }

    // create tasks
    xReturnedTask[0] = xTaskCreatePinnedToCore(vTaskGenerateECG, "taskGEN", 2048, NULL, configMAX_PRIORITIES-1, &xTaskGENhandle, APP_CPU_NUM);
    xReturnedTask[1] = xTaskCreatePinnedToCore(vTaskOutputECG,   "taskOUT", 2048, NULL, configMAX_PRIORITIES-2, &xTaskOUThandle, PRO_CPU_NUM);
    
    for(int itr=0; itr<2; itr++) // iterate over tasks 
    {
        if(xReturnedTask[itr] == pdFAIL){ // tests if task creation fails
            ESP_LOGE("app_main", "Failed to create task %d.\n", itr);
            while(1);
        }
    }

    // set flag informing that the recording is stopped
    xEventGroupSetBits(xEvents, BIT_(ENABLE_ECG_GENERATION));

    ESP_LOGI("app_main", "Successful BOOT!");
    vTaskDelete(NULL);
}

/*
 * freeRTOS section
 * --------------------
 * Declaration of freeRTOS tasks
 */
   
void vTaskGenerateECG(void * pvParameters)
{
    int i;
    while(1)
    {
        if(xEventGroupWaitBits(xEvents, BIT_(ENABLE_ECG_GENERATION), pdFALSE, pdTRUE, portMAX_DELAY) & BIT_(ENABLE_ECG_GENERATION))
        {
            for(i=0; i<BUFFER_LEN; i++)
            {
                act_x[0] = gammat*(prev_x[0]-prev_x[1]-C*prev_x[0]*prev_x[1]-prev_x[0]*prev_x[1]*prev_x[1])*dt+prev_x[0];
                act_x[1] = gammat*(H*prev_x[0]-3*prev_x[1]+C*prev_x[0]*prev_x[1]+prev_x[0]*prev_x[1]*prev_x[1]+beta*(prev_x[3]-prev_x[1]))*dt+prev_x[1];
                act_x[2] = gammat*(prev_x[2]-prev_x[3]-C*prev_x[2]*prev_x[3]-prev_x[2]*prev_x[3]*prev_x[3])*dt+prev_x[2];
                act_x[3] = gammat*(H*prev_x[2]-3*prev_x[3]+C*prev_x[2]*prev_x[3]+prev_x[2]*prev_x[3]*prev_x[3]+2*beta*(prev_x[1]-prev_x[3]))*dt+prev_x[3];
                inBuffer[i] = alpha[0]*act_x[0]+alpha[1]*act_x[1]+alpha[2]*act_x[2]+alpha[3]*act_x[3];
                prev_x[0] = act_x[0];
                prev_x[1] = act_x[1];
                prev_x[2] = act_x[2];
                prev_x[3] = act_x[3];
            }

            // enqueue ecg data buffer
            xQueueSend(xQueueData,&inBuffer,portMAX_DELAY);
            vTaskDelay(1);
        } else{vTaskDelay(1);}
    }
}

void vTaskOutputECG(void * pvParameters)
{
    int i;
    int32_t time;
    while(1)
    {
        while(xQueueData!=NULL && xQueueReceive(xQueueData, &outBuffer, portMAX_DELAY)==pdTRUE) // wait for data to be read
        {
            
            for(i=0; i<BUFFER_LEN; i++)
            {
                time = esp_timer_get_time();
                printf("%f\n", outBuffer[i]);
                while((esp_timer_get_time()-time) < (int32_t)(1e6*dt));
            }
            vTaskDelay(1);
        }
        vTaskDelay(1);
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
    change_pathology();
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
    } else
    {
        xEventGroupSetBits(xPathologies, BIT_(NORMAL_RHYTHM));

        alpha[0] = -0.024;
        alpha[1] = 0.0216;
        alpha[2] = -0.0012;
        alpha[3] = 0.12;
        H = 3;
        gammat = 7;
    }
    prev_x[0] = 0;
    prev_x[1] = 0;
    prev_x[2] = 0.1;
    prev_x[3] = 0;
}

#include <stdio.h>
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



#define task_pin  27            //ADC 7
#define Duty_ratio_Pin1  32


void vTaskCode( void * pvParameters )
{
  
    gpio_reset_pin(task_pin );
    gpio_set_direction(task_pin , GPIO_MODE_OUTPUT  );
   

    while(1){
    gpio_set_level( task_pin ,1); 
    vTaskDelay(500/portTICK_PERIOD_MS);
    gpio_set_level( task_pin ,0);
    vTaskDelay(500/portTICK_PERIOD_MS);
    }
}





void vOtherFunction( void )
{
static uint8_t ucParameterToPass;
TaskHandle_t xHandle = NULL;

  xTaskCreate( vTaskCode, "NAME",8192, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle );
  configASSERT( xHandle );
  
}



void app_main(void)
{

    gpio_reset_pin(Duty_ratio_Pin1);
    gpio_set_direction(Duty_ratio_Pin1, GPIO_MODE_OUTPUT  );
    gpio_set_level( Duty_ratio_Pin1,1);

    vOtherFunction();

}
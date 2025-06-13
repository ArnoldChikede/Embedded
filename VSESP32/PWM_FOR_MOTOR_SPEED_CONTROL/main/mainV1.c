#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/adc_types.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_timer.h"
#include "driver/mcpwm_cmpr.h"
#include "driver/mcpwm_gen.h"



#define PP 18

#define POT_POWER_PIN 12             //ADC 6
#define VOT_IN   32           //ADC 4


#define task_pin  27       
#define Duty_ratio_Pin  35            //ADC 7
#define PWM_PIN 23
#define TEST_PIN 22

int raw_Pulse_Width_in7 =0;
int raw_Pulse_Width_in4 =0;

int Pulse_width_7 = 0;
int Pulse_width_4 = 0;

int duty_updated =10; //gobal varibale

int  Duty_Resolution  = 1000;

mcpwm_cmpr_handle_t ret_cmpr=NULL;

void vTaskCode( void * pvParameters )       //task decleratration with how its gonna perform
{
  
   
    gpio_reset_pin(PP );
    gpio_set_direction(PP, GPIO_MODE_OUTPUT  );
    gpio_set_level( PP ,1);

    gpio_reset_pin(task_pin );
    gpio_reset_pin(POT_POWER_PIN);
    gpio_reset_pin(Duty_ratio_Pin);
    gpio_set_direction(task_pin , GPIO_MODE_OUTPUT  );
    gpio_set_direction(POT_POWER_PIN , GPIO_MODE_OUTPUT  );
    gpio_set_direction(Duty_ratio_Pin, GPIO_MODE_INPUT  );
    gpio_set_level( task_pin ,1);
    gpio_set_level( POT_POWER_PIN ,1);
   

    adc_oneshot_unit_handle_t ADC7_handle;
    adc_oneshot_unit_init_cfg_t init_config7 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config7, &ADC7_handle));    //instance created and installed now we move to setting upt the settings for the channel remember for a unit you just change  mke the hanle once n use it on multiple c channels
    
    
    //configuring the channels
    
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,   //bitwidth isa simply how much time it takes to send one bit ...and baud rate=1/Tb is how many buts per second.
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(ADC7_handle, ADC_CHANNEL_7, &config));  //  now we configured our channel we move on to calibration to get value on mV
    ESP_ERROR_CHECK(adc_oneshot_config_channel(ADC7_handle, ADC_CHANNEL_4, &config)); 

     //setting calibtration scheme for VIN_PWE_handle
     adc_cali_handle_t cali_handle_Unit1 ;
     adc_cali_line_fitting_config_t cali_config_Unit1 = {
        .unit_id =  ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    
    //error checking and calling out different calibraqtion schemes for different channels
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config_Unit1, &cali_handle_Unit1));
    

    while(1){


    //vTaskDelay(1000/portTICK_PERIOD_MS);

    adc_oneshot_read(ADC7_handle,ADC_CHANNEL_7, &raw_Pulse_Width_in7);   //notice how i use the adrerss of and the pointer//....ref from top of code 
    adc_oneshot_read(ADC7_handle,ADC_CHANNEL_4, &raw_Pulse_Width_in4); 
    adc_cali_raw_to_voltage(cali_handle_Unit1, raw_Pulse_Width_in7 , &Pulse_width_7);
    adc_cali_raw_to_voltage(cali_handle_Unit1, raw_Pulse_Width_in4 , &Pulse_width_4);  //This is used to measure the maximum e


   float duty = ((float)Pulse_width_7/Pulse_width_4)* Duty_Resolution ;
   int duty_updated = (int)duty;
   int DCycle_percent = (int) (((float)Pulse_width_7/Pulse_width_4)*100);

    
    printf("Voltage is %d \n", Pulse_width_7 );
    //printf("Voltage SUPPLY is %d \n", Pulse_width_4 );
   // printf("duty_ratio %d \n", duty_updated );
    printf("duty_ratio %d%%\n", DCycle_percent );

    mcpwm_comparator_set_compare_value(ret_cmpr, duty_updated );  //duty updated is just the compare val
    
 


    vTaskDelay(250/portTICK_PERIOD_MS);

    }
}





void vOtherFunction( void )   //fucntion with the configuration and  parameters for the created task
{
static uint8_t ucParameterToPass;
TaskHandle_t xHandle = NULL;

  xTaskCreate( vTaskCode, "NAME",8192, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle );
  configASSERT( xHandle );
  
}





void app_main(void)

{

//CONFIGURATION STRUCTURES FOR MCPWM

//TIMERS configuration structure;  Resolution / period ticks = frequency 


int Resoulution_hz = 800000; //0 0 ;// 100000;            //should be less that the max frequency of the clck( How fast the timer counts )
int Period_ticks = Duty_Resolution ; //1000;       //50000;          //has to be below 65 535(The duty cycle resoluion is hidden here )
int Compare_value = Period_ticks/2 ;      //For setting the duty cyle
//float Duty_Ratio= ((float)Compare_value/Period_ticks)*100;



mcpwm_timer_config_t timer_config = {

.group_id = 0,  
.intr_priority=0,
.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,                       // MCPWM_TIMER_CLK_SRC_PLL160M

.resolution_hz= Resoulution_hz,               // Change later to a variableyou can change anywhere                      //BASICALLY SETTING HOWM MANY TIMER TICKS HAPPEN IN A SECOND  1000000
.count_mode= MCPWM_TIMER_COUNT_MODE_UP,
.period_ticks=Period_ticks // 1000,                                      //this sets the  length of each period

};


//Operator configuration structure

mcpwm_operator_config_t operator_config = {

 .group_id=0,
 .intr_priority=0,
 .flags= {

.update_dead_time_on_sync=0,
.update_dead_time_on_tep=0,
.update_dead_time_on_tez=0,
.update_gen_action_on_sync=0,
.update_gen_action_on_tep=0,
.update_gen_action_on_tez=0,
}

};

//Comparator structure

mcpwm_comparator_config_t comparator_config = {

.intr_priority=0,
.flags = {
    .update_cmp_on_sync=0,
    .update_cmp_on_tep=0,
    .update_cmp_on_tez=0,
}
};

//Generator configuration

mcpwm_generator_config_t generator_config ={

.gen_gpio_num = PWM_PIN,
.flags={
    .invert_pwm=0,
    .io_loop_back=0,
    .io_od_mode=0,
    .pull_down=0,
    .pull_up=0,
}
};


//ALLOCATING THE SUBMODULES EG TIMERS, COMPARATORS ANDF GENERTORS ETC TO MEMEORY WITHT TEH CONFIGURATION SETTINGS



mcpwm_timer_handle_t ret_timer=NULL;
 mcpwm_new_timer( &timer_config, &ret_timer);
 mcpwm_timer_enable(ret_timer);
 mcpwm_timer_start_stop(ret_timer, MCPWM_TIMER_START_NO_STOP );


 mcpwm_oper_handle_t ret_oper=NULL;
 mcpwm_new_operator(&operator_config, &ret_oper);
 mcpwm_operator_connect_timer(ret_oper, ret_timer );

 //mcpwm_cmpr_handle_t ret_cmpr=NULL;
 mcpwm_new_comparator(ret_oper, &comparator_config, &ret_cmpr);
 mcpwm_comparator_set_compare_value(ret_cmpr, Compare_value);  //Just set Duty Cycle to 50% (cmpr_val/period_ticks)*100  500



 mcpwm_gen_compare_event_action_t event_action_config={
.direction=MCPWM_TIMER_DIRECTION_UP,
.comparator=ret_cmpr,
.action=MCPWM_GEN_ACTION_LOW,

 };

 mcpwm_gen_timer_event_action_t  timer_action_config={

.direction=MCPWM_TIMER_DIRECTION_UP,
.event= MCPWM_TIMER_EVENT_EMPTY,
.action=MCPWM_GEN_ACTION_HIGH,

 };

 mcpwm_gen_handle_t ret_gen=NULL;
 mcpwm_new_generator(ret_oper, &generator_config, &ret_gen);
 mcpwm_generator_set_action_on_timer_event(ret_gen, timer_action_config);
 mcpwm_generator_set_action_on_compare_event(ret_gen,event_action_config );
 
    
   
 printf("Duty cycle is %d %%\n", duty_updated);  //just checking global variable 


 vOtherFunction(); // calling the  fuction adn bringing ti to life 




   


}
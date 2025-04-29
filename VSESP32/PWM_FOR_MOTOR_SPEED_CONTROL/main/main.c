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



#define VIN_PWER_PIN 34             //ADC 6
#define VPR_IN   32           //ADC 4

#define VPM_IN   35            //ADC 7

#define PWM_PIN 22

int raw_Pulse_Width_in6 =0;

int Pulse_width_V = 0;

void app_main(void)

{


    adc_oneshot_unit_handle_t ADC1_handle;
    adc_oneshot_unit_init_cfg_t init_config6 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config6, &ADC1_handle));    //instance created and installed now we move to setting upt the settings for the channel
    
    
    //configuring the channels
    
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,   //bitwidth isa simply how much time it takes to send one bit ...and baud rate=1/Tb is how many buts per second.
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(ADC1_handle, ADC_CHANNEL_6, &config));  //  now we configured our channel we move on to calibration to get value on mV


     //setting calibtration scheme for VIN_PWE_handle
     adc_cali_handle_t cali_handle_Unit1 ;
     adc_cali_line_fitting_config_t cali_config_Unit1 = {
        .unit_id =  ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    
    
    //error checking and calling out different calibraqtion schemes for different channels
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config_Unit1, &cali_handle_Unit1));
    
    
//SETTINGS FOR PWM

//TIMERS configuration structure

mcpwm_timer_config_t timer_config = {

.group_id = 0,
.intr_priority=0,
.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
.resolution_hz=1000000, 
.count_mode= MCPWM_TIMER_COUNT_MODE_UP,
.period_ticks= 1000,  //this sets the  length of each period

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

 mcpwm_cmpr_handle_t ret_cmpr=NULL;
 mcpwm_new_comparator(ret_oper, &comparator_config, &ret_cmpr);
 mcpwm_comparator_set_compare_value(ret_cmpr, 500);  //Just set Duty Cycle to 50% (cmpr_val/period_ticks)*100



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
 
    
    while (1){
    
    








    //adc_oneshot_read(ADC1_handle,ADC_CHANNEL_6, &raw_Pulse_Width_in6);   //notice how i use the adrerss of and the pointer//....ref from top of code 
   // adc_cali_raw_to_voltage(cali_handle_Unit1, raw_Pulse_Width_in6 , &Pulse_width_V);
    
    
    












    }








}
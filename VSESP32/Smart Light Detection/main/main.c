#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/adc_types.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"


#define LEDPIN 23
#define PWER_PIN 13
#define POT_PWER_PIN 12
#define VIN_PWER_PIN 34             //ADC 6
#define VPR_IN   32           //ADC 4

#define VPM_IN   35            //ADC 7


int Voltage_in6 = 0 ;
int Voltage_in4 = 0 ;
int Voltage_in7 = 0 ;
//int *ptr_Voltage_in = &Voltage_in ; 

int raw_Voltage_in6 = 0;
int raw_Voltage_in4 = 0;
int raw_Voltage_in7=0;


//2.2K RESISTOR USED IN EXPERIMENT..


void app_main(void)
{


gpio_reset_pin(LEDPIN);
gpio_reset_pin(PWER_PIN);
gpio_reset_pin(VIN_PWER_PIN);
gpio_reset_pin(VPR_IN);
gpio_reset_pin(VPM_IN);
gpio_reset_pin(POT_PWER_PIN);


gpio_set_direction(PWER_PIN, GPIO_MODE_OUTPUT);
gpio_set_direction(LEDPIN, GPIO_MODE_OUTPUT);
gpio_set_direction(POT_PWER_PIN, GPIO_MODE_OUTPUT);
gpio_set_direction(VIN_PWER_PIN, GPIO_MODE_INPUT); //initialising the gpio for Input Voltage measure
gpio_set_direction(VPM_IN, GPIO_MODE_INPUT); //initialising the gpio for potenitiometer measure
gpio_set_direction(VPR_IN, GPIO_MODE_INPUT); //initialising the gpio for photoresistor measure
gpio_set_level(PWER_PIN, 1);
gpio_set_level(POT_PWER_PIN, 1);


//now need to measure the voltage produced by the power pin at any given moment to make it the  max so we need to use ADC.

//CREATION OF A HANDLE(You can only create one handle for for 1 ADC handle not different ones coz all of them will try to own the ADC and conflict of resources arises)...A handle is like a pointer or refrence that thells the hardware that you wanna use the resources.


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

//however we use the same setting so no need to write the code again 
ESP_ERROR_CHECK(adc_oneshot_config_channel(ADC1_handle, ADC_CHANNEL_7, &config));  //  now we configured our channel we move on to calibration to get value on mV

ESP_ERROR_CHECK(adc_oneshot_config_channel(ADC1_handle, ADC_CHANNEL_4, &config));


 //& in &config used to get adress ofa variable (	Ask: "Where is this thing?")  and * is a dereference -Gets the value at an address
//The pointer gives you the address (&x), but if you want to get or modify, that’s when we dereference it (*p)”


 //setting calibtration scheme for VIN_PWE_handle
 adc_cali_handle_t cali_handle_Unit1 ;
 adc_cali_line_fitting_config_t cali_config_Unit1 = {
    .unit_id =  ADC_UNIT_1,
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_DEFAULT,
};


//error checking and calling out different calibraqtion schemes for different channels
ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config_Unit1, &cali_handle_Unit1));



while (1){


adc_oneshot_read(ADC1_handle,ADC_CHANNEL_6, &raw_Voltage_in6);   //notice how i use the adrerss of and the pointer//....ref from top of code 
adc_cali_raw_to_voltage(cali_handle_Unit1, raw_Voltage_in6 , &Voltage_in6);


adc_oneshot_read(ADC1_handle,ADC_CHANNEL_7, &raw_Voltage_in7);   
adc_cali_raw_to_voltage(cali_handle_Unit1, raw_Voltage_in7 , &Voltage_in7);

adc_oneshot_read(ADC1_handle,ADC_CHANNEL_4, &raw_Voltage_in4);   
adc_cali_raw_to_voltage(cali_handle_Unit1, raw_Voltage_in4 , &Voltage_in4);


//setting Potenbtiometer sensitivity

float  Pot_sense=(raw_Voltage_in7/4095.0)*10;
float  LDR_sense=(raw_Voltage_in4/4095.0)*10;
//float Light_Intensity_thresh=6.0;
float Threshold = Pot_sense;

//will basically need to reconfigure this by jst checking the vin calue of threshold you get from them LDR and thts for light conditions since it reduces resistance and then later configure the system to work the 
//way you want 

if( LDR_sense> Threshold){
    gpio_set_level(LEDPIN, 1);
}
else{
gpio_set_level(LEDPIN, 0);      
}


float voltage_in4_f = Voltage_in4 / 1000.0;
float voltage_in6_f = Voltage_in6 / 1000.0;
float voltage_in7_f = Voltage_in7 / 1000.0;

ESP_LOGI("Voltage_V","Input Voltage is %0.2f \n",voltage_in6_f );
ESP_LOGI("Voltage_V","Potentiometer Voltage is %0.2f \n",voltage_in7_f );
ESP_LOGI("Voltage_V","Photoresitor Voltage is %0.2f \n",voltage_in4_f);


printf("Sensitivity Threshold From Potenitometer is %0.2f \n",Pot_sense);
printf("LDR Threshold Value %0.2f \n",LDR_sense);
//printf("raw %d \n",raw_Voltage_in7);

vTaskDelay(2000/ portTICK_PERIOD_MS);    //Recall thatvTaskDelay takes ticks not ms so we need to convert  the   the ms to ticks 
                                                 //by deviding by   portTICK_PERIOD_MS wch is a macro suggesting that 1 tick=10 ms so jst proportion


  }

}



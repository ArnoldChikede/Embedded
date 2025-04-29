#include <stdio.h>
#include "dht.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/adc_types.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"



#define TEMP_PIN_DHT 23
#define LMD35_PWER  25
#define LMD35_VOUT  32


float Vout;
float Vmax=3300.0;    //max value in millivots
float Dmax = 4095.0;
int Dout;
int16_t temperature = 0;
int16_t humidity = 0;
int result=0;
int c_Volt=0;
int c_raw=0;
int temperatures[1][3];   //use this to group the temperaatures together!!





void app_main(void)
{

 int num;
  adc_oneshot_channel_to_io(ADC_UNIT_1, 4 , &num);
  printf("the gpionum is %d", num);
 
  gpio_reset_pin(LMD35_PWER);                          //Just resetting pins from previous states and then intialising them with new states 
  gpio_reset_pin(LMD35_VOUT);
  gpio_reset_pin(TEMP_PIN_DHT);

  
  gpio_set_direction(LMD35_VOUT, GPIO_MODE_DEF_INPUT);
  gpio_set_direction(TEMP_PIN_DHT, GPIO_MODE_INPUT_OUTPUT_OD);


 
  

  adc_oneshot_unit_handle_t adc1_handle;                           //Initialising the adc handle 
  adc_oneshot_unit_init_cfg_t init_config1 = {                    // creating the instance struct  for the configuration settings 
    .unit_id = ADC_UNIT_1,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
};

 
ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));  // func  is passed adrresses as inputs and it goes to location to find the information  // the error check here is more automatic , if anything fails , the system aborts ryt away


 /*esp_err_t ret = adc_oneshot_new_unit(&init_config1, &adc1_handle);

if(ret == ESP_OK) 

{
  ESP_LOGI("ADC_CHANNEL_4", "Config of channel is succeful");

}
else{
  ESP_LOGE("ADC_CHANNEL_4","Configuringof channel is unscucceful, %s:",  esp_err_to_name(ret));
} */                    //RYT HERE WE FOCUS ON A MANUAL WAY OF CHECKING THE adc_one_shot and we use this say when we want to log error messages and stuff like that 
                        //as we see with the else part .


  adc_oneshot_chan_cfg_t config = {                         //configis our instance of the struct..recall the use of typdef so that we have the name config as a nickname
    .bitwidth =  ADC_BITWIDTH_DEFAULT,
    .atten = ADC_ATTEN_DB_12,
};

ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &config));

//BUILDING CALIBRATION NOW
adc_cali_scheme_ver_t scheme;       //  scheme initialised with a specific data type (enum)

adc_cali_check_scheme(&scheme);     //checking for the scheme type and print it out

//now we use the swictch case to check which case it is
switch(scheme){

case ADC_CALI_SCHEME_VER_LINE_FITTING:
printf("ADC_CALI_SCHEME_VER_LINE_FITTING\n");
break;

case  ADC_CALI_SCHEME_VER_CURVE_FITTING:
printf("ADC_CALI_SCHEME_VER_CURVE_FITTING\n");
break;                            //Beuty of break here coz you let the programme know that when it finds a case should go out and not keep searching 

default:
printf("Unkown case!!\n");
break;


}


// Calibration process now..
adc_cali_handle_t cali_handle ;  //using the typedef pointer to initialise the handle for calibration
adc_cali_line_fitting_config_t cali_config = {
  .unit_id = ADC_UNIT_1,
  .atten = ADC_ATTEN_DB_12,
  .bitwidth = ADC_BITWIDTH_DEFAULT,
};
ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle));

while (1) {

  ESP_LOGW("TAG", "NEW OF SESSION");

adc_oneshot_read(adc1_handle,  ADC_CHANNEL_4 , &Dout);

dht_read_data( DHT_TYPE_DHT11 , TEMP_PIN_DHT, &humidity , &temperature);

Vout = Dout * Vmax / Dmax    ;                     //calaulation of the output voltage in mv
//float Temp = (Vout * 0.1 ) ;

adc_cali_raw_to_voltage(cali_handle, Dout, &c_Volt);   //READING The voltage for calib part
float Calib_Temp = (c_Volt * 0.1 ) ; 


    if (result == ESP_OK) {
      //  ESP_LOGI("TEMP","Temperature from DHT: %d°C\n", temperature / 10);  // Convert fixed-point format
        //ESP_LOGI("HUMIDITY","Humidity from DHT: %d%%\n", humidity / 10);  // 
    } 
    else {
        printf("Failed to read from DHT sensor! Error code: %d\n", result);
    }

    ESP_LOGI(" DHT11 Voltage","%d °C",temperature / 10); //Just Loggin to view messeged better than using printf
    ESP_LOGI("Calibrated Voltage","%f C",Calib_Temp);
    ESP_LOGI("Humidity","%d%% ",humidity / 10);



   /*  usign a 2D array to put temperature values.
     temperatures[0][0]=temperature / 10 ;
    temperatures[0][1]=Calib_Temp;
    temperatures[0][2]=Temp;

    for(  int row=0; row<1; row++){

        for(   int column=0;column<3; column++){


         printf(" %d", temperatures[row][column]);
         }

        }*/

      ESP_LOGE("TAG", "END OF SESSION");
    vTaskDelay(5000/ portTICK_PERIOD_MS);   } //Recall thatvTaskDelay takes ticks not ms so we need to convert  the   the ms to ticks 
                                                 //by deviding by   portTICK_PERIOD_MS wch is a macro suggesting that 1 tick=10 ms so jst proportion


  }






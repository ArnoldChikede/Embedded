idf_component_register(SRCS "mainV1.c" "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_gpio  esp_adc 
                    REQUIRES esp_driver_mcpwm )
#include <stdio.h>

void app_main(void)
{
    gpio_dump_io_configuration(stdout, (1ULL << 4) );
}
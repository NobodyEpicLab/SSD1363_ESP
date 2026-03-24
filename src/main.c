#include <stdio.h>

#include "ssd1363_demo.h"

void app_main(void)
{
	printf("SSD1363 application start\n");
	ssd1363_demo_run_i2c_smoke_test();
}
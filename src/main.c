#include <stdio.h>

#include "ssd1363_demo.h"
#include "ssd1363_font_presentation.h"

void app_main(void)
{
	printf("SSD1363 application start\n");
	ssd1363_font_presentation_run_i2c();
}
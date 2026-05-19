#include "run_task.h"
#include "WS2812.h"
#include "freertos.h"
#include "task.h"
#include "cmsis_os.h"

static uint8_t red = 0;
static uint8_t green = 0;
static uint8_t blue = 0;
static uint16_t current_time = 0;


void run_task(void const * argument)
{

	while(1)
	{
    osDelay(1);
		current_time++;
		if (current_time >= 175)
    {
			if(red > 16){red -= 16;}
			if(green > 20){green -= 18;}
			if(blue > 24){blue -= 14;}
			WS_Write(red ,green,blue,1,1);
			red++;
			green += 2;
			blue += 4;
			current_time = 0;
		}
		osDelay(1);
	}
}


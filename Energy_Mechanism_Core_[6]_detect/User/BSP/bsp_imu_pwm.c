#include "bsp_imu_pwm.h"
#include "main.h"

extern TIM_HandleTypeDef htim1;
void imu_pwm_set(uint16_t pwm)
{
    __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, pwm);
}

#include "FreeRTOS.h"
#include "task.h"
#include "Motor_task.h"
#include "M15_task.h"      
#include "CAN_receive.h"   
#include "PID.h"

void move_init(move_t *move_init);
void feedback_update(move_t *move_update);
//static void control_loop(move_t *move_control_loop);
fp32 fp32_constrain(fp32 Value, fp32 minValue, fp32 maxValue);

//move_t chassis_move;
const fp32 rotation_speed_pid[3] = {ROTATION_MOTOR_SPEED_PID_KP, ROTATION_MOTOR_SPEED_PID_KI, ROTATION_MOTOR_SPEED_PID_KD};

void Motor_task(void const * argument)
{
    move_init(&chassis_move);
    while(1)
    {
        //更新电机反馈数据
        feedback_update(&chassis_move);   
        
        //计算 PID 
        //control_loop(&chassis_move);
        
        //连续发送电机控制指令和模式
        CAN_cmd_chassis_set_feedback(0x01);	
        CAN_cmd_chassis_set_mode(MODE_2, MODE_2);
        CAN_cmd_chassis(chassis_move.wz_set);
        vTaskDelay(1);
    }
}

/**
  * @brief          初始化
  * @param[in]      none
  * @retval         none
  */
void move_init(move_t *move_init)
{
    move_init->rotation_motor_measure.motor_measure = get_chassis_motor_1_measure_point();
    PID_init(&move_init->rotation_motor_pid, PID_POSITION, rotation_speed_pid, ROTATION_MOTOR_SPEED_PID_MAX_OUT, ROTATION_MOTOR_SPEED_PID_MAX_IOUT);

    move_init->wz_max_speed = NORMAL_MAX_ROTATION_SPEED_Z;
    move_init->wz_min_speed = -NORMAL_MAX_ROTATION_SPEED_Z;
}

void feedback_update(move_t *move_update)
{

	//云台
	move_update->rotation_motor_measure.speed = move_update->rotation_motor_measure.motor_measure->rpm;

  //更新速度
	move_update->wz = move_update->rotation_motor_measure.speed / 100 / 60 * 2;
}

// static void control_loop(move_t *move_control_loop)
// {
// 	PID_calc(&move_control_loop->rotation_motor_pid, move_control_loop->rotation_motor_measure.speed, move_control_loop->rotation_motor_measure.speed_set);
// 	move_control_loop->rotation_motor_measure.give_current = (int16_t)(move_control_loop->rotation_motor_pid.out);
// }

fp32 fp32_constrain(fp32 Value, fp32 minValue, fp32 maxValue)
{
	if(Value < minValue){
		return minValue;
	}
    else if(Value > maxValue){
        return maxValue;
	}
    else{
        return Value;
	}
}


#ifndef _M15_TASK_H
#define _M15_TASK_H

#include "struct_typedef.h"
#include "CAN_receive.h"
#include "pid.h"
#include "remote_control.h"

#define PI 3.1415926535798932384626

#define MODE_0 0X00  //电压开环
#define MODE_1 0x01  //电流环
#define MODE_2 0x02  //速度环

#define RPM_Second 12000
#define SPEED 1.5
#define NO 0
#define YES 1
#define OFF 0
#define ON 1

//拨杆通道号
#define RIGHT_LEVER 0
#define LEFT_LEVER 1

//摇杆死区
#define CHASSIS_RC_DEADLINE 50

//底盘M15最大can发送电流值
#define MAX_MOTOR_M15_CAN_CURRENT 32767.0f

//M15 rpm转化成底盘速度(m/s)的比例，  
#define M15_MOTOR_RPM_TO_VECTOR 0.0112573736753
#define CHASSIS_MOTOR_RPM_TO_VECTOR_SEN M15_MOTOR_RPM_TO_VECTOR

//M15 rpm转电流 比例，  
#define M15_MOTOR_RPM_TO_CURRENT 156
#define CHASSIS_MOTOR_RPM_TO_CURRENT_SEN M15_MOTOR_RPM_TO_CURRENT

//M15 速度转电流 比例，  
#define M15_MOTOR_VECTOR_TO_CURRENT 13860.544904474

#define CHASSIS_MOTOR_VECTOR_TO_CURRENT_SEN M15_MOTOR_VECTOR_TO_CURRENT

#define NORMAL_MAX_ROTATION_SPEED_Z 1000.0f

//云台旋转速度环PID
#define ROTATION_MOTOR_SPEED_PID_KP 114514.0f
#define ROTATION_MOTOR_SPEED_PID_KI 1.14514f
#define ROTATION_MOTOR_SPEED_PID_KD 0.5f
#define ROTATION_MOTOR_SPEED_PID_MAX_OUT 1000.0f
#define ROTATION_MOTOR_SPEED_PID_MAX_IOUT 1000.0f

typedef struct
{
	double a;
	double w;
	double b;
}Number;

typedef enum
{
	CHASSIS_NO_MOVE,
	HIGH_MOVE,
	MIDDLE_MOVE,
	LOW_MOVE,
	
	HIGH_ROTATION,
	MIDDLE_ROTATION,
	LOW_ROTATION,
} mode_e;

typedef struct
{
	const motor_measure_t *motor_measure;
	int16_t rpm;
  fp32 speed;
  fp32 speed_set;
	int16_t give_current;  //发送的值
	int16_t current;       //返回的电流值
	
} motor_t;

typedef struct
{

  mode_e mode;                      //云台控制状态机
  motor_t rotation_motor_measure;             //云台电机数据
  pid_type_def rotation_motor_pid;                    //云台旋转电机pid
  fp32 wz;                           //底盘旋转角速度，逆时针为正 单位 转/s
  fp32 wz_set;                       //设定旋转速度，逆时针为正 单位 转/s
  fp32 wz_max_speed;  //旋转最大速度 单位m/s
  fp32 wz_min_speed;  //旋转最小速度 单位m/s
  motor_measure_t M15;
} move_t;

extern move_t move;

/**
  * @brief          底盘任务，间隔 2ms
  * @param[in]      pvParameters: 空
  * @retval         none
  */
extern void M15_task(void const *pvParameters);

/**
  * @brief          根据遥控器通道值，计算纵向和横移速度
  * @param[out]     chassis_move_rc_to_vector: "chassis_move" 变量指针
  * @retval         none
  */
extern void rc_to_control_vector(fp32 *vx_set, fp32 *vy_set, move_t *move_rc_to_vector);

typedef enum
{
	small_red = 0,
	small_blue = 1,
	big_red = 2,
	big_blue = 3,
	no = 4,
	fail = 6,
	ce_shi = 7,
	qing_chu = 9
} behaviour_e;


/**
  * @brief          通过逻辑判断，赋值"behaviour_mode"成哪种模式
  * @param[in]      chassis_move_mode: 底盘数据
  * @retval         none
  */
extern void behaviour_mode_set(move_t *move_mode);

/**
  * @brief          设置控制量.根据不同底盘控制模式，三个参数会控制不同运动.在这个函数里面，会调用不同的控制函数.
  * @param[out]     wz_set, 通常控制旋转运动.
  * @param[in]      chassis_move_rc_to_vector,  包括底盘所有信息.
  * @retval         none
  */

extern void behaviour_control_set(fp32 *vx_set, fp32 *vy_set, fp32 *angle_set, move_t *move_rc_to_vector);


#endif

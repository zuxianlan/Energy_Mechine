#ifndef CAN_RECEIVE_H
#define CAN_RECEIVE_H

#include "struct_typedef.h"
#include "main.h"
#include "remote_control.h"

#define CHASSIS_CAN hcan1
#define GIMBAL_CAN hcan2

/* CAN send and receive ID */
typedef enum
{
	CAN_CHASSIS_ALL_ID = 0x32,
  M15_1_ID = 0x97,
	board_1 = 0x101,
	board_2 = 0x102,
	board_3 = 0x103,
	board_4 = 0x104,
	board_5 = 0x105,
} can_msg_id_e;

//rm motor data
typedef struct
{	
  int16_t rpm;            //速度测量值
  int16_t given_current;  //转矩电流值
	int16_t position_value; //位置值
	int16_t fault_value;	  //故障值
	int16_t current_mode;	  //当前模式
	int16_t mode;           //设定的模式
	int16_t feedback_mode;  //电机反馈模式
} motor_measure_t;

//F4接收
typedef struct
{
	uint8_t  	Color;                 //大符颜色
	uint8_t  	Last_Color;            //大符上次颜色
	uint8_t   Hit_LED_state;         //(击中/未击中)LED状态
	uint8_t   Single_Hit_State;      //单个扇叶（击中/未击中）状态
	uint8_t   num_of_hit_ring;       //击中环数
	uint8_t   All_Hit_State;         //全部扇叶（击中/未击中）状态
	uint8_t   err_hit;               //错误击打
	uint8_t   Twinkle_Already;       //闪烁
	uint8_t   Led_proportion;        //灯臂亮起占比
} board;

//发送给F1
typedef struct 
{
	uint8_t    Twinkle_state;
	uint8_t    All_Led_Close;
	uint8_t    qingchu;
	uint8_t    Board_Work;
	uint8_t    Set_Color;
	uint8_t    start_permission;
	uint8_t    RainBow_Flag;
	uint8_t    Led_Proportion;
}Board_Order_info_t;

/**
  * @brief          发送电机控制电流
  * @param[in]      motor_M15:电机控制电流, 范围 [-32767,32767]
  * @retval         none
  */
void CAN_cmd_chassis(int16_t motor_M15);

/**
  * @brief          设置M15电机模式
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_chassis_set_mode(int16_t motor_1_mode, int16_t motor_2_mode);

/**
  * @brief          设置M15电机反馈模式
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_chassis_set_feedback(int16_t feedback);

/**
  * @brief          返回底盘M15电机数据指针
  * @param[in]      none
  * @retval         电机数据指针
  */
extern const motor_measure_t *get_chassis_motor_1_measure_point(void);
extern board board_mode[6];

/**
  * @brief          控制灯板状态
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_board_mode(int16_t sequence, int16_t twinkie_state, int16_t board_work);

/**
  * @brief          控制灯板颜色
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_board_color(int16_t color);

/**
  * @brief          控制灯臂进度
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_lamp_arm_progress(int16_t progress);


/**
  * @brief          控制灯板开关
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_switch(int16_t Switch);

/**
  * @brief          清除累加
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_qing_chu(int16_t qingchu);

/**
  * @brief          全关
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_close(int16_t close);

/**
  * @brief          边框全亮
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_frame(int16_t frame);

#endif

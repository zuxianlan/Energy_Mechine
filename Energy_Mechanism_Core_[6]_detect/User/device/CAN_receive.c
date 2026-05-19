#include "CAN_receive.h"

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

//rpm速度测量值   give_current转矩电流值   position_value位置值   fault_value故障值   current_mode当前模式
#define get_motor_measure_M15(ptr, data)                                \
    {                                                                   \
        (ptr)->rpm = ((data)[0] << 8 | (data)[1]);                      \
        (ptr)->given_current = (int16_t)((data)[2] << 8 | (data)[3]);   \
        (ptr)->position_value = (uint16_t)((data)[4] << 8 | (data)[5]); \
        (ptr)->fault_value = (data)[6]; 		                            \
				(ptr)->current_mode = (data)[7];                                \
    }
	
#define get_motor_mode_M15(ptr, data)            \
    {                                            \
        (ptr)->mode = (data)[0]; 	               \
    }                    

#define get_motor_feedback_mode_M15(ptr, data)   \
    {                                       		 \
        (ptr)->feedback_mode = (data)[0]; 	     \
    } 	

#define get_DF_measure(ptr,data)                 \
		{                                            \
			(ptr)->Color = (data)[0];                  \
			(ptr)->Last_Color = (data)[1];             \
			(ptr)->Hit_LED_state = (data)[2];          \
			(ptr)->Single_Hit_State = (data)[3];       \
			(ptr)->num_of_hit_ring = (data)[4];   \
			(ptr)->All_Hit_State = (data)[5];          \
			(ptr)->err_hit = ((data)[6] == 1) ? 1 : 0; \
			(ptr)->Twinkle_Already = (data)[7];        \
		}                                            
	

motor_measure_t motor_chassis[10];
board board_mode[6];

uint8_t send_flag = 0;
uint8_t send_flag_mode = 0;
uint8_t send_flag_color = 0;
uint8_t send_flag_progress = 0;		
		
static CAN_TxHeaderTypeDef  gimbal_tx_message;
static uint8_t              gimbal_can_send_data[8];
static CAN_TxHeaderTypeDef  chassis_tx_message;
static uint8_t              chassis_can_send_data[8];

/**
  * @brief          hal库CAN回调函数,接收电机数据
  * @param[in]      hcan:CAN句柄指针
  * @retval         none
  */
uint8_t rx_data[8];
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{

    CAN_RxHeaderTypeDef rx_header;
  
	if(hcan==&hcan2)
		{
			uint8_t rx_data_2[8];
			HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data_2);
			switch (rx_header.StdId)
				{
					
					case board_1:
					{
						get_DF_measure(&board_mode[1],rx_data_2);
						break;
					}
					
					case board_2:
					{
						get_DF_measure(&board_mode[2],rx_data_2);
						break;
					}
					
					case board_3:
					{
						get_DF_measure(&board_mode[3],rx_data_2);
						break;
					}
					
					case board_4:
					{
						get_DF_measure(&board_mode[4],rx_data_2);
						break;
					}
					
					case board_5:
					{
						get_DF_measure(&board_mode[5],rx_data_2);
						break;
					}

					default:
						{
							break;
						}
				}
		}
	else if(hcan==&hcan1)
		{
			uint8_t rx_data_1[8];
			HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data_1);
			switch (rx_header.StdId)
				{
				case M15_1_ID:
					{
						get_motor_measure_M15(&motor_chassis[1],rx_data_1);
						break;
					}
				case 0x201:					
					{
						get_motor_mode_M15(&motor_chassis[1],rx_data_1);
						break;
					}
				case 0x265:
					{
						get_motor_feedback_mode_M15(&motor_chassis[1],rx_data_1);
						break;
					}	
				default:
					{
						break;
					}
				}
		}
}

/**
  * @brief          发送电机控制电流
  * @param[in]      motor_M15:电机控制电流, 范围 [-32767,32767]
  * @retval         none
  */
void CAN_cmd_chassis(int16_t motor_M15)
{
    uint32_t send_mail_box;
    chassis_tx_message.StdId = CAN_CHASSIS_ALL_ID;
    chassis_tx_message.IDE = CAN_ID_STD;
    chassis_tx_message.RTR = CAN_RTR_DATA;
    chassis_tx_message.DLC = 0x08;
    chassis_can_send_data[0] = (motor_M15 >> 8);
    chassis_can_send_data[1] = motor_M15;

    HAL_CAN_AddTxMessage(&CHASSIS_CAN, &chassis_tx_message, chassis_can_send_data, &send_mail_box);
}

/**
  * @brief          设置M15电机模式
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_chassis_set_mode(int16_t motor_1_mode, int16_t motor_2_mode)
{
    uint32_t send_mail_box;
    chassis_tx_message.StdId = 0x105;
    chassis_tx_message.IDE = CAN_ID_STD;
    chassis_tx_message.RTR = CAN_RTR_DATA;
    chassis_tx_message.DLC = 0x08;
    chassis_can_send_data[0] = motor_1_mode;
    HAL_CAN_AddTxMessage(&CHASSIS_CAN, &chassis_tx_message, chassis_can_send_data, &send_mail_box);
}

/**
  * @brief          设置M15电机反馈模式
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_chassis_set_feedback(int16_t feedback)
{
    uint32_t send_mail_box;
    chassis_tx_message.StdId = 0x106;
    chassis_tx_message.IDE = CAN_ID_STD;
    chassis_tx_message.RTR = CAN_RTR_DATA;
    chassis_tx_message.DLC = 0x08;
    chassis_can_send_data[0] = feedback;
    HAL_CAN_AddTxMessage(&CHASSIS_CAN, &chassis_tx_message, chassis_can_send_data, &send_mail_box);
}


/**
  * @brief          返回底盘M15电机数据指针
  * @param[in]      none
  * @retval         电机数据指针
  */
const motor_measure_t *get_chassis_motor_1_measure_point(void)
{
    return &motor_chassis[1];
}

/**
  * @brief          控制灯板状态
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_board_mode(int16_t sequence, int16_t twinkie_state, int16_t board_work)
{
	uint32_t send_mail_box;
	gimbal_tx_message.IDE = CAN_ID_STD;
	gimbal_tx_message.RTR = CAN_RTR_DATA;	
	gimbal_tx_message.DLC = 0x08;	
	gimbal_can_send_data[0] = twinkie_state;
	gimbal_can_send_data[1] = board_work;
	if(sequence == 1)
	{
		gimbal_tx_message.StdId = 0x301;
	}
	else if(sequence == 2)
	{
		gimbal_tx_message.StdId = 0x302;
	}
	else if(sequence == 3)
	{
		gimbal_tx_message.StdId = 0x303;
	}
	else if(sequence == 4)
	{
		gimbal_tx_message.StdId = 0x304;
	}
	else if(sequence == 5)
	{
		gimbal_tx_message.StdId = 0x305;
	}
	send_flag_mode = HAL_CAN_AddTxMessage(&GIMBAL_CAN, &gimbal_tx_message, gimbal_can_send_data, &send_mail_box);
}

/**
  * @brief          控制灯板颜色
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_board_color(int16_t color)
{
	uint32_t send_mail_box;
	gimbal_tx_message.StdId = 0x300;
	gimbal_tx_message.IDE = CAN_ID_STD;
	gimbal_tx_message.RTR = CAN_RTR_DATA;	
	gimbal_tx_message.DLC = 0x08;	
	gimbal_can_send_data[0] = color;
	send_flag_color = HAL_CAN_AddTxMessage(&GIMBAL_CAN, &gimbal_tx_message, gimbal_can_send_data, &send_mail_box);
}

/**
  * @brief          控制灯臂进度
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_lamp_arm_progress(int16_t progress)
{
	uint32_t send_mail_box;
	gimbal_tx_message.StdId = 0x299;
	gimbal_tx_message.IDE = CAN_ID_STD;
	gimbal_tx_message.RTR = CAN_RTR_DATA;	
	gimbal_tx_message.DLC = 0x08;	
	gimbal_can_send_data[0] = progress;
	send_flag_progress = HAL_CAN_AddTxMessage(&GIMBAL_CAN, &gimbal_tx_message, gimbal_can_send_data, &send_mail_box);
}

/**
  * @brief          控制错`误击打开关
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_switch(int16_t Switch)
{
	uint32_t send_mail_box;
	gimbal_tx_message.StdId = 0x298;
	gimbal_tx_message.IDE = CAN_ID_STD;
	gimbal_tx_message.RTR = CAN_RTR_DATA;	
	gimbal_tx_message.DLC = 0x08;	
	gimbal_can_send_data[0] = Switch;
	send_flag = HAL_CAN_AddTxMessage(&GIMBAL_CAN, &gimbal_tx_message, gimbal_can_send_data, &send_mail_box);
}


/**
  * @brief          清除累加
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_qing_chu(int16_t qingchu)
{
	uint32_t send_mail_box;
	gimbal_tx_message.StdId = 0x297;
	gimbal_tx_message.IDE = CAN_ID_STD;
	gimbal_tx_message.RTR = CAN_RTR_DATA;	
	gimbal_tx_message.DLC = 0x08;	
	gimbal_can_send_data[0] = qingchu;
	send_flag = HAL_CAN_AddTxMessage(&GIMBAL_CAN, &gimbal_tx_message, gimbal_can_send_data, &send_mail_box);
}

/**
  * @brief          全关
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_close(int16_t close)
{
	uint32_t send_mail_box;
	gimbal_tx_message.StdId = 0x296;
	gimbal_tx_message.IDE = CAN_ID_STD;
	gimbal_tx_message.RTR = CAN_RTR_DATA;	
	gimbal_tx_message.DLC = 0x08;	
	gimbal_can_send_data[0] = close;
	send_flag = HAL_CAN_AddTxMessage(&GIMBAL_CAN, &gimbal_tx_message, gimbal_can_send_data, &send_mail_box);
}

/**
  * @brief          边框全亮
  * @param[in]      none
  * @retval         none
  */
void CAN_cmd_frame(int16_t frame)
{
	uint32_t send_mail_box;
	gimbal_tx_message.StdId = 0x295;
	gimbal_tx_message.IDE = CAN_ID_STD;
	gimbal_tx_message.RTR = CAN_RTR_DATA;	
	gimbal_tx_message.DLC = 0x08;	
	gimbal_can_send_data[0] = frame;
	send_flag = HAL_CAN_AddTxMessage(&GIMBAL_CAN, &gimbal_tx_message, gimbal_can_send_data, &send_mail_box);
}

#include "bluetooth_receive.h"
#include "stm32f4xx_hal.h"
#include "M15_task.h"
#include "cmsis_os.h"
#include "bsp_can.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "PID.h"

fp32 fp32_constrain(fp32 Value, fp32 minValue, fp32 maxValue);
void move_init(move_t *chassis_move_init);
static void set_mode(move_t *chassis_move_mode);
static void feedback_update(move_t *chassis_move_update);
static void set_contorl(move_t *chassis_move_control);
static void control_loop(move_t *chassis_move_control_loop);
void Random_Number(void);
void Get_Random_Permutation_1to5(int result[5]);
void Start_New_Activation_Sequence(void);
void Big_Round_Control(uint8_t action);
void GetTwoDifferentRandom_1to5(uint8_t *a, uint8_t *b);

//速度参数
double a_ = 0, w_ = 0, b_ = 0, t = 0;
double sin_num = 0;
double sin_num_1 = 0;
double sin_num_2 = 0;

//时间戳
uint32_t elapsed = 0;
uint32_t delay_before = 0;
uint32_t delay_after = 0;
uint32_t start_time_small = 0;
uint32_t start_time_big = 0;
uint32_t start_time_big2 = 0;
uint32_t current_time_small = 0;
uint32_t current_time_big = 0;
uint32_t current_time_big2 = 0;

Board_Order_info_t Board_Order_info[6];
Number number;
move_t chassis_move;
behaviour_e mode = no;   //模式
behaviour_e last_mode;   //上次模式

//小能量机关顺序
//int sequence[5];
int sequence[5] = {4,1,2,3
,5};

//能量机关颜色
int DF_color = 666;

//大能量机关开启和当前时间 为三角函数提供参数
double start_time =0, current_time = 0;

//小能量机关状态变量
uint32_t current_activation_time = 0; //当前激活灯板的开始时间（ms）
uint8_t current_active_index = 0;     //当前激活的是第几个
uint8_t current_step = 0;             //当前激活到第几步

//大能量机关状态变量
uint8_t big_active_board1 = 0;        //当前激活的第一块扇叶
uint8_t big_active_board2 = 0;        //当前激活的第二块扇叶

uint8_t hit1 = 0;                     //击打标志位1
uint8_t hit2 = 0;                     //击打标志位2
uint8_t any_hit = 0;                  //可激活的任一灯板被激活
uint8_t both_hit = 0;                 //可激活灯板均被激活
uint8_t wrong_hit_small = 0;          //小能量机关错误激活
uint8_t wrong_hit_big = 0;            //大能量机关错误激活

uint32_t big_round_start_time = 0;    //本轮开始时间戳 (ms)
uint8_t big_round_count = 0;          //已成功完成的轮数
uint8_t big_first_hit_done = 0;       //第一击成功
uint8_t big_second_hit_done = 0;      //第二击成功
uint8_t total_hits = 0;               //击中总数汇总
float hit_rings[10] = {0};            //记录每次击中环数
float average_rings = 0.0f;           //平均环数（总环数 / total_hits）
uint32_t another_board = 0;           //可在1s内激活的灯板
uint8_t big_rune_sent = 0;            //大符结果发送标志位

uint32_t ignore_err_time = 0;  //消抖
uint8_t old_active_board1 = 0;        // 上一轮的第一块灯板
uint8_t old_active_board2 = 0;        // 上一轮的第二块灯板
uint32_t round_switch_time = 0;       // 轮次切换发生的时间戳

void M15_task(void const * argument)
{
	move_init(&chassis_move);
	srand(HAL_GetTick());
	while(1)
	{
		//设置控制模式
    	set_mode(&chassis_move);
		//数据更新
		feedback_update(&chassis_move);   
		//控制量设置
		set_contorl(&chassis_move);		
	    //pid计算
		control_loop(&chassis_move);
		
//		CAN_cmd_chassis(chassis_move.rotation_motor_measure.give_current);
		
		CAN_cmd_chassis(chassis_move.wz_set);
		
		vTaskDelay(1);
	}

}

const static fp32 rotation_speed_pid[3] = {ROTATION_MOTOR_SPEED_PID_KP, ROTATION_MOTOR_SPEED_PID_KI, ROTATION_MOTOR_SPEED_PID_KD};

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

/**
  * @brief          设置控制模式
  * @param[in]      none
  * @retval         none
  */
static void set_mode(move_t *move_mode)
{
    if (move_mode == NULL)	{return;}
    behaviour_mode_set(move_mode);
}
void behaviour_mode_set(move_t *move_mode)
{
    if (move_mode == NULL)	{return;}
}

void feedback_update(move_t *move_update)
{

	//云台
	move_update->rotation_motor_measure.speed = move_update->rotation_motor_measure.motor_measure->rpm;

  //更新速度
	move_update->wz = move_update->rotation_motor_measure.speed / 100 / 60 * 2;
}

/**
  * @brief          控制量设置
  * @param[in]      none
  * @retval         none
  */
static void set_contorl(move_t *move_control)
{
    if (move_control == NULL)	{return;}

    fp32 wz_set = 0.0f;		
		
		//确定模式、颜色
		if(move_control == NULL)	{return;}
		else
		{
		  if(bluetooth_cmd == 0)
			{
				mode = small_red;
				DF_color = 0;
				//发送灯板颜色
			  CAN_cmd_board_color(DF_color);
			}
			else if(bluetooth_cmd == 1)
			{
				mode = small_blue;
				DF_color = 1;
				//发送灯板颜色
			  CAN_cmd_board_color(DF_color);
			}		
			else if(bluetooth_cmd == 2)
			{
				mode = big_red;
				DF_color = 2;
				//发送灯板颜色
			  CAN_cmd_board_color(DF_color);
			}
			else if(bluetooth_cmd == 3)
			{
				mode = big_blue;
				DF_color = 3;
				//发送灯板颜色
			  CAN_cmd_board_color(DF_color);
			}
			else if(bluetooth_cmd == 9)
			{
				mode = no;
			}
			else
			{
				mode = no;
			}
		}
		
    	//复位
		if(mode == no)
		{
			//关闭所有灯板
			CAN_cmd_close(0);
			//清除累计
			CAN_cmd_qing_chu(1);
			
			for(int i = 1; i < 6; i++)
			{
				Board_Order_info[i].Board_Work = OFF;
			}
			
			move_control->wz_set = 0;
		}
    
		//小能量机关模式
		if (mode == small_blue || mode == small_red)
		{
			// 初始化（模式切换时）
			if (last_mode != mode)
			{
				CAN_cmd_frame(2);
				//关闭所有灯板
				HAL_Delay(30);
				CAN_cmd_close(0);
				for(int i = 1; i < 6; i++)
				{
					Board_Order_info[i].Board_Work = OFF;
				}
				HAL_Delay(50);
				
				current_step = 0;
				Start_New_Activation_Sequence();
				
				// 开始计时
				start_time_small = HAL_GetTick();
				
			    CAN_cmd_board_mode(current_active_index, NO, Board_Order_info[current_active_index].Board_Work);

				
				last_mode = mode;
			}
			
			if (current_step < 5)
			{
				uint8_t current_board = current_active_index;

				//误击检测
				for (int i = 1; i < 6; i++)
				{
					if (i != current_board && board_mode[i].err_hit == ON)
					{
						wrong_hit_small = 1;
						//清除分板错误击打标志
						CAN_cmd_switch(1);
						board_mode[i].err_hit = 0;
						break;
					}
				}
				if (wrong_hit_small)
				{
//					Start_New_Activation_Sequence();  // 重置序列
					mode = fail; 
					current_step = 6;
					wrong_hit_small = 0;
					
					//关闭所有灯板
					CAN_cmd_close(0);
					for(int i = 1; i < 6; i++)
					{
						Board_Order_info[i].Board_Work = OFF;
					}
				}
				// 无误击，判断超时/正确击中
				else if(!wrong_hit_small && current_step < 6)  
				{
					current_time_small = HAL_GetTick();
					//超时判定
					if (current_time_small -  start_time_small >= 2500)
					{
//						Start_New_Activation_Sequence();  // 超时重置
						mode = fail; 
						current_step = 6;
						
						//关闭所有灯板
						CAN_cmd_close(0);
						HAL_Delay(50);
						
						for(int i = 1; i < 6; i++)
						{
							Board_Order_info[i].Board_Work = OFF;
						}
					}
					//正确激活
					else if (board_mode[current_board].Single_Hit_State == ON)
					{
						current_step++;
					
						if (current_step < 5)
						{
							uint8_t next_board = sequence[current_step];
							Board_Order_info[next_board].Board_Work = ON;
							
				      		CAN_cmd_board_mode(next_board, NO, Board_Order_info[next_board].Board_Work);
							HAL_Delay(50);
							current_active_index = next_board;
							// 开始计时
			      	start_time_small = HAL_GetTick();
						}
						else
						{
							//点亮全部边框
							CAN_cmd_frame(1);
							
							current_step = 6;
						}
					}
				}
			}
			//M15速度设定
			wz_set = 1000 * 3.5;//减速比1：3
			move_control->wz_set = wz_set;			
		}
		
		
		// 大能量机关模式
		else if (mode == big_red || mode == big_blue)
		{
			// 初始化
			if (last_mode != mode )
			{
				CAN_cmd_lamp_arm_progress(0);
				//关闭所有灯板 避免残留
				HAL_Delay(20);
				CAN_cmd_close(0);
				HAL_Delay(20);
				//同步结构体 工作状态
				for(int i = 1; i < 6; i++)
				{
					Board_Order_info[i].Board_Work = OFF;
					board_mode[i].err_hit = OFF;          // 强制清空物理板历史残留错误标志
					board_mode[i].Single_Hit_State = OFF; // 强制清空物理板历史残留击中标志
				}
				CAN_cmd_qing_chu(1);
                HAL_Delay(20);
                CAN_cmd_switch(1); // 强制让分板硬件清除错误
                HAL_Delay(20);
				//生成速度随机参数
				Random_Number();
				//初始化速度参数
				a_ = number.a; w_ = number.w; b_ = number.b;wz_set = 0;
				//为时间t获取时间戳
				start_time = HAL_GetTick();
				//总环数
				total_hits = 0;
				memset(hit_rings, 0, sizeof(hit_rings));
				//平局环数
				average_rings = 0.0f;					
				//启动第一组随机
				big_round_count = 1;
				Big_Round_Control(1);
				//记录（第一组）开始时 时间戳				
				start_time_big = HAL_GetTick();
				//ignore_err_time = HAL_GetTick(); // <--- 新增：进入大符模式时给免疫期
				//标志位清除
				big_first_hit_done = 0;
				big_second_hit_done = 0;
				hit1 = 0;
				hit2 = 0;
				any_hit = 0;
				both_hit = 0;
				big_rune_sent = 0;
				last_mode = mode;
			}
			
			if(big_round_count > 6)
			{
				big_round_count = 0;
			}
			else if(big_round_count > 0)
			{
			  CAN_cmd_lamp_arm_progress(big_round_count - 1);
			}
//			else if(big_round_count == 0)
//			{
//				CAN_cmd_lamp_arm_progress(0);
//			}
			
			HAL_Delay(50);
			CAN_cmd_board_mode(big_active_board1, NO, Board_Order_info[big_active_board1].Board_Work);
			CAN_cmd_board_mode(big_active_board2, NO, Board_Order_info[big_active_board2].Board_Work);
			HAL_Delay(50);
//			// 误击检测
			for (int i = 1; i < 6; i++)
			{
				// 发现当前非目标的灯板报错了
				if (i != big_active_board1 && i != big_active_board2 && board_mode[i].err_hit == ON)
				{
					// 【核心修复】：如果是上一轮刚打完的灯板，且轮次切换还不到 800ms（物理余震期）
					if ((i == old_active_board1 || i == old_active_board2) && (HAL_GetTick() - round_switch_time < 800))
					{
						// 这是装甲板余震导致的假报错，不触发失败，只清理残留标志
						CAN_cmd_switch(1);         // 通知子板清除
						board_mode[i].err_hit = 0; // 主控本地清除
						continue;                  // 跳过本次循环，不上报 wrong_hit_big
					}

					// 如果不是刚才打的灯板，或者过了 800ms 还在乱报错，那说明是真正的误击
					wrong_hit_big = 1;
					// 清除分板 错误击打标志
					CAN_cmd_switch(1);
					// 清除主板标志
					board_mode[i].err_hit = 0;
					break;
				}
			}
			if (wrong_hit_big)
			{
//				Big_Round_Control(1);
//				big_first_hit_done = 0;
//				big_second_hit_done = 0;
//				//清空环数记录
//				for(int i = 0; i < 5; i++)
//				{
//					hit_rings[i] = 0;
//				}
				
				//关闭所有灯板
				CAN_cmd_close(0);
				CAN_cmd_lamp_arm_progress(0);
				for(int i = 1; i < 6; i++)
				{
					Board_Order_info[i].Board_Work = OFF;
				}			
				mode = fail;
				big_round_count = 0;					
				wrong_hit_big = 0;
			}
			//无错误激活
			else
			{
				if (board_mode[big_active_board1].Single_Hit_State == ON) {hit1 = 1;}
				if (board_mode[big_active_board2].Single_Hit_State == ON) {hit2 = 1;}	
			
				//任意击打判定
				if(hit1 || hit2){any_hit = 1;}
				//全部击打判定
				if(hit1 && hit2){both_hit = 1;}
				//获取当前时间戳
				current_time_big = HAL_GetTick();
				// 2.5s内未激活任一可激活灯板
				if(current_time_big - start_time_big >= 3500 && hit1 == 0 && hit2 == 0)
				{
//						Big_Round_Control(1);
//						big_first_hit_done = 0;
//						big_second_hit_done = 0;
//						//清空环数记录
//						for(int i = 0; i < 10; i++)
//						{
//							hit_rings[i] = 0;
//						}
					
					//关闭所有灯板
					CAN_cmd_close(0);
					for(int i = 1; i < 6; i++)
					{
						Board_Order_info[i].Board_Work = OFF;
					}
					mode = fail;
					big_round_count = 0;	
					
					current_time_big = 0;
					start_time_big = 0;
				}
				// 成功激活一个灯板
				else if(current_time_big > start_time_big && current_time_big - start_time_big <= 3500 && big_first_hit_done == 0 && any_hit)
				{
				    ignore_err_time = HAL_GetTick(); //记录击中瞬间
					big_first_hit_done = 1;
					current_time_big = 0;
					start_time_big = 0;
					//获取此时 时间戳
					start_time_big2 = HAL_GetTick();
					total_hits++;
					// 熄灭被击中的灯板
					if(hit1) 
					{
						Board_Order_info[big_active_board1].Board_Work = OFF;
						CAN_cmd_board_mode(big_active_board1, NO, Board_Order_info[big_active_board1].Board_Work);
						hit_rings[total_hits - 1] = board_mode[big_active_board1].num_of_hit_ring;
						another_board = big_active_board2;
					}
					else if(hit2) 
					{
						Board_Order_info[big_active_board2].Board_Work = OFF;
						CAN_cmd_board_mode(big_active_board2, NO, Board_Order_info[big_active_board2].Board_Work);
						hit_rings[total_hits - 1] = board_mode[big_active_board2].num_of_hit_ring;
						another_board = big_active_board1;
					}	
						
				}
				//获取当前时间戳
				if(big_first_hit_done)
				{
				  current_time_big2 = HAL_GetTick();
				}
				// 第二击判定：第一击后1s内完成，击中另一个
				if(big_first_hit_done && current_time_big2 -start_time_big2 <= 1000 && both_hit && big_second_hit_done == 0)
				{
					    ignore_err_time = HAL_GetTick(); //记录击中瞬间
						big_second_hit_done = 1;
						total_hits++;
						hit_rings[total_hits - 1] = board_mode[another_board].num_of_hit_ring;
						Board_Order_info[another_board].Board_Work = OFF;
						CAN_cmd_board_mode(another_board, NO, Board_Order_info[another_board].Board_Work);
				}

				//轮次强制结束（第二击1s后）
				if ((current_time_big2 -start_time_big2 >= 1000 && big_first_hit_done) || (big_second_hit_done && big_first_hit_done))
				{
					// === 新增：记录刚才打完的灯板，准备进入定向消抖 ===
					old_active_board1 = big_active_board1;
					old_active_board2 = big_active_board2;
					round_switch_time = HAL_GetTick();
					//击中状态清零
					board_mode[big_active_board1].Hit_LED_state = OFF;
					board_mode[big_active_board2].Hit_LED_state = OFF;
					board_mode[big_active_board1].Single_Hit_State = OFF;
					board_mode[big_active_board2].Single_Hit_State = OFF;

					board_mode[big_active_board1].err_hit = OFF;
					board_mode[big_active_board2].err_hit = OFF;
					CAN_cmd_switch(1); // 发送指令强制清空分板的残留误击标志
					// for(int i = 1; i < 6; i++)
					// {
					// 	board_mode[i].Hit_LED_state = OFF;
					// 	board_mode[i].Single_Hit_State = OFF;
					// 	board_mode[i].err_hit = OFF;
					// }
					// CAN_cmd_switch(1); // 发送指令强制清空分板的残留误击标志
					
					CAN_cmd_close(0);
					for(int i = 1; i < 6; i++)
					{
						Board_Order_info[i].Board_Work = OFF;
					}			
					
					//击中标志位和时间清零
					big_first_hit_done = 0;
					big_second_hit_done = 0;
					hit1 = 0;
					hit2 = 0;
					any_hit = 0;
					both_hit = 0;
					current_time_big2 = 0;
					start_time_big2 = 0;
					
					big_round_count++;

					//未满5组 开启新一轮
					if(big_round_count < 6 && big_round_count > 0)
					{
						Big_Round_Control(big_round_count);
						start_time_big = HAL_GetTick();
						//ignore_err_time = HAL_GetTick(); // <--- 新增：切轮时刷新免疫期
					}
					//激活满5轮
					else
					{
						//点亮全部边框
						CAN_cmd_frame(1);
						HAL_Delay(20);
						//点亮全部灯臂、边框灯带
						CAN_cmd_lamp_arm_progress(5);
						HAL_Delay(20);
						big_round_count = 0;
						mode = fail;

//						if(total_hits > 0)
//						{
//								float sum = 0.0f;
//								for (int i = 0; i < 10; i++) sum += hit_rings[i];
//								average_rings = sum / total_hits;
//						}
//						
//						else
//						{
//								average_rings = 0.0f;
//						}

//						// 每次完成都发送一次（因为进入模式时已重置标志）
//						if (big_rune_sent == 0)
//						{
//								Bluetooth_Send_BigRune_Result(total_hits, average_rings);
//								big_rune_sent = 1;  // 本次大符已发送，防止本轮重复发
//						}
					}
				}
			}
			
			current_time = (HAL_GetTick() - start_time) / 1000.0;
			t = current_time;
			wz_set = 0;
			sin_num = sin(w_ * t);
			sin_num_1 = a_ * sin_num;
			sin_num_2 = sin_num_1 + b_; 
			move_control->wz_set = (sin_num_2 / 2 / PI) * 60 * 100 * 3.5;
		}
		
		//速度设定		
		move_control->rotation_motor_measure.speed_set = move_control->wz_set;
		CAN_cmd_chassis_set_feedback(0x01);	
		CAN_cmd_chassis_set_mode(MODE_2,MODE_2);
}

//生成随机速度参数
void Random_Number(void)
{
	double min_a = 0.780, max_a = 1.045;
	double min_w = 1.884, max_w = 2.000;
	number.a = min_a + (max_a - min_a) * ((double)rand() / RAND_MAX);
	number.w = min_w + (max_w - min_w) * ((double)rand() / RAND_MAX);
	number.b = 2.090 - number.a;
}

/**
 * @brief 生成一个包含1~5全排列的数组
 * @param result  输出数组指针，必须由调用者提供至少5个元素的空间
 */
void Get_Random_Permutation_1to5(int result[5])
{
    for(int i = 0; i < 5; i++)
    {
        result[i] = i + 1;
    }
    // Fisher-Yates 洗牌算法
    for(int i = 4; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int temp = result[i];
        result[i] = result[j];
        result[j] = temp;
    }
}
/**
 * @brief  生成两个不同的 1~5 的随机整数，并写入传入的指针
 * @param  a   指向第一个数的指针
 * @param  b   指向第二个数的指针
 * @note   保证 *a != *b，且都在 1~5 范围内
 * @note   必须在程序开始时（main函数最前面）调用一次 srand() 进行种子初始化
 */
void GetTwoDifferentRandom_1to5(uint8_t *a, uint8_t *b)
{
    uint8_t x, y;
    
    do {
        x = (rand() % 5) + 1;   // 1~5
        y = (rand() % 5) + 1;
    } while (x == y);           // 如果相等就重来

    *a = x;
    *b = y;
}

//小能量机关随机顺序
void Start_New_Activation_Sequence(void)
{
		//随机排列灯板亮起顺序
//		Get_Random_Permutation_1to5(sequence);
    uint8_t first_board = sequence[0];
    Board_Order_info[first_board].Board_Work = ON;
    current_active_index = first_board;
}

/**
 * @brief 大能量机关轮控制
 * @param action  1: 失败重置
 *                2: 正常完成本轮
 */
void Big_Round_Control(uint8_t action)
{
	if (action > 0 && action < 6)
	{
		GetTwoDifferentRandom_1to5(&big_active_board1, &big_active_board2);

		Board_Order_info[big_active_board1].Board_Work = ON;
		Board_Order_info[big_active_board2].Board_Work = ON;
	}
	else
	{
		mode = fail;
	}
}

static void control_loop(move_t *move_control_loop)
{
	PID_calc(&move_control_loop->rotation_motor_pid, move_control_loop->rotation_motor_measure.speed, move_control_loop->rotation_motor_measure.speed_set);
	move_control_loop->rotation_motor_measure.give_current = (int16_t)(move_control_loop->rotation_motor_pid.out);
}

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

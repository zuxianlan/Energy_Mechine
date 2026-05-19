#ifndef BLUETOOTN_RECEIVE_H
#define BLUETOOTN_RECEIVE_H

#include "main.h"  // 包含 huart2 的声明

// 全局变量：蓝牙命令（0~3，其他为666）
extern uint16_t bluetooth_cmd;

// 函数声明
void Bluetooth_Init(void);           // 初始化蓝牙接收（中断方式）
void Bluetooth_Process(void);   
// 发送大符完成数据的函数
void Bluetooth_Send_BigRune_Result(uint8_t hits, float average);

#endif

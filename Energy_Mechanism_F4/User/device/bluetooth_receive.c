#include "bluetooth_receive.h"
#include "usart.h"
#include <stdio.h>  // 用于 sprintf
#include <string.h>

// 发送完成数据的函数
void Bluetooth_Send_BigRune_Result(uint8_t hits, float average)
{
    char tx_buffer[32];
    // 格式：H:xx,A:xx.x\n   （xx为整数，平均保留1位小数）
    int len = snprintf(tx_buffer, sizeof(tx_buffer), "H:%d,A:%.1f\n", hits, average);
    
    // 阻塞发送（安全，数据短）
    HAL_UART_Transmit(&huart2, (uint8_t*)tx_buffer, len, HAL_MAX_DELAY);
    
    // 可选：调试串口也打印一份
    // printf("%s", tx_buffer);
}
// 全局变量定义
uint16_t bluetooth_cmd = 666;

// 接收缓冲（单字节）
static uint8_t rx_byte;

// 初始化函数
void Bluetooth_Init(void)
{
    // 开启 USART2 单字节中断接收
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
}

// UART 接收完成回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) 
    {
        // 根据接收到的字符赋值全局变量
        switch (rx_byte)
        {
            case '0':
                bluetooth_cmd = 0;
                break;
            case '1':
                bluetooth_cmd = 1;
                break;
            case '2':
                bluetooth_cmd = 2;
                break;
            case '3':
                bluetooth_cmd = 3;
                break;
            case '4':
                bluetooth_cmd = 4;
                break;
            case '9':
                bluetooth_cmd = 9;
                break;
            default:
                bluetooth_cmd = 666;
                break;
        }

        // 重新开启下一次中断接收
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}

// 可选：主循环中处理函数（目前无需额外逻辑，可留空）
void Bluetooth_Process(void)
{

}

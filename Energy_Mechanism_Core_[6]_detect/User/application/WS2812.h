#ifndef _WS2812_H
#define _WS2812_H


#include "main.h"

#define PIXEL_NUM_R  64
#define NUM_R (24*PIXEL_NUM_R + 250)        // Reset 280us / 1.25us = 224

#define WS1  134
#define WS0  59


void WS_Write(uint8_t n_R, uint8_t n_G, uint8_t n_B ,int16_t WS_NUM ,int16_t i);
void WS_Load_R(void);
void WS_CloseAll_R(void);
void WS_CloseAll_board(void);
void WS_Stop_DMA(uint8_t channel);

#endif

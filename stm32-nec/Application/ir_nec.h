/*
 * ir_nec.h
 *
 *  Created on: Dec 9, 2023
 *      Author: onste
 */

#ifndef IR_NEC_H_
#define IR_NEC_H_

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <string.h>

#define MAX_NEC_CNT 	33
#define IR_HEADER_LOW	9000
#define IR_HEADER_HIGH	4500
#define IR_HEADER_MARGIN			200

#define BIT0_TIME		560
#define BIT1_TIME		1650
#define BIT_TIME_MARGIN	100

extern TIM_HandleTypeDef htim2;
extern DMA_HandleTypeDef hdma_tim2_ch1;
extern DMA_HandleTypeDef hdma_tim2_ch2_ch4;
extern DMA_HandleTypeDef hdma_memtomem_dma1_channel1;
extern UART_HandleTypeDef huart2;

typedef enum
{
		NEC_INIT,
		NEC_FORMAT,
		NEC_REPEAT
}IR_Format;

typedef struct
{
	uint16_t check_start_low;
	uint16_t check_start_high;

	uint16_t edge_falling[MAX_NEC_CNT];
	uint16_t edge_rising[MAX_NEC_CNT];

	uint16_t raw_capture[MAX_NEC_CNT];

	uint8_t data[MAX_NEC_CNT-1];
	uint8_t cap_cnt;
	int8_t decoded[4];
	bool taskFlag;
	bool isEnd;
	IR_Format state;
} NEC;

void irInit();
void irCheckStart();
bool irDecode();
void irStart(IR_Format check_format);


bool irNecTask();

bool irNecData();
#endif /* IR_NEC_H_ */

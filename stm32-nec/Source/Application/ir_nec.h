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

<<<<<<< HEAD:stm32-nec/Source/Application/ir_nec.h
#define MAX_NEC_CNT	50
=======
#define MAX_NEC_CNT 33
>>>>>>> 3b1d541bb81b4484fb4343d26d5d41130c5226ca:stm32-nec/Application/ir_nec.h

extern TIM_HandleTypeDef htim2;
extern DMA_HandleTypeDef hdma_tim2_ch1;
extern DMA_HandleTypeDef hdma_tim2_ch2_ch4;
extern DMA_HandleTypeDef hdma_memtomem_dma1_channel1;

typedef enum
{
<<<<<<< HEAD:stm32-nec/Source/Application/ir_nec.h
	NEC_INIT,
	NEC_FORMAT,
	NEC_REPEAT
=======
		NEC_INIT,
		NEC_FORMAT,
		NEC_REPEAT
>>>>>>> 3b1d541bb81b4484fb4343d26d5d41130c5226ca:stm32-nec/Application/ir_nec.h
}IR_Format;

typedef struct
{
	bool isError;
	uint16_t startL;
	uint16_t startH;
	uint16_t edge_falling[MAX_NEC_CNT];
	uint16_t edge_rising[MAX_NEC_CNT];
	uint16_t raw_capture[MAX_NEC_CNT];
<<<<<<< HEAD:stm32-nec/Source/Application/ir_nec.h
	uint16_t data[MAX_NEC_CNT/2];
=======

	uint8_t data[MAX_NEC_CNT];
>>>>>>> 3b1d541bb81b4484fb4343d26d5d41130c5226ca:stm32-nec/Application/ir_nec.h
	uint8_t cap_cnt;
	uint8_t decoded[4];
	bool taskFlag;
	IR_Format state;
	void (*errCallback)();

} NEC;

void irNecInit();

void irNecStart();

bool irNecDecode();

bool irNecTask();

bool irNecData();
#endif /* IR_NEC_H_ */

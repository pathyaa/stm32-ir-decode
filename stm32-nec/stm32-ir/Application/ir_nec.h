#ifndef IR_NEC_H_
#define IR_NEC_H_

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <string.h>

#define MAX_NEC_PACKET_CNT 33
#define MAX_NEC_REPEAT_CNT 2

extern TIM_HandleTypeDef htim2;
extern DMA_HandleTypeDef hdma_tim2_ch1;
extern DMA_HandleTypeDef hdma_tim2_ch2_ch4;
extern UART_HandleTypeDef huart2;

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif



typedef enum
{
	NEC_INIT,
	NEC_FORMAT,
	NEC_REPEAT,
	NEC_RESET,
	NEC_END
}IR_Format;

typedef struct
{
	uint16_t startL;
	uint16_t startH;

	uint16_t edge_falling[MAX_NEC_PACKET_CNT];
	uint16_t edge_rising[MAX_NEC_PACKET_CNT];
	uint16_t raw_capture[MAX_NEC_PACKET_CNT];
	uint8_t data[MAX_NEC_PACKET_CNT-1];
	int8_t decoded[4];

	volatile uint8_t cap_cnt;

	bool isInit;
	uint16_t repeatCheckL[MAX_NEC_REPEAT_CNT];
	uint16_t repeatCheckH[MAX_NEC_REPEAT_CNT];
	bool taskFlag;
	IR_Format state;
} NEC;

void irInit(IR_Format cur_state);
void irReset(void);
void irStart(void);
void irGetData(void);
void irDataDecode(void);
void irTask(void);
#endif /* IR_NEC_H_ */

#ifndef NEC_H_
#define NEC_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>


// Header Low Time  : 9000 us
// Header High Time : 4500 us
#define AGC_START_TIME	13500

// Repeat Pulse : 11800 us
#define REPEAT_TIME     11800

// BIT1_BOUNDARY : 1650 us
#define BIT1_BOUNDARY	1650

extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart2;

typedef enum {
	NEC_IDLE,
	NEC_BEGIN,
	NEC_CHECK_HEADER,
    NEC_CHECK_REPEAT,
    NEC_DECODE,
	NEC_REPEAT
} NEC_STATE;



void necInit(void);
void nec(void);
void necDecode(void);

#endif /* INC_NEC_DECODE_H_ */

#ifndef NEC_H_
#define NEC_H_

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#define AGC_START_TIME	12500
#define BIT_BOUNDARY	1650

typedef enum {
    NEC_NOT_EXTENDED, NEC_EXTENDED
} NEC_TYPE;

typedef enum {
    NEC_INIT, NEC_AGC_OK, NEC_AGC_FAIL, NEC_FAIL, NEC_OK
} NEC_STATE;

typedef struct {
    uint16_t rawTimerData[32];
    uint8_t decoded[4];

    NEC_STATE state;

    TIM_HandleTypeDef *timerHandle;

    uint32_t timerChannel;
    HAL_TIM_ActiveChannel timerChannelActive;

    uint16_t timingBitBoundary;
    uint16_t timingAgcBoundary;
    NEC_TYPE type;

    void (*NEC_DecodedCallback)(uint8_t, uint8_t);
    void (*NEC_ErrorCallback)();
    void (*NEC_RepeatCallback)();
} NEC;

extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart2;
extern NEC nec;

void NEC_Init(NEC* handle);

void NEC_DeInit(NEC* handle);

void NEC_TIM_IC_CaptureCallback(NEC* handle);

void NEC_Read(NEC* handle);

void myNecDecodedCallback(uint8_t address, uint8_t cmd);
void myNecErrorCallback(void);
void myNecRepeatCallback(void);

#endif /* INC_NEC_DECODE_H_ */

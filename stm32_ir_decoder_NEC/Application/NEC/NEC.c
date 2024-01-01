
/*
 * NEC_Decode.c
 *
 *  Created on: Mar 9, 2016
 *      Author: peter
 */

#include "NEC.h"

// Header Low Time  : 9000 us 
// Header High Time : 4500 us
#define AGC_START_TIME	13500

// Repeat Pulse : 11800 us
#define REPEAT_TIME     11800

// BIT1_BOUNDARY : 1650 us
#define BIT_BOUNDARY	1650

static TIM_HandleTypeDef *nec_timer = &htim2;
static UART_HandleTypeDef *nec_uart = &huart2;

typedef enum {
	NEC_IDLE,
    NEC_INIT, 
	NEC_CHECK_HEADER,
    NEC_AGC_OK, 
    NEC_AGC_FAIL,
    NEC_CHECK_REPEAT,
    NEC_DECODE
} NEC_STATE;

typedef struct {
	uint16_t header[2];
    uint16_t rawTimerData[32];
	uint16_t repeat[2];
    uint8_t  decoded[4];
    uint16_t timingBitBoundary;
    uint16_t timingAgcBoundary;
} NEC;

bool repeat_flag = false;
static uint8_t nec_cmd = 0;

void nec(void)
{
	static NEC nec;
	static NEC_STATE nec_state = NEC_INIT;
	uint16_t header_time = nec.header[1];
	uint16_t repeat_time = nec.repeat[1];
	static uint32_t header_check_time = 0;

	switch(nec_state)
	{
		case NEC_INIT :
			nec.timingBitBoundary = 1650;
			nec.timingAgcBoundary = 13500;
			// SLAVE MODE 의 RESET MODE 이므로, 처음 FALLING 에서 RESET -> HEADER 끝부분에서 다시 FALLING 할 때 CAPTURE VALUE 를 header 에 저장
			HAL_UART_Receive_DMA(&huart2, (uint8_t*)&nec_cmd, 1);
			printf("Press Any key, then NEC Decoding Start\r\n");
			HAL_TIM_IC_Start_DMA(nec_timer, TIM_CHANNEL_1, (uint32_t*)nec.header, 2);
			nec_state = NEC_CHECK_HEADER;	
		break;

		case NEC_CHECK_HEADER :
			if (header_time < AGC_START_TIME + 500 && header_time > AGC_START_TIME - 500)
			{
				HAL_TIM_IC_Start_DMA(nec_timer, TIM_CHANNEL_1, (uint32_t*)nec.rawTimerData, 32);
				nec_state = NEC_AGC_OK;
			}
			else
			{
				if (HAL_GetTick()-header_check_time >= 5000)
				{
					printf("NEC Begin Again!\r\n");
					nec_state = NEC_INIT;
				}
			}
		break;

		case NEC_AGC_OK :
			for (uint8_t pos = 0; pos < 32; pos++)
			{
				uint16_t time = nec.rawTimerData[pos];
				if (time > BIT_BOUNDARY)
				{
					nec.decoded[pos / 8] |= 1 << (pos % 8);
				}
				else
				{
					nec.decoded[pos / 8] &= ~(1 << (pos % 8));
				}
			}

			uint8_t addr = nec.decoded[0];
			uint8_t naddr = nec.decoded[1];
			uint8_t data = nec.decoded[2];
			uint8_t ndata = nec.decoded[3];
			
			if ((addr == ~naddr) && (data == ~ndata))
			{
				HAL_TIM_IC_Start_DMA(nec_timer, TIM_CHANNEL_1, (uint8_t*)nec.repeat, 2);
				nec_state = NEC_CHECK_REPEAT;
			}
			else
			{
				nec_state = NEC_AGC_FAIL;
			}
			break;

		case NEC_CHECK_REPEAT :
			if (repeat_time > 12500 && repeat_time < 13000)
			{
				repeat_flag = true;
			}
		break;

		case NEC_DECODE :
			if (repeat_flag)
			{
				repeat_flag = false;
				printf("Repeat Pulse : ADDR : 0x%x, DATA : 0x%x\r\n", nec.decoded[0], nec.decoded[2]);
			}
			else
			{
				printf("ADDR : 0x%x, DATA : 0x%x\r\n", nec.decoded[0], nec.decoded[2]);
			}
			nec_state = NEC_IDLE;
		break;

		case NEC_AGC_FAIL :

			nec_state = NEC_IDLE;
		break;

		case NEC_IDLE :
			if (nec_cmd == '1')
			{
				memset(nec.repeat, 0, sizeof(uint16_t) * 2);
				memset(nec.rawTimerData, 0, sizeof(uint16_t) * 32);
				memset(nec.decoded, 0, sizeof(uint8_t) * 4);
				nec_state = NEC_INIT;
			}
		break;
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	static NEC_STATE state_idx = NEC_INIT;
    if (htim == &htim2)
    {
        state_idx++;
    }
}

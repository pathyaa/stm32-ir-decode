
/*
 * NEC_Decode.c
 *
 *  Created on: Mar 9, 2016
 *      Author: peter
 */

#include "NEC.h"

NEC user_nec;

TIM_HandleTypeDef *nec_timer = &htim2;

bool isRepeat = false;
bool isInit = false;
bool isDecode = false;
bool check_header = false;
bool necResetFlag = false;

void necInit(void)
{
	user_nec.state = NEC_IDLE;
	HAL_TIM_IC_Start_DMA(nec_timer, TIM_CHANNEL_1, (uint32_t*)user_nec.header, 2);
	isInit = true;
	printf("Wait for IR (NEC) Signal\r\n");
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

    if (htim == &htim2)
    {
    	if (user_nec.state == NEC_IDLE)
    	{
    		uint16_t header_time = user_nec.header[1];
    		if (header_time < AGC_START_TIME + 500 && header_time > AGC_START_TIME - 500)
			{
    			check_header = true;
    			HAL_TIM_IC_Start_DMA(nec_timer, TIM_CHANNEL_1, (uint32_t*)user_nec.rawTimerData, 32);
    			printf("start decode\r\n");
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
				user_nec.state = NEC_DECODE;
			}
    	}
    	else if (user_nec.state == NEC_DECODE)
    	{
    		isDecode = true;
			if (user_nec.repeat[1] > 11000)
			{
				isRepeat = true;
				user_nec.state = NEC_REPEAT;
			}
    	}
    }
}
void nec(void)
{
	static uint32_t led_time = 0;

	switch(user_nec.state)
	{
		case NEC_IDLE :
			if (HAL_GetTick()-led_time >= 1000)
			{
				led_time = HAL_GetTick();
				HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			}
		break;

		case NEC_BEGIN :
			if (check_header)
			{
				check_header = false;
				user_nec.state = NEC_DECODE;
				printf("nec state   	   : %s\r\n", user_nec.state == NEC_BEGIN ? "NEC_BEGIN" : "ERR");
				printf("check header 	   : %s\r\n", user_nec.header[1] > AGC_START_TIME ? "Start Decoding" : "ERR");
				printf("header 			   : %d\r\n", user_nec.header[1]);
			}
			
		break;

		case NEC_DECODE :
			if (isDecode)
			{
				necDecode();
			}
			break;

		case NEC_REPEAT :

			if (isRepeat)
			{
				isRepeat = false;
				printf("[Repeat] ADDR      : 0x%x\r\n", user_nec.decoded[0]);
				printf("[Repeat] inv_ADDR  : 0x%x\r\n", user_nec.decoded[1]);
				printf("[Repeat] DATA      : 0x%x\r\n", user_nec.decoded[2]);
				printf("[Repeat] int_DATA  : 0x%x\r\n", user_nec.decoded[3]);
			}
			else
			{
				printf("nec initialize\r\n");
				isInit = false;
				check_header = false;
				isDecode = false;
				isRepeat = false;
				user_nec.state = NEC_IDLE;
			}
			break;
	}

}


void necDecode(void)
{
	for (uint8_t pos = 0; pos < 32; pos++)
	{
		uint16_t time = user_nec.rawTimerData[pos];
		if (time > BIT1_BOUNDARY)
		{
			user_nec.decoded[pos / 8] |= 1 << (pos % 8);
		}
		else
		{
			user_nec.decoded[pos / 8] &= ~(1 << (pos % 8));
		}
	}

	uint8_t addr = user_nec.decoded[0];
	uint8_t naddr = user_nec.decoded[1];
	uint8_t data = user_nec.decoded[2];
	uint8_t ndata = user_nec.decoded[3];

	isDecode = false;
	printf("nec addr 	: 0x%x\r\n", addr);
	printf("nec naddr 	: 0x%x\r\n", naddr);
	printf("nec data 	: 0x%x\r\n", data);
	printf("nec ndata 	: 0x%x\r\n", ndata);
	

	HAL_TIM_IC_Start_DMA(nec_timer, TIM_CHANNEL_1, (uint32_t*)user_nec.repeat, 2);

	if ((addr == ~naddr) && (data == ~ndata))
	{

	}
}



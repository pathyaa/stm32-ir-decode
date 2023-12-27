
/*
 * NEC_Decode.c
 *
 *  Created on: Mar 9, 2016
 *      Author: peter
 */

#include "NEC.h"

void NEC_Init(NEC* handle)
{
	handle->timerHandle = &htim2;

	handle->timerChannel = TIM_CHANNEL_1;
	handle->timerChannelActive = HAL_TIM_ACTIVE_CHANNEL_1;

	handle->timingBitBoundary = 1650;
	handle->timingAgcBoundary = 13500;
	handle->type = NEC_NOT_EXTENDED;

	handle->NEC_DecodedCallback = myNecDecodedCallback;
	handle->NEC_ErrorCallback = myNecErrorCallback;
	handle->NEC_RepeatCallback = myNecRepeatCallback;

	NEC_Read(handle);
}

void NEC_TIM_IC_CaptureCallback(NEC* handle)
{
	switch(handle->state)
	{
	case NEC_INIT :
		
		uint16_t header_time = handle->rawTimerData[1];
		// Repeat length = 11.825ms
		if (header_time < 12000 && header_time > 11000)
		{
			handle->state = NEC_OK;
			handle->NEC_RepeatCallback();
		}
		else if (header_time < AGC_START_TIME + 500 && header_time > AGC_START_TIME - 500)
		{
			handle->state = NEC_AGC_OK;
			HAL_TIM_IC_Start_DMA(handle->timerHandle, handle->timerChannel,
					(uint32_t*) handle->rawTimerData, 32);
		}
		else
		{
			handle->state = NEC_AGC_FAIL;
		}
		break;

	case NEC_AGC_OK :
		
		for (uint8_t pos = 0; pos < 32; pos++)
		{
			uint16_t time = handle->rawTimerData[pos];
			if (time > BIT_BOUNDARY)
			{
				handle->decoded[pos / 8] |= 1 << (pos % 8);
			}
			else
			{
				handle->decoded[pos / 8] &= ~(1 << (pos % 8));
			}
		}

		uint8_t valid = 1;

		uint8_t naddr = ~handle->decoded[1];
		uint8_t ncmd = ~handle->decoded[3];
		
		if ((handle->type == NEC_NOT_EXTENDED && handle->decoded[0] != naddr) || handle->decoded[2] != ncmd)
		{
			valid = 0;
			handle->state = NEC_AGC_FAIL;
		}
		else
		{
			handle->state = NEC_OK;
		}

		if (valid)
			handle->NEC_DecodedCallback(handle->decoded[0], handle->decoded[2]);
		else
			handle->NEC_ErrorCallback();	

		break;

		case NEC_OK :
			handle->NEC_DecodedCallback(handle->decoded[0], handle->decoded[2]);
		break;

		case NEC_FAIL     :
		case NEC_AGC_FAIL :

			NEC_Reset(handle);
		
		break;
	}
}

void NEC_Read(NEC* handle)
{
    handle->state = NEC_INIT;
    HAL_TIM_IC_Start_DMA(handle->timerHandle, handle->timerChannel,
            (uint32_t*) handle->rawTimerData, 2);
}

void NEC_Reset(NEC* handle)
{
	memset(handle->rawTimerData, 0, sizeof(uint16_t) * 32);
	memset(handle->decoded, 0, sizeof(uint8_t) * 4);
//	HAL_TIM_IC_Stop_DMA(handle->timerHandle, handle->timerChannel);
	NEC_Read(handle);
}
void myNecDecodedCallback(uint8_t address, uint8_t cmd)
{
    printf("ADDR : 0x%x, DATA : 0x%x\r\n", address, cmd);
    NEC_Read(&nec);
}

void myNecErrorCallback() {
    char* msg = "Error!\n";
    HAL_UART_Transmit_DMA(&huart2, (uint8_t*) msg, strlen(msg));

    NEC_Reset(&nec);
}

void myNecRepeatCallback() {

	printf("Repeated!!\r\n");

    nec.NEC_DecodedCallback(nec.decoded[0], nec.decoded[2]);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim2)
    {
        NEC_TIM_IC_CaptureCallback(&nec);
    }
}

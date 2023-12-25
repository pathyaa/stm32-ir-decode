
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
}

void NEC_TIM_IC_CaptureCallback(NEC* handle)
{
	switch(handle->state)
	{
	case NEC_INIT :
		uint16_t header_time = handle->rawTimerData[1];
		if (header_time < AGC_START_TIME)
		{
			handle->state = NEC_OK;
			handle->NEC_RepeatCallback();
		}
		else
		{
			handle->state = NEC_AGC_OK;
			HAL_TIM_IC_Start_DMA(handle->timerHandle, handle->timerChannel,
					(uint32_t*) handle->rawTimerData, 32);
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

		if (handle->type == NEC_NOT_EXTENDED && handle->decoded[0] != naddr)
			valid = 0;
		if (handle->decoded[2] != ncmd)
			valid = 0;

		handle->state = NEC_OK;

		if (valid)
			handle->NEC_DecodedCallback(handle->decoded[0], handle->decoded[2]);
		else
			handle->NEC_ErrorCallback();
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

	NEC_Read(handle);
}
void myNecDecodedCallback(uint8_t address, uint8_t cmd)
{
    char buff[100];
    snprintf(buff, 100, "Addr:0x%x\tC:%d\n", address, cmd);
    HAL_UART_Transmit_DMA(&huart2, (uint8_t*) buff, strlen(buff));

    NEC_Reset(&nec);
}

void myNecErrorCallback() {
    char* msg = "Error!\n";
    HAL_UART_Transmit_DMA(&huart2, (uint8_t*) msg, strlen(msg));

    NEC_Reset(&nec);
}

void myNecRepeatCallback() {
    char* msg = "Repeat!\n";
    HAL_UART_Transmit_DMA(&huart2, (uint8_t*) msg, strlen(msg));

    NEC_Reset(&nec);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim2)
    {
        NEC_TIM_IC_CaptureCallback(&nec);
    }
}

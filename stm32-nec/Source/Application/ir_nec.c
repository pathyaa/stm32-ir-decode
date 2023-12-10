/*
 * inputcapture.c
 *
 *  Created on: Dec 9, 2023
 *      Author: onste
 */

#include "ir_nec.h"

NEC nec =
{
	.isError = false,
	.taskFlag = false,
	.cap_cnt = 0,
	.startL = 0,
	.startH = 0,
	.edge_falling = {0,},
	.edge_rising = {0,},
	.raw_capture = {0,},
	.data = {0,},
	.state = NEC_INIT
};
void irNecErrorHandler()
{
	if (nec.isError)
	{
		memset(nec.edge_falling, 0, sizeof(nec.edge_falling));
		memset(nec.edge_rising, 0, sizeof(nec.edge_rising));
		nec.startH = 0;
		nec.startL = 0;
		nec.state = NEC_INIT;
		HAL_TIM_IC_Stop_DMA(&htim2, TIM_CHANNEL_1);
		HAL_TIM_IC_Stop_DMA(&htim2, TIM_CHANNEL_2);
		nec.isError = false;
	}
}
void irNecInit()
{
	HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*)&nec.startH, 1);
	HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_2, (uint32_t*)&nec.startL, 1);
}

void irNecStart()
{
	nec.state = NEC_ADDR;
	HAL_TIM_IC_Stop_DMA(&htim2, TIM_CHANNEL_1);
	HAL_TIM_IC_Stop_DMA(&htim2, TIM_CHANNEL_2);

	HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*)nec.edge_falling, MAX_NEC_CNT);
	HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_2, (uint32_t*)nec.edge_rising, MAX_NEC_CNT);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

	if (htim == &htim2)
	{
	switch (nec.state)
	{
		case NEC_INIT :
			if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
			{
				if (nec.startL)
				{
					if (nec.startH > 4400 && nec.startL > 9000)
					{
						irNecStart();
					}
				}
				else
				{
					isError = true;
					nec.errCallback = irNecErrorHandler;
				}
				TIM2->CNT = 0;
			}
			else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
			{
				TIM2->CNT = 0;
			}
		break;

		case NEC_ADDR :
			nec.taskFlag = true;
			break;
		}
	}
}

bool irNecTask()
{
	if (irNecData())
	{
		irNecDecode();
	}

	return true;
}

bool irNecData()
{
	uint8_t i = 0;

	if (nec.taskFlag)
	{
		for (nec.cap_cnt=0; nec.cap_cnt < MAX_NEC_CNT; nec.cap_cnt++)
		{
			if (nec.edge_rising[nec.cap_cnt] < nec.edge_falling[nec.cap_cnt])
			{
				nec.raw_capture[nec.cap_cnt] = nec.edge_falling[nec.cap_cnt] - nec.edge_rising[nec.cap_cnt];
			}
			else
			{
				nec.raw_capture[nec.cap_cnt] = nec.edge_rising[nec.cap_cnt] - nec.edge_falling[nec.cap_cnt];
			}
		}

		for (nec.cap_cnt=1; nec.cap_cnt < MAX_NEC_CNT; nec.cap_cnt+=2)
		{
			if (nec.raw_capture[nec.cap_cnt] > 1600 && nec.raw_capture[nec.cap_cnt] < 1700)
			{
				nec.data[i] = 1;
			}
			else if (nec.raw_capture[nec.cap_cnt] > 500 && nec.raw_capture[nec.cap_cnt] < 600)
			{
				nec.data[i] = 0;
			}
			i++;
		}
		nec.taskFlag = false;
	}
	return false;
}
bool irNecDecode()
{
	
	return true;
}

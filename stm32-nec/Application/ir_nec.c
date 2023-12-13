/*
 * inputcapture.c
 *
 *  Created on: Dec 9, 2023
 *      Author: onste
 */

#include "ir_nec.h"

NEC nec =
{
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

void irNecInit()
{
	HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*)&nec.startH, 1);
	HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_2, (uint32_t*)&nec.startL, 1);
}

void irNecStart()
{
	nec.state = NEC_FORMAT;
	HAL_TIM_IC_Stop_DMA(&htim2, TIM_CHANNEL_1);
	HAL_TIM_IC_Stop_DMA(&htim2, TIM_CHANNEL_2);
	hdma_tim2_ch1.Init.Mode = DMA_CIRCULAR;
	hdma_tim2_ch2_ch4.Init.Mode = DMA_CIRCULAR;
	HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*)nec.edge_falling, MAX_NEC_CNT);
	HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_2, (uint32_t*)nec.edge_rising, MAX_NEC_CNT);
}

void	HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
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
						HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*)&nec.startH, 1);
						HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_2, (uint32_t*)&nec.startL, 1);
				}
				TIM2->CNT = 0;
			}
			else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
			{
				TIM2->CNT = 0;
			}
		break;

		case NEC_FORMAT :
			nec.taskFlag = true;
			break;
		}
	}
}

bool irNecTask()
{
	if (irNecData())
	return irNecDecode();
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
	return true;
}
bool irNecDecode()
{
	uint8_t i = 0;
	uint8_t j = 0;
	uint16_t check = 0;
	bool isEnd = false;
	while(!isEnd)
	{

		for (j=0; j<4; j++)
		{
			for (i=8*j; i<(8*j+8); i++)
			{
//				nec.decoded[j] = (i%8+1)<< nec.data[i];
				nec.decoded[j] <<= nec.data[i] << (i%8);
//				nec.decoded[j] <<= nec.data[i] << (8%(i+1));
//				nec.decoded[j] = (8%(i+1)) << nec.data[i];
			}
		}

		if ((nec.decoded[0]) == (~nec.decoded[1]) && (nec.decoded[2]) == (~nec.decoded[3]))
		{
			isEnd = true;
			break;
		}
		else
		{
			check++;
		}
	}
	return isEnd;
}

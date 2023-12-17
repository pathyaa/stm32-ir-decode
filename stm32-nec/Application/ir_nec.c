/*
 * inputcapture.c
 *
 *  Created on: Dec 9, 2023
 *      Author: onste
 */

#include "ir_nec.h"

static TIM_HandleTypeDef *used_timer = &htim2;
bool isInit = false;

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


bool irInit(IR_Format cur_state)
{
	switch(cur_state)
	{
		case NEC_INIT :
		{
			if (nec.state != cur_state)
				return false;

			uint32_t* p_check_header_low = (uint32_t*)&nec.startL;
			uint32_t* p_check_header_high = (uint32_t*)&nec.startH;

			HAL_StatusTypeDef check_ch1 = HAL_ERROR;
			HAL_StatusTypeDef check_ch2 = HAL_ERROR;

			check_ch1 = HAL_TIM_IC_Start_DMA(used_timer, TIM_CHANNEL_1, p_check_header_high, 1);
			check_ch2 = HAL_TIM_IC_Start_DMA(used_timer, TIM_CHANNEL_2, p_check_header_low, 1);

			return check_ch1 && check_ch2;
		}
		break;

		case NEC_FORMAT :
		{
			nec.state = NEC_FORMAT;
			HAL_StatusTypeDef check_ch1 = HAL_ERROR;
			HAL_StatusTypeDef check_ch2 = HAL_ERROR;

			hdma_tim2_ch1.Init.Mode = DMA_CIRCULAR;
			hdma_tim2_ch2_ch4.Init.Mode = DMA_CIRCULAR;

			check_ch1 = HAL_TIM_IC_Start_DMA(used_timer, TIM_CHANNEL_1, (uint32_t*)nec.edge_falling, MAX_NEC_CNT);
			check_ch2 = HAL_TIM_IC_Start_DMA(used_timer, TIM_CHANNEL_2, (uint32_t*)nec.edge_rising, MAX_NEC_CNT);

			return check_ch1 && check_ch2;
		}
		break;

		case NEC_END :
			return NEC_END;
		break;
	}

	return false;
}


void irReset()
{
	nec.state = NEC_INIT;
	nec.taskFlag = false;
	nec.cap_cnt = 0;
	nec.startL = 0;
	nec.startH = 0;
	memset(nec.edge_rising, 0, sizeof(uint16_t)*MAX_NEC_CNT);
	memset(nec.edge_falling, 0, sizeof(uint16_t)*MAX_NEC_CNT);
	memset(nec.raw_capture, 0, sizeof(uint16_t)*MAX_NEC_CNT);
	memset(nec.data, 0, MAX_NEC_CNT-1);
	memset(nec.decoded, 0, 4);
}

void irStart()
{
	if (used_timer->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
	{
		if (nec.startL)
		{
			if ((nec.startH > 4300 && nec.startH < 4700) && (nec.startL > 8800 && nec.startL < 9200))
			{
				irInit(NEC_FORMAT);
			}
		}
		else
		{
			HAL_TIM_IC_Start_DMA(used_timer, TIM_CHANNEL_1, (uint32_t*)&nec.startH, 1);
			HAL_TIM_IC_Start_DMA(used_timer, TIM_CHANNEL_2, (uint32_t*)&nec.startL, 1);
		}
		TIM2->CNT = 0;
	}
	else if (used_timer->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
	{
		TIM2->CNT = 0;
	}
}
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	static uint8_t format_idx=0;
	if (htim == &htim2)
	{
	switch (nec.state)
	{
		case NEC_INIT :
			irStart();
		break;

		case NEC_FORMAT :
//			format_idx++;
//			if (format_idx == 2)
//			{
				if (!nec.taskFlag)
				{
					nec.taskFlag = true;
				}
//			}
		break;
		}
	}
}
void irGetData()
{
	volatile uint8_t i = 0;

	if (nec.taskFlag)
	{
		for (nec.cap_cnt=0; nec.cap_cnt < MAX_NEC_CNT; nec.cap_cnt++)
		{
			if (nec.edge_rising[nec.cap_cnt] < nec.edge_falling[nec.cap_cnt])
			{
				nec.raw_capture[nec.cap_cnt] = nec.edge_falling[nec.cap_cnt] - nec.edge_rising[nec.cap_cnt];
			}
			else if (nec.edge_rising[nec.cap_cnt] > nec.edge_falling[nec.cap_cnt])
			{
				nec.raw_capture[nec.cap_cnt] = nec.edge_rising[nec.cap_cnt] - nec.edge_falling[nec.cap_cnt];
			}

			if (nec.raw_capture[nec.cap_cnt] > 1550 && nec.raw_capture[nec.cap_cnt] < 1750)
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
}

void irDataDecode()
{
	volatile int8_t i = 0;
	volatile int8_t j = 0;
	bool decodeEnd = false;
	while(!decodeEnd)
	{
		if (nec.decoded[0] == ~(nec.decoded[1]) && nec.decoded[2] == ~(nec.decoded[3]))
		{
			decodeEnd = true;
			break;
		}
		for (j=0; j<4; j++)
		{
			for (i=8*j; i<(8*j+8); i++)
			{
				nec.decoded[j] += (nec.data[i] << (8*j+7-i));
			}
		}
	}
}

bool irTask()
{
	if (!isInit)
	{
		irInit(NEC_INIT);
		isInit = true;
	}
	while (!nec.taskFlag);
	if (isInit)
	{
		irGetData();


			irDataDecode();

	}

}




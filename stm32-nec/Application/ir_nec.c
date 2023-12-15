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
	.check_start_low = 0,
	.check_start_high = 0,
	.edge_falling = {0,},
	.edge_rising = {0,},
	.raw_capture = {0,},
	.data = {0,},
	.state = NEC_INIT
};

void irInit()
{
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
}

void irStart(IR_Format check_format)
{
	if (check_format == NEC_FORMAT)
	{
		HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);
		HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_2);

		hdma_tim2_ch1.Init.Mode = DMA_CIRCULAR;
		hdma_tim2_ch2_ch4.Init.Mode = DMA_CIRCULAR;

		HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*)nec.edge_falling, MAX_NEC_CNT);
		HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_2, (uint32_t*)nec.edge_rising, MAX_NEC_CNT);

		nec.taskFlag = true;
	}
}

void irCheckStart()
{
	if (nec.state == NEC_INIT)
	{
		if (htim2.Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			nec.check_start_low = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1);
			TIM2->CNT = 0;// Reset Captured point (TIMER COUNTER)
		}
		else if (htim2.Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			nec.check_start_high = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_2);
			TIM2->CNT = 0; // Reset Captured point (TIMER COUNTER)
		}
	}

	if (nec.check_start_low > IR_HEADER_LOW - IR_HEADER_MARGIN && nec.check_start_low < IR_HEADER_LOW + IR_HEADER_MARGIN)
	{
		if (nec.check_start_high > IR_HEADER_HIGH - IR_HEADER_MARGIN && nec.check_start_high < IR_HEADER_HIGH + IR_HEADER_MARGIN)
		{
			nec.state = NEC_FORMAT;
			irStart(nec.state);
		}
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim2)
	{
		irCheckStart();
	}
}

bool irDecode()
{
	volatile uint8_t bit_data_idx = 0;
	volatile uint8_t decoded_idx = 0;
	nec.isEnd = false;

	if (nec.taskFlag)
	{
		for (nec.cap_cnt = 0; nec.cap_cnt < MAX_NEC_CNT; nec.cap_cnt++)
		{
			if (nec.edge_rising[nec.cap_cnt] < nec.edge_falling[nec.cap_cnt])
			{
				nec.raw_capture[nec.cap_cnt] = nec.edge_falling[nec.cap_cnt] - nec.edge_rising[nec.cap_cnt];
			}
			else
			{
				nec.raw_capture[nec.cap_cnt] = nec.edge_rising[nec.cap_cnt] - nec.edge_falling[nec.cap_cnt];
			}

			if (nec.raw_capture[nec.cap_cnt] > BIT1_TIME - BIT_TIME_MARGIN && 
			nec.raw_capture[nec.cap_cnt] < BIT1_TIME + BIT_TIME_MARGIN)
			{
				nec.data[bit_data_idx] = 1;
			}
			else if (nec.raw_capture[nec.cap_cnt] > BIT0_TIME - BIT_TIME_MARGIN && 
				nec.raw_capture[nec.cap_cnt] < BIT0_TIME + BIT_TIME_MARGIN)
			{
				nec.data[bit_data_idx] = 0;
			}

			bit_data_idx++;
		}
		nec.taskFlag = false;
	}

	
	while(!nec.isEnd)
	{
		for (decoded_idx = 0; decoded_idx < 4; decoded_idx++)
		{
			for (bit_data_idx = 8 * decoded_idx; bit_data_idx < (8 * decoded_idx + 8); bit_data_idx++)
			{
				nec.decoded[decoded_idx] += nec.data[bit_data_idx] << (8 * decoded_idx + 7 - bit_data_idx);
				
				if (nec.decoded[0] == ~(nec.decoded[1]) && nec.decoded[2] == ~(nec.decoded[3]))
				{
					nec.isEnd = true;
			 		return nec.isEnd;
				}
			}
		}
	}
}

void uartTX(uint32_t* decodeVal)
{
	uint8_t decoded_idx;

	for (decoded_idx = 0; decoded_idx < 4; decoded_idx++)
	{
		HAL_UART_Transmit_DMA(&huart2, (uint32_t*)decodeVal[decoded_idx], 1);
	}
}

void clearBuf(NEC *cur_nec)
{

}

bool ircheckDecoded()
{
	if (nec.isEnd)
	{
		uartTX(nec.decoded);
		clearBuf(&nec);
		return true;
	}
	else
	{
		return false;
	}
}

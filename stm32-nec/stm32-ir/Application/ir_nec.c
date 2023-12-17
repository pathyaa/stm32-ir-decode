
#include "ir_nec.h"

static TIM_HandleTypeDef *used_timer = &htim2;

PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

NEC nec =
{
	.repeatCheckH = {0,},
	.repeatCheckL = {0,},
	.isInit = false,
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


void irInit(IR_Format set_state)
{
	switch(set_state)
	{
		case NEC_INIT :

			uint32_t* p_check_header_low = (uint32_t*)&nec.startL;
			uint32_t* p_check_header_high = (uint32_t*)&nec.startH;

			HAL_TIM_IC_Start_DMA(used_timer, TIM_CHANNEL_1, p_check_header_high, 1);
			HAL_TIM_IC_Start_DMA(used_timer, TIM_CHANNEL_2, p_check_header_low, 1);

		break;

		case NEC_FORMAT :


			hdma_tim2_ch1.Init.Mode = DMA_CIRCULAR;
			hdma_tim2_ch2_ch4.Init.Mode = DMA_CIRCULAR;

			HAL_TIM_IC_Start_DMA(used_timer, TIM_CHANNEL_1, (uint32_t*)nec.edge_falling, MAX_NEC_PACKET_CNT);
			HAL_TIM_IC_Start_DMA(used_timer, TIM_CHANNEL_2, (uint32_t*)nec.edge_rising, MAX_NEC_PACKET_CNT);

		break;

		case NEC_REPEAT :
//
//			HAL_TIM_IC_Stop_DMA(used_timer, TIM_CHANNEL_1);
//			HAL_TIM_IC_Stop_DMA(used_timer, TIM_CHANNEL_2);
//
//			HAL_TIM_IC_Start_DMA(used_timer, TIM_CHANNEL_1, (uint32_t*)nec.repeatCheckL, 2);
//			HAL_TIM_IC_Start_DMA(used_timer, TIM_CHANNEL_2, (uint32_t*)nec.repeatCheckH, 2);

			break;
	}
}


void irReset()
{
	HAL_TIM_IC_Stop_DMA(used_timer, TIM_CHANNEL_1);
	HAL_TIM_IC_Stop_DMA(used_timer, TIM_CHANNEL_2);
	nec.state = NEC_INIT;
	nec.taskFlag = false;
	nec.cap_cnt = 0;
	nec.startL = 0;
	nec.startH = 0;
	nec.isInit = false;
	memset(nec.repeatCheckH, 0, sizeof(uint16_t) * 2);
	memset(nec.repeatCheckL, 0, sizeof(uint16_t) * 2);
	memset(nec.edge_rising, 0, sizeof(uint16_t) * MAX_NEC_PACKET_CNT);
	memset(nec.edge_falling, 0, sizeof(uint16_t) * MAX_NEC_PACKET_CNT);
	memset(nec.raw_capture, 0, sizeof(uint16_t) * MAX_NEC_PACKET_CNT);
	memset(nec.data, 0, MAX_NEC_PACKET_CNT-1);
	memset(nec.decoded, 0, 4);
	hdma_tim2_ch1.Init.Mode = DMA_NORMAL;
	hdma_tim2_ch2_ch4.Init.Mode = DMA_NORMAL;

	irInit(NEC_INIT);
	nec.isInit = true;

}

void irStart()
{
	if (used_timer->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
	{
		if (nec.startL)
		{
			if (nec.startH > 4300 && nec.startH < 4700 && nec.startL > 8800 && nec.startL < 9200)
			{
				nec.state = NEC_FORMAT;
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
	if (htim == &htim2)
	{
		switch (nec.state)
		{
			case NEC_INIT :
				irStart();
			break;

			case NEC_FORMAT :
				nec.taskFlag = true;
			break;
		}
	}
}
void irGetData()
{
	volatile uint8_t i = 0;

	for (nec.cap_cnt=0; nec.cap_cnt < MAX_NEC_PACKET_CNT; nec.cap_cnt++)
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

}


void irDataDecode()
{
	volatile int8_t i = 0;
	volatile int8_t j = 0;

	bool decodeEnd = false;

	while(!decodeEnd)
	{
		if ((nec.decoded[0] == ~(nec.decoded[1])) && (nec.decoded[2] == ~(nec.decoded[3])))
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

void irShowDecoded()
{
	for (uint8_t txIdx=0; txIdx < 4; txIdx++)
	{
		printf("decoded[%d] : 0x%x\r\n", txIdx, (uint8_t)nec.decoded[txIdx]);
	}

}

void irTask()
{
	// Task Lock : only go into isInit == false

	if (nec.taskFlag && nec.isInit)
	{
		irGetData();
		irDataDecode();
//		irInit(NEC_REPEAT);
//		HAL_UART_Receive_DMA(&huart2, rxBuf, 1);
		irShowDecoded();
		nec.isInit = false;
		irReset();
	}
	else
	{
		irReset();
	}
}




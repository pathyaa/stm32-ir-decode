#include "NEC.h"

typedef struct 
{
	uint16_t header[2];
    uint16_t rawTimerData[32];
	uint16_t repeat[2];
    uint8_t  decoded[4];
} NEC;

NEC user_nec;
static uint16_t irRaw[34];
uint8_t decode[4];
static TIM_HandleTypeDef *nec_timer = &htim2;
bool check_repeat;
bool get = false;

int _write(int file, char* p, int len){
	HAL_UART_Transmit(&huart2, p, len, 10);
	return len;
}

void necInit(void)
{
	HAL_TIM_IC_Start_DMA(nec_timer, TIM_CHANNEL_1, (uint32_t*)irRaw, 34);
	printf("Wait for IR (NEC) Signal\r\n");
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim2)
    {
    	check_repeat = true;
    	get = true;
    }
}

void necParsing(void)
{
	uint8_t irIdx = 0;

	uint16_t head = 0;
	
	head = irRaw[1];
	printf("head : %d\r\n", head);
	if (head > 13000 && head < 14000)
	{
		for (irIdx=2; irIdx<34; irIdx++)
		{
			if (irRaw[irIdx] > 2200 && irRaw[irIdx] < 2300)
			{
				decode[(irIdx-2)/8] += 1<<((irIdx-2) % 8);
			}
			else if (irRaw[irIdx] > 1100 && irRaw[irIdx] < 1200)
			{
				decode[(irIdx-2)/8] += 0<<((irIdx-2) % 8);
			}
		}

	}


	if (decode[0] == (int8_t)~decode[1] && decode[2] == (int8_t)~decode[3])
	{
		printf("success\r\n");

	}
	else
	{
		printf("fail\r\n");
	}
	printf("decode[0] : 0x%x\r\n", decode[0]);
	printf("decode[1] : 0x%x\r\n", decode[1]);
	printf("decode[2] : 0x%x\r\n", decode[2]);
	printf("decode[3] : 0x%x\r\n", decode[3]);

	HAL_TIM_IC_Start_DMA(nec_timer, TIM_CHANNEL_1, (uint32_t*)irRaw, 34);
	printf("restart!\r\n");
}






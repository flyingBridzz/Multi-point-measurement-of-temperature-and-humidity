#include "dht22.h"
#include "tim.h"
#include "usart.h"
#include "communication.h"

extern uint8_t Usart3_Rx;

void DHT22_DATA_OUT(void)
{
	GPIOG->CRL = (GPIOG->CRL|0x00003000);
	GPIOG->CRL = (GPIOG->CRL&0xFFFF3FFF);
}

void DHT22_DATA_IN(void)
{
	GPIOG->CRL = (GPIOG->CRL|0x00008000);
	GPIOG->CRL = (GPIOG->CRL&0xFFFF8FFF);
}

void DHT22_Init(void)
{
	Delay_us_ON();
	HAL_TIM_Base_Start_IT(&htim7);
	HAL_TIM_Base_Start_IT(&htim3);
}

void Rst_DHT22(void)
{
	DHT22_DATA_OUT();
	DHT22_DOWN;
	Delay_us_tim6(500);
	DHT22_UP;
	Delay_us_tim6(30);
	DHT22_DATA_IN();
	while((GPIOG->IDR&GPIO_PIN_3)==GPIO_PIN_3);//等待低电平
	while((GPIOG->IDR&GPIO_PIN_3)==0x00);//等待高电平
	while((GPIOG->IDR&GPIO_PIN_3)==GPIO_PIN_3);//等待低电平
}

uint8_t Read_Byte_DHT22(void)
{
	uint8_t i,data=0,time;
	for(i=0;i<8;i++)
	{
		while((GPIOG->IDR&GPIO_PIN_3)==0x00);//等待高电平
		TIM7->CNT = 0;
		while((GPIOG->IDR&GPIO_PIN_3)==GPIO_PIN_3);//等待低电平
		time = TIM7->CNT;
		data<<=1;
		if(time>50){
			data++;
		}
	}
	return data;
}

uint8_t Read_Data(uint8_t*buffer)
{
	uint8_t i,SumError,SUM;
	HAL_UART_AbortReceive_IT(&huart3);
	HAL_TIM_Base_Stop_IT(&htim3);
	Rst_DHT22();
	for(i=0,SUM=0;i<5;i++)
	{
		buffer[i] = Read_Byte_DHT22();
		if(i<4)
		{
			SUM += buffer[i];
		}		
	}
	if(SUM == buffer[4]){
		SumError = 0;
	}else{
		SumError = 1;
	}
	HAL_UART_Receive_IT(&huart3, &Usart3_Rx, 1);
	HAL_TIM_Base_Start_IT(&htim3);
	return SumError;
}

float Get_Temperature(uint8_t*buffer)
{
	uint16_t intermediate = 0;
	float Temperature = 0;
	intermediate = buffer[2];
	intermediate = intermediate<<8U;
	intermediate |= buffer[3];
	Temperature = (intermediate & (0x7FFF));
	Temperature /= 10;
	if((intermediate & 0x8000) == 0x8000)
	{
		Temperature = 0-Temperature;
	}
	return Temperature;
}

float Get_Humidity(uint8_t*buffer)
{
	uint16_t intermediate = 0;
	float Humidity = 0;
	intermediate = buffer[0];
	intermediate = intermediate<<8U;
	intermediate |= buffer[1];
	Humidity = (intermediate & (0x7FFF));
	Humidity /= 10;
	if((intermediate & 0x8000) == 0x8000)
	{
		Humidity = 0-Humidity;
	}
	return Humidity;
}

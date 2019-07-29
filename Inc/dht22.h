#ifndef _DHT22_H_
#define _DHT22_H_

#include "main.h"

#define	DHT22_UP		(GPIOG->BSRR = GPIO_PIN_3)
#define	DHT22_DOWN	(GPIOG->BSRR = GPIO_PIN_3<<16U)

void DHT22_DATA_OUT(void);
void DHT22_DATA_IN(void);

void DHT22_Init(void);
uint8_t Read_Data(uint8_t*buffer);

float Get_Temperature(uint8_t*buffer);
float Get_Humidity(uint8_t*buffer);

#endif 

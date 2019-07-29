#ifndef _MY_IIC_H_
#define _MY_IIC_H_

#include "main.h"

#define GPIOxIIC	GPIOB
#define GPIO_PIN_SCL	GPIO_PIN_8
#define GPIO_PIN_SDA	GPIO_PIN_9

#define IIC_SCL_High	(GPIOxIIC->BSRR = GPIO_PIN_SCL)
#define IIC_SCL_Low		(GPIOxIIC->BSRR = (uint32_t)GPIO_PIN_SCL << 16U)
#define IIC_SDA_High 	(GPIOxIIC->BSRR = GPIO_PIN_SDA)
#define IIC_SDA_Low		(GPIOxIIC->BSRR = (uint32_t)GPIO_PIN_SDA << 16U)


void IIC_Start(void);
void IIC_Stop(void);
void Write_IIC_Command(unsigned char IIC_Command);
void Write_IIC_Data(unsigned char IIC_Data);
void Write_IIC_Byte(unsigned char IIC_Byte);

#endif

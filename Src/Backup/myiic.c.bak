#include "myiic.h"
#include "tim.h"
#include "gpio.h"

//»Ìº˛IIC≥ı ºªØ
/**********************************************
//IIC Start
**********************************************/
void IIC_Start(void)
{
   IIC_SCL_High;
   Delay_us_tim6(1);		
   IIC_SDA_High;
   Delay_us_tim6(1);
   IIC_SDA_Low;
   Delay_us_tim6(1);
   IIC_SCL_Low;
	 Delay_us_tim6(1);
}

/**********************************************
//IIC Stop
**********************************************/
void IIC_Stop(void)
{
   IIC_SCL_Low;
   Delay_us_tim6(1);
   IIC_SDA_Low;
   Delay_us_tim6(1);
   IIC_SCL_High;
   Delay_us_tim6(1);
   IIC_SDA_High;
   Delay_us_tim6(1);
}
/**********************************************
// IIC Write byte
**********************************************/
void Write_IIC_Byte(unsigned char IIC_Byte)
{
	unsigned char i;
	for(i=0;i<8;i++)		
	{
		if(IIC_Byte & 0x80)		//1?0?
		IIC_SDA_High;
		else
		IIC_SDA_Low;
		Delay_us_tim6(1);
		IIC_SCL_High;
		Delay_us_tim6(1);
		IIC_SCL_Low;
		Delay_us_tim6(1);
		IIC_Byte<<=1;			//loop
	}
	IIC_SDA_High;
	Delay_us_tim6(1);
	IIC_SCL_High;
	Delay_us_tim6(1);
	IIC_SCL_Low;
	Delay_us_tim6(1);
}
/**********************************************
// IIC Write Command
**********************************************/
void Write_IIC_Command(unsigned char IIC_Command)
{
   IIC_Start();
   Write_IIC_Byte(0x78);            //Slave address,SA0=0
   Write_IIC_Byte(0x00);			//write command
   Write_IIC_Byte(IIC_Command); 
   IIC_Stop();
}
/**********************************************
// IIC Write Data
**********************************************/
void Write_IIC_Data(unsigned char IIC_Data)
{
   IIC_Start();
   Write_IIC_Byte(0x78);			
   Write_IIC_Byte(0x40);			//write data
   Write_IIC_Byte(IIC_Data);
   IIC_Stop();
}




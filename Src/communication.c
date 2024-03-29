#include "communication.h"
#include "usart.h"
#include "tim.h"
#include "dht22.h"
#include "oled.h"

volatile struct Circle_FIFO SaveBlock = {0};
volatile struct ComTable Others[ComTableMax] = {0};
volatile struct ComTable Mine = {
	0x01,			//正在被使用
	0x03,			//地址
	0x00,			//显示本台机器所连接的机器总数
	{0x00,0x00,0x00,0x00,0x00},
	0x01,			//数据未刷新
	0x00,			//组网模式
	0x00,			//主机模式
	{0x00,0x00,0x00},//{Host,Slave,Data}
};
volatile struct Command_FIFO mail = {0};
uint8_t Usart3_Rx;
uint8_t Frame[20]={0};
float sumH = 0,sumT = 0;

extern volatile uint32_t cnt1ms;
extern volatile uint32_t cnt5ms;
extern volatile uint32_t cnt10ms;
extern volatile uint32_t cnt50ms;
extern volatile uint32_t cnt100ms;
extern volatile uint32_t cnt500ms;
extern volatile uint32_t cnt1000ms;
extern volatile uint32_t cnt2000ms;

void Scheduler(void)
{
	uint8_t i,j,k;
	uint8_t data[5]={0};
	uint8_t tmp[4]={0,0,0,0};
	int average;
	if(cnt1ms > 0)
	{
		cnt1ms = 0;
		if((GPIOE->IDR & GPIO_PIN_4) == 0x00)
		{
			Delay_us_tim6(499);
			if((GPIOE->IDR & GPIO_PIN_4) == 0x00)
			{			
				if(Mine.Host == 1){
					Mine.Host = 0;
				}else if(Mine.Host == 0){
					Mine.Host = 1;
				}
				ClearComTableOthers();
				while((GPIOE->IDR & GPIO_PIN_4) == 0x00);
			}
		}else if((GPIOE->IDR & GPIO_PIN_6) == 0x00){
			Delay_us_tim6(499);
			if((GPIOE->IDR & GPIO_PIN_6) == 0x00)
			{
				if(Mine.Net == 1){
					Mine.Net = 0;
				}else if(Mine.Net == 0){
					Mine.Net = 1;
				}
				ClearComTableOthers();
				while((GPIOE->IDR & GPIO_PIN_6) == 0x00);
			}
		}
	}
	if(cnt2000ms>1999)
	{
		cnt2000ms = 0;
		if(Read_Data(data)==0)
		{
			Mine.Data[0] = data[0];
			Mine.Data[1] = data[1];
			Mine.Data[2] = data[2];
			Mine.Data[3] = data[3];
			Mine.Data[4] = data[4];
		}
		if(Mine.Net == 1)
		{
			if(Mine.Host == 0)
			{
				if((Others[0].use == 1)&&(Others[0].Host == 1))
				{
					SendOneFrame(Mine.Data,Others[0].addr,5);
					Others[0].WAC[2]++;
					if(Others[0].WAC[2]>2)
					{
						SendOneFrame(Mine.Data,Others[0].addr,5);
						SendOneFrame(Mine.Data,Others[0].addr,5);
						SendOneFrame(Mine.Data,Others[0].addr,5);
					}
					if(Others[0].WAC[2]>5)
					{
						Others[0].use = 0;
						Others[0].addr = 0;
						Others[0].Host = 0;
						Others[0].Net = 0;
						Others[0].Newdata = 0;
						Others[0].WAC[0] = 0;
						Others[0].WAC[1] = 0;
						Others[0].WAC[2] = 0;
						Mine.MaCnt = 0;
						
					}
				}
			}else if(Mine.Host == 1){
				for(i=0,j=0,k=0,sumH=0,sumT=0;j<Mine.MaCnt;i++)
				{
					if(Others[i].use == 1)
					{
						j++;
						if(Others[i].Newdata == 1)
						{
							k++;
							sumH+=Get_Humidity(Others[i].Data);
							sumT+=Get_Temperature(Others[i].Data);
							Others[i].Newdata = 0;
							Others[i].WAC[2] = 0;
						}else{
							Others[i].WAC[2]++;
						}
						if(Others[i].WAC[2] > 5)
						{
							Others[i].use = 0;
							Others[i].Newdata = 0;
							Others[i].Host = 0;
							Others[i].Net = 0;
							Others[i].addr = 0;
							Others[i].WAC[2] = 0;
							Others[i].WAC[1] = 0;
							Others[i].WAC[0] = 0;
							Mine.MaCnt--;
						}
					}
				}
				sumT += Get_Temperature(Mine.Data);
				sumH += Get_Humidity(Mine.Data);
				//温度
				average = (int)((sumT/(k+1))*10);
				if(average < 0)
				{
					Mine.Data[2] = 0x80;
					average = 0-average;
				}else{
					Mine.Data[2] = 0x00;
				}
				Mine.Data[2] |= ((average & 0xff00)>>8);
				Mine.Data[3] = (average & 0xff);
				//湿度
				average = (int)((sumH/(k+1))*10);
				if(average < 0)
				{
					Mine.Data[0] = 0x80;
					average = 0-average;
				}else{
					Mine.Data[0] = 0x00;
				}
				Mine.Data[0] |= ((average & 0xff00)>>8);
				Mine.Data[1] = (average &0xff);
				//计算校验和
				Mine.Data[4] = (uint8_t)(Mine.Data[0]+Mine.Data[1]+Mine.Data[2]+Mine.Data[3]);
			}
		}
	}
	
	if(cnt5ms>4)
	{
		cnt5ms = 0;
		if(Mine.Net == 1)
		{
			if(ReceiveOneFrame(Frame))//接收函数修改完成可以正常接收帧了
			{
				ReceiveOrder(Frame);
				ReceiveData(Frame);
			}
		}
	}
	
	if(cnt10ms>9)
	{
		cnt10ms = 0;
		if(Mine.Net == 1)
		{
			if(mail.NowCnt>0)
			{
				PopOrder(tmp);
				if(Mine.Host == 0){
					if(tmp[2] == HostPlease)
					{
						if(Others[0].use == 0)
						{
							SendAck(tmp[1],HostPlease);
							Others[0].use = 1;
							Others[0].addr = tmp[1];
							Others[0].Host = 1;
							Others[0].Net = 1;
							Others[0].Newdata = 0;
							Others[0].WAC[0] = 0;
							Others[0].WAC[1] = 0;
							Others[0].WAC[2] = 0;
							Mine.MaCnt = 1;
						}else{
							if(Others[0].addr == tmp[1])
							{
								SendAck(tmp[1],HostPlease);
							}
						}
					}else if(tmp[2] == Ack){
						if(tmp[3] == DataPlease)
						{
							if(Others[0].use == 1)
							{
								Others[0].WAC[2] = 0;
							}
						}
					}
				}else if(Mine.Host == 1){
					if(tmp[2] == SlavePlease)
					{
						for(i=0,j=0;j<Mine.MaCnt;i++)
						{
							if(Others[i].use == 1)
							{
								j++;
								if(Others[i].addr == tmp[1])
								{
									//重新连接
									Others[i].use = 1;
									Others[i].addr = tmp[1];
									Others[i].Host = 0;
									Others[i].Net = 1;
									Others[i].Newdata = 0;
									Others[i].WAC[0]++;
									SendHostPlease(tmp[1]);
									if(Others[i].WAC[0]>5)
									{
										SendHostPlease(tmp[1]);
										SendHostPlease(tmp[1]);
										SendHostPlease(tmp[1]);
										Others[i].WAC[0] = 0;
									}
									break;
								}
							}
						}
						if(Others[i].use == 0)//没有搜索到，需要建立连接
						{
							for(j=0;Others[j].use == 1;j++);
							//连接
							Others[j].use = 1;
							Others[j].addr = tmp[1];
							Others[j].Host = 0;
							Others[j].Net = 1;
							Others[j].Newdata = 0;
							Others[j].WAC[0] = 1;
							Others[j].WAC[2] = 0;
							Mine.MaCnt++;
							SendHostPlease(tmp[1]);
						}
					}else if(tmp[2] == Ack){
						if(tmp[3] == HostPlease)
						{
							for(i=0,j=0;j<Mine.MaCnt;i++)
							{
								if(Others[i].use == 1)
								{
									j++;
									if(Others[i].addr == tmp[1])
									{
										Others[i].WAC[0] = 0;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	if(cnt50ms>49)
	{
		cnt50ms = 0;
		if(Mine.Net == 1)
		{
			if(Mine.Host == 0)
			{
				if(Mine.MaCnt == 0)
				{
					SendSlavePlease(BroadcastAddress);
				}
			}
		}
	}
	
	if(cnt1000ms>999)
	{
		cnt1000ms = 0;
		if(Mine.Net == 1)
		{
			if((Mine.Host == 1)&&(Mine.MaCnt > 0))
			{
				SendOneFrame(Mine.Data,BroadcastAddress,5);
			}
		}
	}
	
	if(cnt100ms>99)
	{
		cnt100ms = 0;
		
	}
	
	if(cnt500ms>499)
	{
		cnt500ms = 0;
		if(Mine.Net == 1)
		{
			if(Mine.Host == 0)
			{
				if((Others[0].use == 1)&&(Others[0].Newdata == 1))
				{
					Mine.Data[0] = Others[0].Data[0];
					Mine.Data[1] = Others[0].Data[1];
					Mine.Data[2] = Others[0].Data[2];
					Mine.Data[3] = Others[0].Data[3];
					Mine.Data[4] = Others[0].Data[4];
				}
			}
		}
		
		OLED_Clear();
		OLED_ShowString(20,1,"T:");
		OLED_ShowFloat(40,1,Get_Temperature(Mine.Data),16);
		OLED_ShowString(86,1,"C");		
		OLED_ShowString(20,3,"H:");
		OLED_ShowFloat(40,3,Get_Humidity(Mine.Data),16);
		OLED_ShowString(86,3,"%RH");
		if(Mine.Host == 1)
		{
			OLED_ShowString(0,6,"Host");
		}else if(Mine.Host == 0){
			OLED_ShowString(0,6,"Slave");
		}
		if(Mine.Net == 1)
		{
			OLED_ShowString(100,6,"Net");
		}else if(Mine.Net == 0){
			OLED_ShowString(80,6,"Single");
		}
	}
	
	
}

void ClearComTableOthers(void)
{
	uint8_t i,j;
	for(i=0,j=0;j<Mine.MaCnt;i++)
	{
		if(Others[i].use == 1)
		{
			j++;
			Others[i].addr = 0;
			Others[i].Host = 0;
			Others[i].Net = 0;
			Others[i].Newdata = 0;
			Others[i].use = 0;
			Others[i].WAC[0] = 0;
			Others[i].WAC[1] = 0;
			Others[i].WAC[2] = 0;
		}
	}
	Mine.MaCnt = 0;
}

uint8_t GetByte(void)
{
	uint8_t tmp;
	if(SaveBlock.FIFO_Cnt>0)
	{
		tmp = SaveBlock.FIFO[SaveBlock.GetP];
		SaveBlock.GetP++;
		if(SaveBlock.GetP >= FIFO_MAX)
		{
			SaveBlock.GetP = 0;
		}
		SaveBlock.FIFO_Cnt--;
	}else{
		tmp = 0;
	}
	return tmp;
}

void PullByte(uint8_t data)
{
	if(SaveBlock.FIFO_Cnt<=FIFO_MAX)
	{
		SaveBlock.FIFO[SaveBlock.PullP] = data;
		SaveBlock.PullP++;
		if(SaveBlock.PullP >= FIFO_MAX)
		{
			SaveBlock.PullP = 0;
		}
		SaveBlock.FIFO_Cnt++;
	}
}

void PopOrder(uint8_t *order)
{
	order[0] = mail.FIFO[mail.Pop].Gaddr;
	order[1] = mail.FIFO[mail.Pop].Saddr;
	order[2] = mail.FIFO[mail.Pop].Order;
	if(order[2] == Ack)
	{
		order[3] = mail.FIFO[mail.Pop].OrderGoal;
	}else{
		order[3] = 0x00;
	}
	mail.Pop++;
	if(mail.Pop >= ComMax)
	{
		mail.Pop = 0;
	}
		mail.NowCnt--;
}

void PushOrder(uint8_t *frame)
{
	if(mail.NowCnt<=ComMax)
	{
		mail.FIFO[mail.Push].Gaddr = frame[1];
		mail.FIFO[mail.Push].Saddr = frame[2];
		mail.FIFO[mail.Push].Order = frame[3];
		if(frame[3] == Ack)
		{
			mail.FIFO[mail.Push].OrderGoal = frame[4];
		}else{
			mail.FIFO[mail.Push].OrderGoal = 0x00;
		}
		mail.Push++;
		if(mail.Push >= ComMax)
		{
			mail.Push = 0;
		}
		mail.NowCnt++;
	}
}

uint8_t SendByte(uint8_t MessageByte)
{
	if((USART3->SR&0x0080) == 0x0080)
	{
		USART3->DR = MessageByte;
		return 0;//数据发送完成
	}else{
		return 1;//数据未发送
	}
}

void SendMessage(uint8_t *Message,uint8_t len)
{
	uint8_t i;
	for(i=0;i<len;i++)
	{
		while(SendByte(Message[i]));
	}		
}

void ReceiveOrder(uint8_t *frame)
{
	if(mail.NowCnt<ComMax)
	{
		if((frame[0]<=5)&&((frame[3] == HostPlease)||(frame[3] == SlavePlease)||(frame[3] == Ack)))
		{
			PushOrder(frame);
		}
	}
}

void ReceiveData(uint8_t *frame)
{
	uint8_t i,j;
	if((frame[0]>5)&&((uint8_t)(frame[3]+frame[4]+frame[5]+frame[6]) == frame[7]))
	{
		for(i=0,j=0;j<Mine.MaCnt;i++)
		{
			if(Others[i].use == 1)
			{
				j++;
				if(Others[i].addr == frame[2])
				{
					Others[i].Data[0] = frame[3];
					Others[i].Data[1] = frame[4];
					Others[i].Data[2] = frame[5];
					Others[i].Data[3] = frame[6];
					Others[i].Data[4] = frame[7];
					Others[i].Newdata = 1;
					if(Mine.Host == 1)
					{
						SendAck(frame[2],DataPlease);
					}
				}
			}
		}
	}
}

/*应答信号：起始信号+目的地址+源地址+应答字节+所应答的目标命令*/
void SendAck(uint8_t GA,uint8_t AckGoal)
{
	uint8_t tmp[5]={StartByte,GA,SourceAddress,Ack,AckGoal};
	SendMessage(tmp,5);
}

/*主机请求信号：起始信号+目的地址+源地址+主机请求字节*/
void SendHostPlease(uint8_t GA)
{
	uint8_t tmp[4]={StartByte,GA,SourceAddress,HostPlease};
	SendMessage(tmp,4);
}

/*从机请求信号：起始信号+目的地址+源地址+从机请求字节*/
void SendSlavePlease(uint8_t GA)
{
	uint8_t tmp[4]={StartByte,GA,SourceAddress,SlavePlease};
	SendMessage(tmp,4);
}

/*数据请求信号：起始信号+目的地址+源地址+主机请求字节*/
void SendDataPlease(uint8_t GA)
{
	uint8_t tmp[4]={StartByte,GA,SourceAddress,DataPlease};
	SendMessage(tmp,4);
}


/*帧格式：起始信号+目的地址+源地址+信息+结束信号*/
void SendOneFrame(uint8_t *Message,uint8_t GoalAddress,uint8_t len)
{
	uint8_t tmp[20]={0};
	uint8_t i,cnt;
	tmp[0] = StartByte;
	tmp[1] = GoalAddress;
	tmp[2] = SourceAddress;
	for(i=0,cnt=3;i<len;i++)
	{
		if((Message[i]==StartByte)||(Message[i]==StopByte)||(Message[i]==Ack)||(Message[i]==transfer)||(Message[i]==HostPlease)||(Message[i]==DataPlease)||(Message[i]==SlavePlease))
		{
			tmp[cnt] = transfer;
			cnt++;
		}
		tmp[cnt] = Message[i];
		cnt++;
	}
	tmp[cnt] = StopByte;
	cnt++;
	SendMessage(tmp,cnt);
}
//返回1：接收成功
//返回0：接收失败
//接收到的数据格式：帧长+目的地址+源地址+信息
uint8_t ReceiveOneFrame(uint8_t *RxMessage)
{
	uint8_t tmp;
	uint8_t i;
	tmp = GetByte();
	if(tmp == StartByte)
	{
		tmp = GetByte();
		if((tmp == SourceAddress)||(tmp == BroadcastAddress))
		{
			RxMessage[1] = tmp;
			tmp = GetByte();
			RxMessage[2] = tmp;
			tmp = GetByte();
			if((tmp == HostPlease)||(tmp == DataPlease)||(tmp == SlavePlease))
			{
				RxMessage[3] = tmp;
				RxMessage[0] = 4;
				return 1;
			}else if(tmp == Ack){
				RxMessage[3] = tmp;
				tmp = GetByte();
				RxMessage[4] = tmp;
				RxMessage[0] = 5;
				return 1;
			}else{
				for(i=3;(i<20)&&(tmp!=StopByte);i++)
				{
					if(tmp == transfer)
					{
						tmp = GetByte();
					}else if(tmp == Ack){
						return 0;
					}else if(tmp == StartByte){
						return 0;
					}else if(tmp == HostPlease){
						return 0;
					}else if(tmp == DataPlease){
						return 0;
					}else if(tmp == SlavePlease){
						return 0;
					}
					RxMessage[i] = tmp;
					tmp = GetByte();
				}
				if(tmp == StopByte)
				{
					RxMessage[0] = i;
					return 1;
				}else{
					return 0;
				}
			}
		}
		return 0;
	}
	return 0;
}

//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//  /* Prevent unused argument(s) compilation warning */
//  UNUSED(GPIO_Pin);
//  /* NOTE: This function Should not be modified, when the callback is needed,
//           the HAL_GPIO_EXTI_Callback could be implemented in the user file
//   */
//	if(GPIO_Pin == GPIO_PIN_4)
//	{
//		if(Mine.Host == 1){
//			Mine.Host = 0;
//		}else if(Mine.Host == 0){
//			Mine.Host = 1;
//		}
//	}else if(GPIO_Pin == GPIO_PIN_6){
//		if(Mine.Net == 1){
//			Mine.Net = 0;
//		}else if(Mine.Net == 0){
//			Mine.Net = 1;
//		}
//	}
//}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
  /* NOTE: This function Should not be modified, when the callback is needed,
           the HAL_UART_RxCpltCallback could be implemented in the user file
   */
	if(huart->Instance == USART3)
	{
		if(SaveBlock.FIFO_Cnt < FIFO_MAX)
		{
			PullByte(Usart3_Rx);
		}
		HAL_UART_Receive_IT(&huart3, &Usart3_Rx, 1);
	}
}

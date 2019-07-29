#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_
/*
当前问题：从机部分已经调整完成，主机部分未修改
需要将温度数据读取放定时器中断中读取
*/
#include "main.h"

#define		StartByte						0xfe
#define		StopByte						0xfd
#define		Ack									0xaa
#define		HostPlease					0xbb
#define		SlavePlease					0xcc
#define		DataPlease					0xdd
#define		transfer						0x43

#define		SourceAddress				(Mine.addr)
#define		BroadcastAddress		0xff
#define		FIFO_MAX						100
#define		ComMax							100
#define		HostPleaseScan			20
#define		DataPleaseScan			20
#define		SendDataScan				20
#define		WaitDataPleaseScan	40
#define		AckScan							6
#define		ComTableMax					5

struct Circle_FIFO{
	uint8_t PullP;
	uint8_t GetP;
	uint8_t FIFO_Cnt;
	uint8_t FIFO[FIFO_MAX];
};

struct ComTable{
	uint8_t use;
	uint8_t addr;
	uint8_t MaCnt;
	uint8_t Data[5];
	uint8_t Newdata;
	uint8_t Net;
	uint8_t Host;
	uint8_t WAC[3];
};

struct Command{
	uint8_t Saddr;
	uint8_t Gaddr;
	uint8_t Order;
	uint8_t OrderGoal;
};

struct Command_FIFO{
	uint8_t Push;
	uint8_t Pop;
	uint8_t NowCnt;
	struct Command FIFO[ComMax];
};

void Scheduler(void);
void ClearComTableOthers(void);
uint8_t SendByte(uint8_t MessageByte);
void SendMessage(uint8_t *Message,uint8_t len);
void SendOneFrame(uint8_t *Message,uint8_t GoalAddress,uint8_t len);
uint8_t ReceiveOneFrame(uint8_t *RxMessage);
void ReceiveOrder(uint8_t *frame);
void ReceiveData(uint8_t *frame);
void PopOrder(uint8_t *order);
void PushOrder(uint8_t *frame);
void SendAck(uint8_t GA,uint8_t AckGoal);
void SendHostPlease(uint8_t GA);
void SendSlavePlease(uint8_t GA);
void SendDataPlease(uint8_t GA);

#endif

#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include "main.h"

#define TX_ERR		0x120

#define TX_CHANGE_RANGE	0x600
#define TX_RESPONSE			0x280
#define TX_CMD		0x422 + HARDWARE_NUMBER
#define TX_DATA		0x722 + HARDWARE_NUMBER

#define RX_CMD		0x180
#define RX_DATA		0x222 + HARDWARE_NUMBER



#define FACTORS					0x93	//поправочные коэфициенты
#define DEADTIME				0x94	//мёртвое время
#define EFFICIENCY			0x95	//эффективное кол-во открытых счётчиков
#define SENSIVITY				0x96	//чувствительность
#define RANGE						0x97	//предельная скорость счёта для переключения
#define SAVE_PARAMETRS	0xE0
/*
#define TIMESPAN							0x40	//время накопления
#define PERIOD_PULSE_POWER		0x80
#define PULSE_DURATION_POWER	0x81
#define WORKING_MODE					0x83
#define CURRENT_RANGE					0x90
#define SLIDER_SIZE_1					0x91
#define SLIDER_SIZE_2					0x92
#define SLIDER_SIZE_3					0x93
#define DEADTIME_1						0x94
#define DEADTIME_2						0x95
#define DEADTIME_3						0x96
#define RANGE_1_TO_2					0x97
#define RANGE_2_TO_3					0x98
#define RANGE_3_MAX						0x99
#define RANGE_3_TO_2					0x9A
#define RANGE_2_TO_1					0x9B

#define CORR_FACTOR_1_1				0xA1
#define CORR_FACTOR_1_2				0xA2
#define CORR_FACTOR_1_3				0xA3
#define CORR_FACTOR_1_4				0xA4

#define CORR_FACTOR_2_1				0xB1
#define CORR_FACTOR_2_2				0xB2

#define CORR_FACTOR_3_1				0xC1
#define CORR_FACTOR_3_2				0xC2
*/

typedef enum
{
	E_SAVE_STATE,
	E_NO_SAVE_STATE
}eSaveSate;

typedef enum
{
	E_COMPLETED_RESPONSE = 0x00,	//завершаем процесс передачи данных
	E_ACCEPT_RESPONSE = 0x01,			//получиил подтверждение о приему запроса на передачу
	E_NEWDATA_RESPONSE = 0x03			//новые данные
}eResponseState;

typedef  struct
{
	uint8 		frmVer;			//X.X - 4bits
	uint8 		prVer;			//X.X	-	4bits
	uint32		serialNum;	//HEX
}sServiceMsg;

void Protocol_Initialization(void);
void Protocol_ReceiveMsgCallback(uint16	canID, uint8 *pData);
void Protocol_TransmitMsgCallback(uint16 canID);

void Protocol_StartMsg(void);
void Protocol_HeaderDataMsg(uint16 counterRate,float averageDose);
void Protocol_CountersMsg(uint16	*pData);
void Protocol_DoseRateMsg(float doseValue);

void Protocol_SetRangeValue(uint8);

void Protocol_ResponseDataPackaje(eResponseState responseState);

uint8 Protocol_GetRangeValue(void);


#endif

#ifndef _DEBUG_H
#define _DEBUG_H

#include "main.h"

#define DEBUG_CAN_TX	0x7E0

typedef struct
{
	unsigned uRangeNumber	:2;
	unsigned uQueryFull		:1;
	unsigned uAlgorithm		:1;
	unsigned uState				:1;
	unsigned uReserved		:3;
}tFlag;

typedef struct
{
	uint32	uSerialNumber;
	uint16	uSensorData[12];
	uint16	uSortSensorData[12];
	float		fCurrentDose;
	tFlag		uFlag;
}tDebugData;



void Debug_Initialization(void);
void Debug_Process(void);
void Debug_Start(void);
void Debug_Stop(void);

void Debug_SetCurrentDose(float);
void Debug_SetRangeNumber(uint8);
void Debug_SetQueryFull(uint8);
void Debug_SetAlgorithmType(uint8);
void Debug_SetCurrentSensorsData(uint8*);
void Debug_SetSortSensorsData(uint8*);

uint8 *Debug_CreatePacket(uint8	cmd,uint8 *pData,uint8 dataSize,uint8 *packSize);
#endif

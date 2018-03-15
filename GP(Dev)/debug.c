#include "debug.h"
#include "can.h"

static tDebugData	debugData;

void Debug_Initialization()
{
	memset(&debugData,0,sizeof(tDebugData));
	
	debugData.uSerialNumber = SerialNumberBD;
	
		T6CON = 0x0006;
		T6IC = 0x0069;
		T6UD = 1;
		T6 = 0x6769;
	
	
		Debug_Stop();
	
}

void Debug_Process() interrupt T6IC_VEC
{
	uint8	packSize = 0;
	uint8	*pPackage = 0;
	
	debugData.uFlag.uState = T0R;
	debugData.uFlag.uRangeNumber = HARDWARE_NUMBER;
		pPackage = Debug_CreatePacket(0x01,(uint8*)&debugData,sizeof(tDebugData),&packSize);
	
	if(pPackage){
		
		CAN_SendDataPackage(DEBUG_CAN_TX + HARDWARE_NUMBER,pPackage,packSize);
		
		free(pPackage);
	}
	T6 = 0x6769;
}

void Debug_Start()
{
	T6R = 1;
}

void Debug_Stop()
{
	T6R = 0;
}

uint8 *Debug_CreatePacket(uint8	cmd,uint8 *pData,uint8 dataSize,uint8 *packSize)
{
	uint8	bufferSize = (uint8)(5 + dataSize);
	uint8	*pBuf = (uint8*)malloc(sizeof(uint8) * bufferSize);
	
	uint16	ks = 0,i = 0;
	
	if(pBuf)
	{
		pBuf[0] = 0xA5;
		pBuf[1] = bufferSize;
		pBuf[2] = cmd;
		memcpy((pBuf + 3),pData,dataSize);

			for(i = 0;i<bufferSize - 2;i++)
				ks+=pBuf[i];
		
		memcpy((pBuf + (bufferSize - 2)),&ks,2);
		
		*packSize = bufferSize;
	

	}
	return pBuf;
}

void Debug_SetCurrentDose(float fDose)
{
	debugData.fCurrentDose = fDose;
}
void Debug_SetRangeNumber(uint8 uRangeNumber)
{
	debugData.uFlag.uRangeNumber = (uRangeNumber & 0x3);
}
void Debug_SetQueryFull(uint8 uFull)
{
	debugData.uFlag.uQueryFull = uFull;
}
void Debug_SetAlgorithmType(uint8 uType)
{
	debugData.uFlag.uAlgorithm = uType;
}
void Debug_SetCurrentSensorsData(uint8 *pData)
{
	memcpy(debugData.uSensorData,pData,sizeof(uint16) * 12);
}
void Debug_SetSortSensorsData(uint8* pData)
{
	memcpy(debugData.uSortSensorData,pData,sizeof(uint16) * 12);
}


#include "protocol.h"
#include "can.h"
#include "settings.h"
#include "process.h"
#include "system.h"

uint16	timeSpan;			//время измерения (в десятых долях сек)
uint8		rangeValue;		//номер текущего диапазона (0 - авто)

const uint8 mResetResponse[] = {0x42};
const uint8	mErrorResponse[] = {0x90};

//команда включения диапазона, первый байт - номер диапазона, второй байт - статус(включить - выключить)
const uint8	mStartRangeAuto2[]	= {0x02,0x01,0x01};
const uint8	mStartRangeAuto3[]	= {0x03,0x01,0x01};

const uint8	mStartRangeManual2[]	= {0x02,0x01,0x00};
const uint8	mStartRangeManual3[]	= {0x03,0x01,0x00};


const uint8	mStopRange2[]	= {0x02,0x00};
const uint8	mStopRange3[]	= {0x03,0x00};



static uint8	dataMsgCounter = 0;



void Protocol_Initialization()
{
	rangeValue = 0;
	timeSpan = 10;
	
}

void Protocol_SetRangeValue(uint8	Value)
{
	rangeValue = Value;
}

uint8 Protocol_GetRangeValue()
{
	return rangeValue;
}

void Protocol_TransmitMsgCallback(uint16	canID)
{
	
}
//отправка сообщения с параметром
void Protocol_SendDataMsg(uint8*	pData,uint8	*pValue,uint8 dataSize)
{
	uint8	pIndex = pData[1];
	
		memcpy(pData + 4,(uint32*)(pValue),dataSize);
	
	pData[1] = pIndex;
	pData[2] = 0;
	
	CAN_SendMessage(TX_CMD,pData,8);
}
//получение параметра от ПО
void Protocol_GetDataMsg(uint8*	pValue,	uint8	*pData,uint8	dataSize,eSaveSate saveState)
{
	memcpy(pValue,pData + 4,dataSize);
		if(saveState == E_SAVE_STATE)SETTINGS_Save();
	
	Protocol_SendDataMsg(pData,(uint8*)pValue,dataSize);
}
//отправка синхронизационного сообщения
void Protocol_ResponseDataPackaje(eResponseState responseState)
{
	uint8	pData[2];
				pData[0] = 0x40;
				pData[1] = (uint8)responseState;
	
		CAN_SendMessage(TX_DATA,pData,sizeof(pData));
}
//подтверждение остановки приема данных
void Protocol_ConfirmStopData()
{
	uint8	pData[2];
		pData[0] = 0x41;
		pData[1] = 0;
	
	CAN_SendMessage(TX_DATA,pData,sizeof(pData));
}
void Protocol_StartMsg()
{
	CAN_SendMessage(TX_RESPONSE,mResetResponse,1);
}

//Заголовочное сообщение данных
void Protocol_HeaderDataMsg(uint16 counterRate,float averageDose)
{	
	uint8	pData[8];

	dataMsgCounter = 0;
	memset(pData,0,sizeof(pData));

		pData[0] = 0x40;
		pData[1] = 0xFF;
		pData[2] = (uint8)(counterRate & 0x00FF);
		pData[3] = (uint8)(counterRate >> 8);

		memcpy(pData + 4,&averageDose,sizeof(float));
	
	CAN_SendMessage(TX_DATA,pData,sizeof(pData));
}
void Protocol_CountersMsg(uint16	*pData)
{
	CAN_SendMessage(TX_DATA,(uint8*)pData,8);
}
void Protocol_DoseRateMsg(float doseValue)
{
	uint8	pData[8];
		
		memcpy(pData,&doseValue,sizeof(float));
		memset(pData + 4,(uint8)0xFF,sizeof(float));
	
	CAN_SendMessage(TX_DATA,pData,sizeof(pData));
}


void Protocol_ReceiveMsgCallback(uint16	canID, uint8 *pData)
{
	switch(canID)
	{
			//обработка команды сброса для ведомой платы
		#ifndef MAIN_RANGE
			case RX_CMD:
			{
				switch(pData[0])
				{
					case 0x42:  System_Reset(); break;
				}
			}break;
		#endif
			//остальные команды только для главной
		#ifdef MAIN_RANGE	
			case RX_CMD:	
			{
				switch(pData[0])
				{
					case 0x40:
					{
						//автоматический режим
						switch(pData[1])
						{
							case 0:
							{
								Process_SetAutoMode(AUTO_ON);	//включаем авторежим для 3 диапазона
								Process_Start(); //запускаем измерения в 3 диапазоне
								
									CAN_SendMessage(TX_RESPONSE,pData,2);
							}break;
							
							case 2:
							{
								Process_SetAutoMode(AUTO_OFF);	//мануальный режим
								CAN_SendMessage(TX_CHANGE_RANGE,mStartRangeManual2,3);
								
									CAN_SendMessage(TX_RESPONSE,pData,2);
							}break;
							
							case 3:
							{
								Process_SetAutoMode(AUTO_OFF);	//мануальный режим
								Process_Start(); 
								
									CAN_SendMessage(TX_RESPONSE,pData,2);
							}break;
						}
					}break;
					
					case 0x41:
					{ 
				
							CAN_SendMessage(TX_CHANGE_RANGE,mStopRange2,2);
						Process_Stop(); 
						CAN_SendMessage(TX_RESPONSE,pData,1);
					}break;
					
					
				}
 			}break;
		#endif
			
		
			//принимаем сообщение по ид смены диапазона
		case TX_CHANGE_RANGE:
		{
			switch(pData[0])
			{
				#ifdef MAIN_RANGE	
					case 3:
					{
						//устанавливаем авторежим для диапазона приёмника
						if(pData[2] == 1)//принята команда авторежима
							Process_SetAutoMode(AUTO_ON);
						else
							Process_SetAutoMode(AUTO_OFF);
						
						//включаем/выключаем диапазон приёмник
						if(pData[1] == 1)
							Process_Start();
						else
							Process_Stop();
					}break;
				#else
					case 2:
					{
						//устанавливаем авторежим для диапазона приёмника
						if(pData[2] == 1)//принята команда авторежима
							Process_SetAutoMode(AUTO_ON);
						else
							Process_SetAutoMode(AUTO_OFF);
						
						//включаем/выключаем диапазон приёмник
						
						if(pData[1] == 1)
							Process_Start();
						else
							Process_Stop();
					}break;
				#endif
			}
		}break;
			
		case RX_DATA:	
		{
			switch(pData[0])
			{
				
				case 0x04:	//запись параметров
				{
					switch(pData[1])
					{
						case FACTORS:			Protocol_GetDataMsg((uint8*)&MainSettings.correctionFactors[pData[3]],pData,sizeof(float),E_NO_SAVE_STATE);	break;
						case DEADTIME: 		Protocol_GetDataMsg((uint8*)&MainSettings.mDeadTime,pData,sizeof(MainSettings.mDeadTime),E_NO_SAVE_STATE); break;
						case EFFICIENCY: 	Protocol_GetDataMsg((uint8*)&MainSettings.mEfficiency,pData,sizeof(MainSettings.mEfficiency),E_NO_SAVE_STATE); break;
						case SENSIVITY: 	Protocol_GetDataMsg((uint8*)&MainSettings.mSensivity,pData,sizeof(MainSettings.mSensivity),E_NO_SAVE_STATE); break;
						case RANGE: 			Protocol_GetDataMsg((uint8*)&MainSettings.rangeLimits,pData,sizeof(MainSettings.rangeLimits),E_NO_SAVE_STATE); break;
						
						
						case SAVE_PARAMETRS:
						{
							if(pData[3] == 0xFF)
								SETTINGS_Save();
							if(pData[3] == 0x00){
								SETTINGS_Default();
								SETTINGS_Save();
							}
							CAN_SendMessage(TX_CMD,pData,8);
						}break;
					}
				}break;
				
				case 0x05:	//запрос
				{
					switch(pData[1])
					{
						case FACTORS:			Protocol_SendDataMsg(pData,(uint8*)&MainSettings.correctionFactors[pData[3]],sizeof(float));		break;
						case DEADTIME:		Protocol_SendDataMsg(pData,(uint8*)&MainSettings.mDeadTime,sizeof(MainSettings.mDeadTime));			break;
						case EFFICIENCY:	Protocol_SendDataMsg(pData,(uint8*)&MainSettings.mEfficiency,sizeof(MainSettings.mEfficiency));	break;
						case SENSIVITY:		Protocol_SendDataMsg(pData,(uint8*)&MainSettings.mSensivity,sizeof(MainSettings.mSensivity));		break;
						case RANGE:				Protocol_SendDataMsg(pData,(uint8*)&MainSettings.rangeLimits,sizeof(MainSettings.rangeLimits));	break;
						
					}
				}break;
				
				
			}
		}break;
	}
/*
	if(devID == 0)	//пришло сообщение для всех БД
	{
		if(pData[0] == 0x01)
		{	
			CAN_SendMessage(TX_CMD,mResetResponse,8);
		}
	}
	else if(devID == DEVICE_NUMBER)	//получили месседж для конкретного блока
	{
		switch(pData[0])//проверяем индекс команды
		{
			case 0x04:	//выдать параметр
			{
				switch(pData[1])
				{
				
					case FACTORS:
					{
						if(pData[3]>=0 && pData[3] < FACTORS_COUNT)
							Protocol_SendDataMsg((uint8*)&MainSettings.correctionFactors[pData[3]],pData,sizeof(float));
					}break;
					
					case DEADTIME: 		Protocol_SendDataMsg(pData,(uint8*)&MainSettings.mDeadTime,sizeof(MainSettings.mDeadTime));				break;
					case EFFICIENCY:	Protocol_SendDataMsg(pData,(uint8*)&MainSettings.mEfficiency,sizeof(MainSettings.mEfficiency));		break;
					case SENSIVITY:		Protocol_SendDataMsg(pData,(uint8*)&MainSettings.mSensivity,sizeof(MainSettings.mSensivity));			break;
					case RANGE:				Protocol_SendDataMsg(pData,(uint8*)&MainSettings.rangeLimits,sizeof(MainSettings.rangeLimits));		break;
					
					default: 
					{
						pData[0] = 0xEE;
						pData[2] = pData[1];
						pData[1] = 0x04;
						CAN_SendMessage(TX_ERROR,pData,3);
					}break;
					
				}
			}break;
			
			case 0x05:	//задать параметр
			{
				switch(pData[1])
				{
					

					default: 
					{
						pData[0] = 0xEE;
						pData[2] = pData[1];
						pData[1] = 0x05;
							CAN_SendMessage(TX_ERROR,pData,3);
					}break;	
				}
				
				
			}break;
			
			case 0x40:	//старт измерений
			{
				Protocol_ResponseDataPackaje(E_ACCEPT_RESPONSE);
				Process_Start();
				
				
			}break;
			
			case 0x41:	//стоп измерений
			{
				
				Protocol_ConfirmStopData();
				Process_Stop();
				//Protocol_ResponseDataPackaje(E_COMPLETED_RESPONSE);
				
				
			}break;
		}
	}
	else
	{
		
	}
*/
}

#include "process.h"
#include "protocol.h"
#include "settings.h"
#include "can.h"

uint8		firstMeasureCounter = 0;
uint8		sliderCounter = 0;
uint16	sensorCnt[SENSORS];
float		sliderQuery[SLIDER_SIZE];

eAutoModeState	autoMode;

void Process_SetAutoMode(eAutoModeState State)
{
	autoMode = State;
}
uint8	Process_GetAutoModeStatus()
{
	return autoMode;
}

void	Process_Initializetion(void)
{
	CC8IC = 0x0070;
	CC9IC = 0x0071;
	CC10IC = 0x0072;
	CC11IC = 0x0073;
	CC12IC = 0x0074;
	CC13IC = 0x0075;
	CC14IC = 0x0076;
	CC15IC = 0x0077;
	
	T2IC = 0x0078;
	T3IC = 0x0079;
	T4IC = 0x007A;
	T5IC = 0x007B;
	memset(sensorCnt,0,sizeof(sensorCnt));
	memset(sliderQuery,0,sizeof(sliderQuery));
	
	autoMode = AUTO_OFF;
}

/*
	Обработка импульса счётчика
	1. Инкрементируем счётчик текущего канала
	2. Снимаем питание  на счётчиках на 100us
*/

void Process_EXIN1() 	interrupt CC8IC_VEC
{
	
	T7 = 0xFFFF;
	sensorCnt[0]++;
}
void Process_EXIN2()	interrupt CC9IC_VEC
{
	T7 = 0xFFFF;
	sensorCnt[1]++;
	
}
void Process_EXIN3()	interrupt CC10IC_VEC
{
	T7 = 0xFFFF;
	sensorCnt[2]++;
}
void Process_EXIN4()	interrupt CC11IC_VEC
{
	T7 = 0xFFFF;
	sensorCnt[3]++;
}
void Process_EXIN5()	interrupt CC12IC_VEC
{
	T7 = 0xFFFF;
	sensorCnt[4]++;
}
void Process_EXIN6()	interrupt CC13IC_VEC
{
	T7 = 0xFFFF;
	sensorCnt[5]++;
}
void Process_EXIN7()	interrupt CC14IC_VEC
{
	T8 = 0xFFFF;
	sensorCnt[6]++;
}
void Process_EXIN8()	interrupt CC15IC_VEC
{
	T8 = 0xFFFF;
	sensorCnt[7]++;
}
void Process_EXIN9()	interrupt T5IC_VEC
{
	T8 = 0xFFFF;
	
	sensorCnt[8]++;
}
void Process_EXIN10()	interrupt T4IC_VEC
{
	T8 = 0xFFFF;
	
	sensorCnt[9]++;
}
void Process_EXIN11()	interrupt T3IC_VEC
{
	T8 = 0xFFFF;
	
	sensorCnt[10]++;
}
void Process_EXIN12()	interrupt T2IC_VEC
{
	T8 = 0xFFFF;
	
	sensorCnt[11]++;
}

void	Process_MainProc()	interrupt T0IC_VEC
{
	 uint8	*pSendPackage;
	 uint8	mStartRangeAuto2[]	= {0x02,0x01,0x01};
	 uint8	mStartRangeAuto3[]	= {0x03,0x01,0x01};
/*	float currentDose = 0;
	
	currentDose = Process_CalculateDoseRate();
	
	
	Protocol_HeaderDataMsg();
	Protocol_CounterMsg(sensorCnt);
	Protocol_DoseRateMsg(currentDose);
	*/
	//Protocol_ResponseDataPackaje(E_NEWDATA_RESPONSE);
	uint8	i = 0;
	
	uint16	rangeMin = (uint16)(MainSettings.rangeLimits & 0xFFFF);
	uint16	rangeMax = (uint16)( (MainSettings.rangeLimits >> 16) & 0xFFFF);
	
	
	float currentDose = 0;
	float averageDose = 0;
	float counterRate = 0;
	uint16	uCntRate = 0;
		currentDose = Process_CalculateCurrentDoseRate();
		counterRate = Process_CalculateCountingRate();
	
	uCntRate = counterRate;
	
	if(Process_GetAutoModeStatus() == AUTO_ON)
	{
	
		//отработка переключения диапазона
		#ifdef MAIN_RANGE
			//если скорость счёта меньше нижнего предела, то останавливаем работу и включаем нижний диапазон
			if((counterRate <= rangeMin) && firstMeasureCounter >= 2){
				
				Process_Stop();
				CAN_SendMessage(TX_CHANGE_RANGE,mStartRangeAuto2,3);
			}
		#else
			//если скорость счёта выше верхнего предела, то включаем верхний диапазон
			if(counterRate >= rangeMax){
				
				Process_Stop();
				CAN_SendMessage(TX_CHANGE_RANGE,mStartRangeAuto3,3);
			}
		#endif
		}
	
	if(sliderCounter<SLIDER_SIZE){
		sliderQuery[sliderCounter] = currentDose;
		sliderCounter++;
	}
	
	else{
		memcpy(&sliderQuery[0],&sliderQuery[1],sizeof(float) * (SLIDER_SIZE - 1));
		sliderQuery[SLIDER_SIZE - 1] = currentDose;
	}
	for(i = 0;i<SLIDER_SIZE;i++)
		averageDose += sliderQuery[i];
	
	averageDose = averageDose * 0.5f;
	
	//формируем пакет данных на отправку
	/*
		4 сообщения по 8 байт = 32 байта
	
		1 - заголовок 
	  2 - 4 - данные счетчиков
	*/
	pSendPackage = (uint8*)malloc(sizeof(uint8) * 32);
	
	if(pSendPackage){
		pSendPackage[0] = 0x40;
		pSendPackage[1] = 0xFF;
		
		memcpy(pSendPackage + 2,&uCntRate,sizeof(uint16));
		memcpy(pSendPackage + 4,&averageDose,sizeof(float));
		
		memcpy(pSendPackage + 8,sensorCnt,sizeof(uint16) * SENSORS);
		
		CAN_SendDataPackage(TX_DATA,pSendPackage,32);
	}
	
	
	free(pSendPackage);
	/*
	Protocol_HeaderDataMsg(counterRate,averageDose);
	Protocol_CountersMsg(sensorCnt);
	Protocol_CountersMsg(sensorCnt + 4);
	Protocol_CountersMsg(sensorCnt + 8);
	*/
	
	memset(sensorCnt,0,sizeof(sensorCnt));//обнуляем массив счётчиков
}

float	Process_CalculateCurrentDoseRate()
{
	
	
	uint16	i = 0;
	float	numerator = 0;
	float	denominator = 0;
	float	dose = 0;
	
	float leftPart = 0;
	float rightPart = 0;
	
	//проверка на максимальную скорость счёта в массиве счётчиков
	#ifdef MAIN_RANGE	
		uint16	rangeMax = (uint16)( (MainSettings.rangeLimits >> 16) & 0xFFFF);
	
		for(i = 0;i<SENSORS;i++)
			if(sensorCnt[i] > rangeMax)sensorCnt[i] = rangeMax;
	#endif
	
		leftPart = 1.0f / (MainSettings.mSensivity * MainSettings.mEfficiency * 1);
			
			for(i = 0;i<SENSORS;i++)
			{
				numerator = sensorCnt[i] * MainSettings.correctionFactors[i];
				denominator = 1.0f - (sensorCnt[i] * MainSettings.mDeadTime);
				
				rightPart = rightPart + (numerator / denominator);
			}
	dose = leftPart * rightPart;
	
	return dose;
}
float	Process_CalculateCountingRate()
{
	float	cntRate = 0;
	uint16	i = 0;
	
		for(i = 0;i<SENSORS;i++)
				cntRate += sensorCnt[i];
			cntRate = cntRate / (float)(MainSettings.mEfficiency /* SENSORS*/);	//06.03.17 - корректировка формулы рассчёта средней скрорости счёта
	
	if(firstMeasureCounter < 2)
		firstMeasureCounter++;
	
	return cntRate;
}
void Process_Start()
{
	Process_Stop();
	
	sliderCounter = 0;
	firstMeasureCounter = 0;
	
	memset(sensorCnt,0,sizeof(uint16) * SENSORS);
	memset(sliderQuery,0,sizeof(float) * SLIDER_SIZE);
	/*
	Алгоритм запуска измерений
	
		- проверяем текущий заданных диапазон. Если это 1,2,3 - то запускаем измерение в соответствующем диапазоне
		-
	*/
	
	
	T0 = 0xFFFF;
	T7R = 1;
	T8R = 1;
	
	HV_STATE = 1;
	//TO_DO ожидаем нарастание высокого напряжения
	
	
	T0R = 1;
	

}

void Process_Stop()
{
	//PWM OFF
	T7R = 0;
	T8R = 0;
	//MAINTIMER OFF
	T0R = 0;
	//H-Bridge off
		KEY11_L = 0; KEY11_H = 0;
		KEY12_L = 0; KEY12_H = 0;
	HV_STATE = 0;
	
	
}

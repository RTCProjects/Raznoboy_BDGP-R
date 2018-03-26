#include "process.h"
#include "protocol.h"
#include "settings.h"
#include "can.h"
#include "debug.h"

const uint16	tempSensorData_1[] = {29,22,31,1,0,0,2,1,0,2,18,20};

uint8		firstMeasureCounter = 0;
uint8		sliderCounter = 0;
uint8		sliderMode = 0;
uint16	sensorCnt[SENSORS];
uint16	sensorCntCopy[SENSORS];

float		sliderQuery[SLIDER_SIZE];

eAutoModeState	autoMode;
eExternalRange	extRange;

void 	Process_SetAutoMode(eAutoModeState State){autoMode = State;}
uint8	Process_GetAutoModeStatus(){return autoMode;}
void	Process_SetExternalRange(eExternalRange ExtRange){extRange = ExtRange;}
uint8 Process_GetExternalRange(){return extRange;}

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
	memset(sensorCnt,0,sizeof(uint16) * SENSORS);
	memset(sensorCntCopy,0,sizeof(uint16) * SENSORS);
	
	memset(sliderQuery,0,sizeof(float) * SLIDER_SIZE);
	
	autoMode = AUTO_OFF;
	extRange = EXT_OFF;
	
	sliderMode = 0;
}

/*
	Обработка импульса счётчика
	1. Инкрементируем счётчик текущего канала
	2. Снимаем питание  на счётчиках на 100us
*/

void Process_EXIN1() 	interrupt CC8IC_VEC
{
	
	T7 = 0xFFFF;
	sensorCntCopy[0]++;
}
void Process_EXIN2()	interrupt CC9IC_VEC
{
	T7 = 0xFFFF;
	sensorCntCopy[1]++;
	
}
void Process_EXIN3()	interrupt CC10IC_VEC
{
	T7 = 0xFFFF;
	sensorCntCopy[2]++;
}
void Process_EXIN4()	interrupt CC11IC_VEC
{
	T7 = 0xFFFF;
	sensorCntCopy[3]++;
}
void Process_EXIN5()	interrupt CC12IC_VEC
{
	T7 = 0xFFFF;
	sensorCntCopy[4]++;
}
void Process_EXIN6()	interrupt CC13IC_VEC
{
	T7 = 0xFFFF;
	sensorCntCopy[5]++;
}
void Process_EXIN7()	interrupt CC14IC_VEC
{
	T8 = 0xFFFF;
	sensorCntCopy[6]++;
}
void Process_EXIN8()	interrupt CC15IC_VEC
{
	T8 = 0xFFFF;
	sensorCntCopy[7]++;
}
void Process_EXIN9()	interrupt T5IC_VEC
{
	T8 = 0xFFFF;
	
	sensorCntCopy[8]++;
}
void Process_EXIN10()	interrupt T4IC_VEC
{
	T8 = 0xFFFF;
	
	sensorCntCopy[9]++;
}
void Process_EXIN11()	interrupt T3IC_VEC
{
	T8 = 0xFFFF;
	
	sensorCntCopy[10]++;
}
void Process_EXIN12()	interrupt T2IC_VEC
{
	T8 = 0xFFFF;
	
	sensorCntCopy[11]++;
}

void	Process_MainProc()	interrupt T0IC_VEC
{
	 uint8	*pSendPackage;
	
	 uint8	mStartRangeAuto1[]	= {0x01,0x01,0x01};
	 uint8	mStartRangeAuto2[]	= {0x02,0x01,0x01};
	 uint8	mStartRangeAuto3[]	= {0x03,0x01,0x01};
/*	float currentDose = 0;
	
	currentDose = Process_CalculateDoseRate();
	
	
	Protocol_HeaderDataMsg();
	Protocol_CounterMsg(sensorCnt);
	Protocol_DoseRateMsg(currentDose);
	*/
	//Protocol_ResponseDataPackaje(E_NEWDATA_RESPONSE);
	uint8	i = 0,j = 0;
	
	uint16	rangeMin = (uint16)(MainSettings.rangeLimits & 0xFFFF);
	uint16	rangeMax = (uint16)( (MainSettings.rangeLimits >> 16) & 0xFFFF);
	uint16	rangeMinExt = (uint16)(MainSettingsExt.rangeLimits & 0xFFFF);
	uint16	rangeMaxExt = (uint16)( (MainSettingsExt.rangeLimits >> 16) & 0xFFFF);

	float currentDose = 0;
	float averageDose = 0;
	float counterRate = 0;
	uint16	uCntRate = 0;
	
	uint8	sliderMax = 2;	//параметр динамического заполнения движка
	 
	memcpy((uint8*)sensorCnt,(uint8*)sensorCntCopy,sizeof(uint16) * SENSORS);
	
	/*
	Тестовые данные для проверки МД
	*/
	memcpy((uint8*)sensorCnt,(uint8*)tempSensorData_1,sizeof(uint16) * SENSORS);
	

	#ifdef RANGE3 //для 38-х счётчиков расчёт по другой формуле
		if(Process_GetExternalRange() == EXT_ON)
			currentDose = Process_CalculateLogDoseRate();
		else
			currentDose = Process_CalculateMaximumSumDoseRate();
		
			Debug_SetAlgorithmType(1);
	#else 
	  //для 21-х расчёт ведется по двум формулам, в зависимости от порога
		currentDose = Process_CalculateCurrentDoseRate();
		Debug_SetAlgorithmType(0);
		if(currentDose >= DOSERATE_LIMIT){
			currentDose = Process_CalculateMaximumSumDoseRate();
			Debug_SetAlgorithmType(1);
		}
		/*if(currentDose >= DOSERATE_LIMIT){
			currentDose = Process_CalculateMaximumSumDoseRate();
			Debug_SetAlgorithmType(1);
		}*/
	#endif
	
		//currentDose = Process_CalculateCurrentDoseRate();
		counterRate = Process_CalculateCountingRate();
	
	uCntRate = counterRate;
	
	Debug_SetCurrentDose(currentDose);
	Debug_SetCurrentSensorsData((uint8*)sensorCnt);

	if(Process_GetAutoModeStatus() == AUTO_ON)
	{
	
		//отработка переключения диапазона
		#ifdef RANGE3
			//если включен расширенный режим (4 диапазон)
			if(Process_GetExternalRange())
			{
				//понижаем до 3его
				if((counterRate <= rangeMinExt) && firstMeasureCounter >= FIRST_MEASURE_TIME){
					Process_Stop();
					Process_SetExternalRange(EXT_OFF);
					Process_Start();
				}
				if(counterRate >= rangeMaxExt){
					//тут проверка на 20 секунд TODO
				}
			}
			else
			{
				//если скорость счёта меньше нижнего предела, то останавливаем работу и включаем нижний диапазон
				if((counterRate <= rangeMin) && firstMeasureCounter >= FIRST_MEASURE_TIME){
					
					Process_Stop();
					CAN_SendMessage(TX_CHANGE_RANGE,mStartRangeAuto2,3);
				}
				if(counterRate >= rangeMaxExt){
					Process_Stop();
					Process_SetExternalRange(EXT_ON);
					Process_Start();
				}
			}
		

		#endif
			
		#ifdef RANGE2
			if((counterRate <= rangeMin) && firstMeasureCounter >= FIRST_MEASURE_TIME){
				
				Process_Stop();
				CAN_SendMessage(TX_CHANGE_RANGE,mStartRangeAuto1,3);
			}
			if(counterRate >= rangeMax){
				Process_Stop();
				CAN_SendMessage(TX_CHANGE_RANGE,mStartRangeAuto3,3);
			}
		#endif
			
		#ifdef RANGE1
			if(counterRate >= rangeMax){
				Process_Stop();
				CAN_SendMessage(TX_CHANGE_RANGE,mStartRangeAuto2,3);
			}
		#endif			
		}
	
	
	//постоянный движок в 2с. только для 2 3 и 4 диапазонов		
	#ifndef RANGE1
		if(sliderCounter<SLIDER_SIZE){
			sliderQuery[sliderCounter] = currentDose;
			sliderCounter++;
			
			Debug_SetQueryFull(0);
		}
		
		else{
			memcpy(&sliderQuery[0],&sliderQuery[1],sizeof(float) * (SLIDER_SIZE - 1));
			sliderQuery[SLIDER_SIZE - 1] = currentDose;
			
			Debug_SetQueryFull(1);
		}
		for(i = 0;i<SLIDER_SIZE;i++)
			averageDose += sliderQuery[i];
		
		averageDose = averageDose * 0.5f;
	#else
		//динамический двидок для 1 диапазона
		//if(currentDose < SLIDER_30_MAX_LIM){
		if(counterRate <= 3){
			sliderMax = 30;
			if(sliderMode == 2 || sliderMode == 3){		
				sliderCounter = 30;
				
				if(sliderMode == 3)firstMeasureCounter = 0;
					
				//memset(sliderQuery,0,sizeof(float) * SLIDER_SIZE);
			}

			if(sliderMode != 1)sliderCounter = 0;
			sliderMode = 1;
		}
		//else if(currentDose >= SLIDER_10_MIN_LIM && currentDose < SLIDER_10_MAX_LIM){
		else if(counterRate >= 2.5f && counterRate <= 12){
			sliderMax = 10;
			if(sliderMode == 3){
					sliderCounter = 10;
					//memset(sliderQuery,0,sizeof(float) * SLIDER_SIZE);
			}
			if(sliderMode != 2)sliderCounter = 0;
			sliderMode = 2;
		}
		//else if(currentDose >= SLIDER_2_MIN_LIM) {
		else if(counterRate >= 10){
			sliderMax = 2;
			
			if(sliderMode != 3)sliderCounter = 0;
			sliderMode = 3;
		}
		
		if(sliderCounter<sliderMax){
		sliderQuery[sliderCounter] = currentDose;
		sliderCounter++;
		}
	
		else{
			memcpy(&sliderQuery[0],&sliderQuery[1],sizeof(float) * (sliderMax - 1));
			sliderQuery[sliderMax - 1] = currentDose;
		}
		
		for(i = 0;i<sliderCounter;i++)
			averageDose += sliderQuery[i];
		
		
		sliderCounter ? averageDose = averageDose / sliderCounter : currentDose;
	#endif
	
	//формируем пакет данных на отправку
	/*
		4 сообщения по 8 байт = 32 байта
	
		1 - заголовок 
	  2 - 4 - данные счетчиков
	*/
	pSendPackage = (uint8*)malloc(sizeof(uint8) * 32);
	
	if(pSendPackage /*&& (firstMeasureCounter >= FIRST_MEASURE_TIME)*/){
		pSendPackage[0] = 0x40;
		pSendPackage[1] = 0xFF;
		
		memcpy(pSendPackage + 2,&uCntRate,sizeof(uint16));
		memcpy(pSendPackage + 4,&averageDose,sizeof(float));
		
		memcpy(pSendPackage + 8,sensorCnt,sizeof(uint16) * SENSORS);
		
		#ifdef RANGE3
			if(Process_GetExternalRange() == EXT_ON)
				CAN_SendDataPackage(TX_DATA_EXT,pSendPackage,32);
			else
				CAN_SendDataPackage(TX_DATA,pSendPackage,32);
		#else
				CAN_SendDataPackage(TX_DATA,pSendPackage,32);
		#endif
	}
	
	
	free(pSendPackage);
	/*
	Protocol_HeaderDataMsg(counterRate,averageDose);
	Protocol_CountersMsg(sensorCnt);
	Protocol_CountersMsg(sensorCnt + 4);
	Protocol_CountersMsg(sensorCnt + 8);
	*/
	
	memset(sensorCntCopy,0,sizeof(uint16) * SENSORS);//обнуляем массив счётчиков
}

//фия расчёта МД по формуле ЧТЗ
float	Process_CalculateCurrentDoseRate()
{
	
	
	uint16	i = 0;
	float	numerator = 0;
	float	denominator = 0;
	float	dose = 0;
	
	float leftPart = 0;
	float rightPart = 0;
	/*
		uint16	rangeMax = (uint16)( (MainSettings.rangeLimits >> 16) & 0xFFFF);
		uint16	rangeMaxExt = (uint16)( (MainSettingsExt.rangeLimits >> 16) & 0xFFFF);
	//проверка на максимальную скорость счёта в массиве счётчиков
	#ifdef RANGE3	
		uint16	rangeMax = (uint16)( (MainSettings.rangeLimits >> 16) & 0xFFFF);
	
		for(i = 0;i<SENSORS;i++)
			if(sensorCnt[i] > rangeMax)sensorCnt[i] = rangeMax;
	#endif
	
	#ifdef RANGE3	
		if(Process_GetExternalRange() == EXT_ON)
			for(i = 0;i<SENSORS;i++)if(sensorCnt[i] > rangeMax)sensorCnt[i] = rangeMaxExt;
		else
			for(i = 0;i<SENSORS;i++)if(sensorCnt[i] > rangeMax)sensorCnt[i] = rangeMax;
	#else
		for(i = 0;i<SENSORS;i++)
			if(sensorCnt[i] > rangeMax)sensorCnt[i] = rangeMax;
	#endif
	*/
	
	//получаем ограничения счетов по значению мёртвого времени
	float	fDeadTime = MainSettings.mDeadTime;
	float	fDeadTimeExt = MainSettingsExt.mDeadTime;
	
	float fCntLimit = 0,fCntLimitExt = 0;
	
		fCntLimit = 1.0 / fDeadTime - 1.0f;
		fCntLimitExt = 1.0 / fDeadTimeExt - 1.0f;
	
	#ifdef RANGE3	

		
		if(Process_GetExternalRange() == EXT_ON)
			for(i = 0;i<SENSORS;i++)if(sensorCnt[i] > fCntLimit)sensorCnt[i] = fCntLimit;
		else
			for(i = 0;i<SENSORS;i++)if(sensorCnt[i] > fCntLimit)sensorCnt[i] = fCntLimitExt;
	#else
		for(i = 0;i<SENSORS;i++)
			if(sensorCnt[i] > fCntLimit)sensorCnt[i] = fCntLimit;
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

//подсчёт МД по формуле суммы трёх максимумов
float Process_CalculateMaximumSumDoseRate()
{
	float fResult = 0;
	float	sensorCorrectCnt[SENSORS];
	float fMax = 0;
	uint8	i = 0,j = 0;
	
	//проверка на максимальную скорость счёта в массиве счётчиков
	/*#ifdef RANGE3	
		uint16	rangeMax = (uint16)( (MainSettings.rangeLimits >> 16) & 0xFFFF);
	
		for(i = 0;i<SENSORS;i++)
			if(sensorCnt[i] > rangeMax)sensorCnt[i] = rangeMax;
	#endif*/
	
	memset(sensorCorrectCnt,0,sizeof(float) * SENSORS);
	
		//получаем откорректированные счета
		for(i = 0;i<SENSORS;i++){
			sensorCorrectCnt[i] = (sensorCnt[i] * MainSettings.correctionFactors[i]) / (1.0f - (sensorCnt[i] * MainSettings.mDeadTime));
		}
		//сортируем массив по возрастанию
			for(i = 0;i<SENSORS - 1;i++){
				for(j = 0;j<SENSORS - i - 1;j++){
					if(sensorCorrectCnt[j] > sensorCorrectCnt[j+1]){
						fMax = sensorCorrectCnt[j];
						sensorCorrectCnt[j] = sensorCorrectCnt[j+1];
						sensorCorrectCnt[j+1] = fMax;
					}
				}
			}
		Debug_SetSortSensorsData((uint8*)sensorCorrectCnt);
			
		fResult = (1.0f / (3.0f * MainSettings.mSensivity)) * (sensorCorrectCnt[11] + sensorCorrectCnt[10] + sensorCorrectCnt[9]);
			
		return fResult;
}
//подсчёт МД по логарифмической формуле
float Process_CalculateLogDoseRate()
{
	uint8 i = 0,j = 0;
	
	uint16 uMax = 0;
	uint16	sensorCorrectCnt[SENSORS];	//сортируемый массив счётчиков
	
	float fCurCorrFactor = 0;
	float	fSortCorrFactors[SENSORS];
	float fReturn = 0;
	float	sensorsCorrectionSum = 0;
	
	float fPulseDuration = MainSettingsExt.pulseDuration / 1000000.0f;
	float fPulsePeriod = MainSettingsExt.pulsePwrPeriod / 1000000.0f;
	
	memcpy((uint8*)sensorCorrectCnt,(uint8*)sensorCnt,sizeof(uint16) * SENSORS);
	memcpy((uint8*)fSortCorrFactors,(uint8*)MainSettingsExt.correctionFactors,sizeof(float) * SENSORS);
	
	//сортируем массив по возрастанию
			for(i = 0;i<SENSORS - 1;i++){
				for(j = 0;j<SENSORS - i - 1;j++){
					if(sensorCorrectCnt[j] > sensorCorrectCnt[j+1]){
						uMax = sensorCorrectCnt[j];
						fCurCorrFactor = fSortCorrFactors[j];
						
						sensorCorrectCnt[j] = sensorCorrectCnt[j+1];	
						fSortCorrFactors[j] = fSortCorrFactors[j+1];	//массив поправочных сортируем следом за массивом счетов
												
						sensorCorrectCnt[j+1] = uMax;
						fSortCorrFactors[j+1] = fCurCorrFactor;
					}
				}
			}
	
	
	
	//считаем по 3-м последним(максимальные счета)
		for(i = 9;i<SENSORS;i++){
			sensorsCorrectionSum +=( /*MainSettingsExt.correctionFactors[i]*/fSortCorrFactors[i] * (log(1.0 - sensorCorrectCnt[i] *  fPulsePeriod)/ fPulseDuration));
		}
		
		fReturn = -1.0 / (MainSettingsExt.mSensivity * MainSettingsExt.mEfficiency) * sensorsCorrectionSum;
	
	return fReturn;
}

float	Process_CalculateCountingRate()
{
	float	cntRate = 0;
	uint16	i = 0;
	
		for(i = 0;i<SENSORS;i++)
				cntRate += sensorCnt[i];
			cntRate = cntRate / (float)(MainSettings.mEfficiency /* SENSORS*/);	//06.03.17 - корректировка формулы рассчёта средней скрорости счёта
	
	if(firstMeasureCounter < FIRST_MEASURE_TIME)
		firstMeasureCounter++;
	
	return cntRate;
}

void Process_Start()
{
	//перенастройка ШИМ для 3 диапазона
	#ifdef RANGE3
		if(Process_GetExternalRange())PWM2();
		else PWM1();
	#endif
	
	Process_Stop();
	
	sliderCounter = 0;
	firstMeasureCounter = 0;
	
	memset(sensorCnt,0,sizeof(uint16) * SENSORS);
	memset(sensorCntCopy,0,sizeof(uint16) * SENSORS);
	
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

#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "main.h"

#define RANGES_COUNT	1
#define FACTORS_COUNT	12


typedef struct
{
	uint16	pulsePwrPeriod;		//период импульсного питания (мкс)
	uint16	pulseDuration;		//длительность импульсного питания (мкс)
	uint8		modeValue;				//вариант работы в режиме измерения
	//uint8		rangeValue;			//номер текущего диапазона
	uint8		mSliderSize;			//размеры движков
	float		mDeadTime;				//мертвое время
	float		mEfficiency;			//эффективность
	float		mSensivity;				//чувствительность
	
	uint32	rangeLimits;			//[32:16-max 15:0-min]
	

	float		correctionFactors[FACTORS_COUNT];
	 
	//uint16	timeSpan;
}tSettings;

extern tSettings	MainSettings;

void		SETTINGS_Init(void);
void		SETTINGS_Default(void);
void 		SETTINGS_Save(void);
uint8  	SETTINGS_Load(void);

#endif

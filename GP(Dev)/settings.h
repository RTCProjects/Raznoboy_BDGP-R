#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "main.h"

#define RANGES_COUNT	1
#define FACTORS_COUNT	12

#define BASE_SET_ADDRESS	0x22000
#define EXT_SET_ADDRESS		0x22080

typedef struct
{
	uint16	pulsePwrPeriod;		//период импульсного питания (мкс)
	uint16	pulseDuration;		//длительность импульсного питания (мкс)
	uint8		modeValue;				//вариант работы в режиме измерения
	uint8		mSliderSize;			//размеры движков
	float		mDeadTime;				//мертвое время
	float		mEfficiency;			//эффективность
	float		mSensivity;				//чувствительность
	
	uint32	rangeLimits;			//[32:16-max 15:0-min]

	float		correctionFactors[FACTORS_COUNT];
}tSettings;

extern tSettings	MainSettings;
extern tSettings	MainSettingsExt;

void		Settings_Init(void);
void		Settings_Default(void);
void		Settings_DefautlExt(void);
//void 		SETTINGS_Save(void);
//uint8  	SETTINGS_Load(void);

void	Settings_Save(uint16	*pData,uint16 uDataSize,uint32	uAddress);
uint8	Settings_Load(uint16	*pData,uint16 uDataSize,uint32	uAddress);

#endif

#include "settings.h"
#include "flash.h"

tSettings	MainSettings;

void	SETTINGS_Init()
{

	if(!SETTINGS_Load())
	{
		SETTINGS_Default();
		SETTINGS_Save();

	}
}

void	SETTINGS_Default()
{
	int i = 0;
	uint16	limitMin = 0;
	uint16	limitMax = 0;
	
		MainSettings.pulsePwrPeriod = 100;	
		MainSettings.pulseDuration = 100;
		MainSettings.modeValue = 1;
		
			MainSettings.mSliderSize = 10;
			MainSettings.mDeadTime = 0.0006f;
		
		#ifndef MAIN_RANGE
			MainSettings.mEfficiency = 9.276f;
			MainSettings.mSensivity = 383.0f;
		
				limitMin = 0;
				limitMax = 1220;
		#else
			MainSettings.mEfficiency = 8.4f;
			MainSettings.mSensivity = 1.26f;
			
				limitMin = 15;
				limitMax = 1470;
		#endif
			MainSettings.rangeLimits = (uint32)((uint32)limitMax << 16)|limitMin;
	
	for(i = 0;i<FACTORS_COUNT;i++)
		MainSettings.correctionFactors[i] = 1.0f;
	
	//MainSettings.timeSpan = 10;
}

void 	SETTINGS_Save()
{
	int i = 0,addr_counter = 0;
	
	 uint16  *pSettings = (tSettings*)&MainSettings;
	 uint16 sizeSettings = sizeof(MainSettings);
	
	 uint16	chkSum = 0;
	
	 for(i = 0;i<sizeSettings >> 1;i++)
			chkSum += *(pSettings + i);
	
	FLASH_EraseSector(0x22000);
	FLASH_FSRControl();
	
	FLASH_WriteWord(0x22000,chkSum);
	FLASH_FSRControl();
	
	for(i = 0;i<sizeSettings;i+=2)
	{
		FLASH_WriteWord(0x22002 + i,*(pSettings + addr_counter));
		FLASH_FSRControl();
		

		addr_counter++;
	}
}
uint8 SETTINGS_Load()
{
	int i = 0;
	unsigned int far *addressChkSum = (unsigned int far *) 0x022000;
	unsigned int far *addressData = (unsigned int far *) 0x022002;
	
	uint16	*pSettings = (tSettings*)&MainSettings;
	uint16	dataSize = sizeof(MainSettings);
	uint16  chkSum = 0;
	
	for(i = 0;i<dataSize >> 1;i++){
		chkSum += *(addressData + i);

	}

	if(chkSum != *addressChkSum)
		return 0;
	
	
	for(i = 0;i<dataSize >> 1;i++){
		*(pSettings + i) = *(addressData + i);
		
	}
	return 1;
}

#include "settings.h"
#include "flash.h"

tSettings	MainSettings;

/*
Дополнительный блок настроек для 3 диап. с другой характеристикой импульсного питания
*/

tSettings MainSettingsExt;


void	Settings_Init()
{	
	/*if(!SETTINGS_Load())
	{
		SETTINGS_Default();
		SETTINGS_Save();
	}*/
	if(!Settings_Load((uint16*)&MainSettings,sizeof(tSettings),BASE_SET_ADDRESS)){
		Settings_Default();
		Settings_Save((uint16*)&MainSettings,sizeof(tSettings),BASE_SET_ADDRESS);
	}
	//загрузка настроек для расширенного диапазона
	#ifdef RANGE3
	if(!Settings_Load((uint16*)&MainSettingsExt,sizeof(tSettings),EXT_SET_ADDRESS)){
		Settings_DefautlExt();
		Settings_Save((uint16*)&MainSettingsExt,sizeof(tSettings),EXT_SET_ADDRESS);
	}
	#endif
}

void	Settings_Default()
{
	int i = 0;
	uint16	limitMin = 0;
	uint16	limitMax = 0;
	
		MainSettings.pulsePwrPeriod = 100;	
		MainSettings.pulseDuration = 100;
		MainSettings.modeValue = 1;
		
			MainSettings.mSliderSize = 10;
			MainSettings.mDeadTime = 0.0006f;
		
		#ifdef RANGE3
			MainSettings.mEfficiency = 8.17f;
			MainSettings.mSensivity = 1.26f;
			
				limitMin = 10;
				limitMax = 1280;
		#endif
	
		#ifdef RANGE2
			MainSettings.mEfficiency = 8.63f;
			MainSettings.mSensivity = 383.f;
			
				limitMin = 550;
				limitMax = 1280;
		#endif
		
		#ifdef RANGE1
			MainSettings.mEfficiency = 8.83f;
			MainSettings.mSensivity = 1405.f;
				limitMin = 0;
				limitMax = 1280;
		#endif
			MainSettings.rangeLimits = (uint32)((uint32)limitMax << 16)|limitMin;
	
	for(i = 0;i<FACTORS_COUNT;i++)
		MainSettings.correctionFactors[i] = 1.0f;
}
void	Settings_DefautlExt()
{
	uint8 i = 0;

	MainSettingsExt.pulsePwrPeriod = 200;	
	MainSettingsExt.pulseDuration = 10;
	MainSettingsExt.modeValue = 1;
	
	MainSettingsExt.mDeadTime = 0.0006f;
	MainSettingsExt.mEfficiency = 3.0f;
	MainSettingsExt.mSensivity = 1.26f;
	MainSettingsExt.rangeLimits = (uint32)((uint32)4000 << 16)|150;
	
	for(i = 0;i<FACTORS_COUNT;i++)
		MainSettingsExt.correctionFactors[i] = 1.5f;
}

void	Settings_Save(uint16	*pData,uint16 uDataSize,uint32	uAddress)
{
	int i = 0,addr_counter = 0;
	
	 uint16  *pSettings = (tSettings*)pData;
	 uint16 sizeSettings = uDataSize;
	
	 uint16	chkSum = 0;
	
	 for(i = 0;i<sizeSettings >> 1;i++)
			chkSum += *(pSettings + i);
	
	FLASH_EraseSector(uAddress);
	FLASH_FSRControl();
	
	FLASH_WriteWord(uAddress,chkSum);
	FLASH_FSRControl();
	
	for(i = 0;i<sizeSettings;i+=2)
	{
		FLASH_WriteWord((uAddress+2) + i,*(pSettings + addr_counter));
		FLASH_FSRControl();
		

		addr_counter++;
	}
}
uint8	Settings_Load(uint16	*pData,uint16 uDataSize,uint32	uAddress)
{
	int i = 0;
	unsigned int far *addressChkSum = (unsigned int far *) uAddress;
	unsigned int far *addressData = (unsigned int far *) (uAddress+2);
	
	uint16	*pSettings = (tSettings*)pData;
	uint16	dataSize = uDataSize;
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

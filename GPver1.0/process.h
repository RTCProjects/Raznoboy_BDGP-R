#ifndef _PROCESS_H
#define _PROCESS_H

#include "main.h"

#define SENSORS			12
#define SLIDER_SIZE 2

typedef enum
{
	IMDB_INITIALIZATION 						= 0x00,
	IMDB_OPERATION_CONTROL					= 0x01,
	IMDB_FULLTIME_WORK							= 0x02,
	IMDB_SETTINGS_MODE		 					= 0x03,
	IMDB_SPECTR_ACCUMULATION	 			= 0x04,
	IMDB_CALIBRATION_I		 					= 0x05,
	IMDB_CALIBRATION_II		 					= 0x06,
	IMDB_CALIBRATION_III		 				= 0x07,
	IMDB_CALIBRATION_IV				 			= 0x08,
	IMDB_BACKGROUND_ACCUMULATION	 	= 0x09
}IMDB_Status;

typedef enum
{
	DEVICE_READY = 1,
	DEVICE_NOTREADY = 0
}Device_Status;

typedef enum
{
	AUTO_ON	= 1,
	AUTO_OFF = 0
}eAutoModeState;

sbit KEY11_L	= P9^1;
sbit KEY11_H	= P9^0;

sbit KEY12_L	= P9^4;
sbit KEY12_H	= P9^5;

sbit HV_STATE	= P3^0;


void	Process_Initializetion(void);
void 	Process_Start(void);
void 	Process_Stop(void);

float	Process_CalculateCurrentDoseRate(void);
float	Process_CalculateCountingRate(void);

void Process_SetAutoMode(eAutoModeState State);
uint8	Process_GetAutoModeStatus(void);
#endif

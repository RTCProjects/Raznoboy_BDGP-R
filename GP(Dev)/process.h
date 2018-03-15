#ifndef _PROCESS_H
#define _PROCESS_H

#include "main.h"

#define SENSORS					12
#define DOSERATE_LIMIT	0.03f
#ifdef RANGE1
	#define SLIDER_SIZE 		30
#else
	#define SLIDER_SIZE			2
#endif
#define FIRST_MEASURE_TIME	2
#define SLIDER_30_MAX_LIM		0.0022
#define SLIDER_10_MIN_LIM		0.0017
#define SLIDER_10_MAX_LIM		0.0087
#define SLIDER_2_MIN_LIM		0.0075
//характеристики для динамического движка первого диапазона


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

typedef enum
{
	EXT_ON = 1,
	EXT_OFF = 0
}eExternalRange;

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
float Process_CalculateMaximumSumDoseRate(void);
float Process_CalculateLogDoseRate(void);
void 	Process_SetAutoMode(eAutoModeState State);
uint8	Process_GetAutoModeStatus(void);
void	Process_SetExternalRange(eExternalRange ExtRange);
uint8 Process_GetExternalRange(void);
#endif

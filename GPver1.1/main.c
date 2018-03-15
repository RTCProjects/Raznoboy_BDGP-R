#include "main.h"
#include "can.h"
#include "flash.h"
#include "settings.h"
#include "protocol.h"
#include "process.h"
#include "system.h"
#include "math.h"

uint8 	malloc_mempool[0x1000];	//4КБ динамической памяти 

const uint32 SerialNumberBD = 1301001;

void main(void)
{
	unsigned int far *pReprogWord = (unsigned int far *) 0x50000;
	
	WDTIN = 1;
	WDTPRE = 1;

	CAN_Init();

	if(SWR == 1)
	{
		if(*pReprogWord == 0x1234)
		{
			*pReprogWord = 0;
			
			__asm {CALLS 04H 0E000H}
		}	
	}

		init_mempool(&malloc_mempool, sizeof(malloc_mempool));
		
		SETTINGS_Init();		

		
		Process_Initializetion();
		Protocol_Initialization();
	
	DP3 = 0x0003;
	P3 = 0x0000;
	
	
	P2 = 0x0000;
	DP2 = 0x0000;
	
	//P7 |= 0x0000;
	DP7 |= 0x0004;
		
	T01CON = 0x0106;
	T0REL = 0x6769;
	
	T0IC = 0x006A;

	
	T78CON = 0x0101;
	T7IC = 0x006B;
	T8IC = 0x006C;
	

	CC28IC = 0x007F;
	CC30IC = 0x007E;
	/////////
	//Вариант 1 - генерация прерываний в режиме таймера (по нарастанию и спаданию фронта)
	/*T3CON = 0x0079;
	T2CON = 0x0079;
	T4CON = 0x0079;*/
	
	//Вариант 2 - генерация прерываний в режиме счётчика с переполнением(по положительному фронту)
	T3CON = 0x0049;
	T3 = 0xFFFF;
	
	T2CON = 0x0049;
	T2 = 0xFFFF;
	
	T4CON = 0x0049;
	T4 = 0xFFFF;
	//Т5 работает только в режиме счётчика
	T5CON = 0x0049;
	T5= 0xFFFF;
	//T3 = 0xFFFF;
	
	////////
	
	T7R = 0;
	T8R = 0;

	T0R = 0;	

	//P7.4
	CC28 = 0xFB88;
	CCM7 |= 0x0006;
	//P7.6
	CC30 = 0xFB88;
	CCM7 |= 0x0F00; 


	//настройка внешних входов прерываний
	EXICON = 0x5555; // все входы по положительному фронту
	EXISEL0 = 0;	//стандартный вход P2.X
	EXISEL1 = 0;
	
	IEN = 1;

	//проверка на перезапуск
	if(SWR)Protocol_StartMsg();

	

		while(1)
		{
			_srvwdt_();
			CAN_Process();
		}
}



#include "system.h"



uint8	System_RAMTest(void)
{
		__IO uint32	cnt = 0;
		__IO uint32	temp = 0;
		uint32 	index = 0;
		uint16		*ramStart = 0x600000;

		uint16	ramData = 0;
		uint16	loopData = 0;
	
		xmemset(ramStart,0,0x80000);
	
		for(index = 0;index<0x40000;index++)
		{
			(*(ramStart+index)) = (uint16)index;
		}


		for(index = 0;index<0x40000;index++)
		{
			ramData = (*((uint16*)ramStart+index));
		
			
			if(ramData != (uint16)index)
			{
				return 0;
			}
		}
		xmemset(ramStart,0,0x80000);
		
		return 1;
}

void	System_Delay(unsigned long z){
  unsigned long x;
  for (x = 0; x < z; x++){
		
	}
}
void	System_Reset()
{
	__asm { SRST }
}

void	System_WDT_Reset()
{/*
	__asm 
	{
		MOV 1111H R12
	}*/
}


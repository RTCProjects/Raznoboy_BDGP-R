#ifndef _FLASH_H
#define _FLASH_H

#include "main.h"

void FLASH_EraseSector(unsigned long sector);
void FLASH_WriteWord(unsigned long address,unsigned int word);
void FLASH_FSRControl();
#endif

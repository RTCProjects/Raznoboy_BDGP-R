#include "can.h"
#include "settings.h"
#include "protocol.h"
#include "system.h"
#include "main.h"

static uint8	txEnable = 0;

uint16	packID = 0;			//ИД по к-ому передается пакет
uint8		packIndex = 0;	//индекс передаваемого блока в пакете данных
uint8		packCount = 0;	//кол-во блоков
uint8		packLastSize = 0;	//размер последнего блока
uint8		*packData;					//оказатель на блок пакетов

sbit CAN0IR = CAN_IC0^7;

void CAN_Interrupt0() interrupt 0x54	
{
	uint8	currentPackLen = 8;

	txEnable = 1;
	
	if(packIndex < packCount){

		if(packIndex == packCount - 1)
			if(packLastSize != 0)
				currentPackLen = packLastSize;
		
		CAN_SendMessageIT(packID, packData,currentPackLen);
		
			packData += 8;
			packIndex++;
	}
	

}

void CAN_Interrupt1() interrupt 0x55		
{
	uint16 canID = ((CAN_Message_16[3].MOARH >> 2) & 0x07FF);
	uint8 *msgData = (uint8*)malloc(sizeof(uint8) * 8);

		*((uint16*)&msgData[0]) = CAN_Message_16[3].MODATALL;
		*((uint16*)&msgData[2]) = CAN_Message_16[3].MODATALH;
		*((uint16*)&msgData[4]) = CAN_Message_16[3].MODATAHL;
		*((uint16*)&msgData[6]) = CAN_Message_16[3].MODATAHH;

	Protocol_ReceiveMsgCallback(canID,msgData);

	free(msgData);
	
	
}

void CAN_Interrupt2() interrupt 0x56			
{
	uint16 canID = ((CAN_Message_16[4].MOARH >> 2) & 0x07FF);
	uint8 *msgData = (uint8*)malloc(sizeof(uint8) * 8);

		*((uint16*)&msgData[0]) = CAN_Message_16[4].MODATALL;
		*((uint16*)&msgData[2]) = CAN_Message_16[4].MODATALH;
		*((uint16*)&msgData[4]) = CAN_Message_16[4].MODATAHL;
		*((uint16*)&msgData[6]) = CAN_Message_16[4].MODATAHH;

	Protocol_ReceiveMsgCallback(canID,msgData);

	free(msgData);

}
void CAN_Interrupt3() interrupt CAN3IC_VEC			
{
	uint8 *msgData = (uint8*)malloc(sizeof(uint8) * 8);

		*((uint16*)&msgData[0]) = CAN_Message_16[2].MODATALL;
		*((uint16*)&msgData[2]) = CAN_Message_16[2].MODATALH;
		*((uint16*)&msgData[4]) = CAN_Message_16[2].MODATAHL;
		*((uint16*)&msgData[6]) = CAN_Message_16[2].MODATAHH;

	Protocol_ReceiveMsgCallback(0x600,msgData);

	free(msgData);
}
void CAN_Interrupt4() interrupt CAN4IC_VEC
{
	unsigned int far *pReprogWord = (unsigned int far *) 0x50000;
	
	uint8	msgData[8];
	uint32 serialNumber = 0;
	
		*((uint16*)&msgData[0]) = CAN_Message_16[5].MODATALL;
		*((uint16*)&msgData[2]) = CAN_Message_16[5].MODATALH;
		*((uint16*)&msgData[4]) = CAN_Message_16[5].MODATAHL;
		*((uint16*)&msgData[6]) = CAN_Message_16[5].MODATAHH;
	
	//тестовый код для запуска загрузчика
	//if(regID == 0x000)
		if(msgData[0] == 0xD2 && msgData[1] == 0x2D && msgData[2] == 0x00 && msgData[3] == 0x00 &&
			 msgData[4] == 0xFF && msgData[5] == 0xFF && msgData[6] == 0xFF && msgData[7] == 0xFF)
			{
				//на запрос серийного номера, отправим серийный номер текущего устройства
				memcpy(msgData + 4,(uint32*)&SerialNumberBD,4);
				//и его аппаратный номер
				msgData[3] = HARDWARE_NUMBER;
				
				CAN_SendMessage(0x000,msgData,8);
				/*__asm
				{
					PUSH R2
					MOV R2 #1234H
					EXTS #05H #1H
					MOV 0000H R2
					POP R2
					
					SRST
				}*/
			}
		if(msgData[0] == 0xD3 && msgData[1] == 0x3D && msgData[2] == 0x00){
			memcpy((uint32*)&serialNumber,(uint8*)msgData + 4,4);
			
			if(serialNumber == SerialNumberBD && msgData[3] == HARDWARE_NUMBER)
			{
				*pReprogWord = 0x1234;
				
				__asm {SRST};
			}
			/*
			msgData[3] - SubIndex
			msgData[4 - 7] - SerialNumber
			*/
		}
}
 
void CAN_Init()
{
	txEnable = 1;
	
	packID = 0;			//ИД по к-ому передается пакет
	packIndex = 0;	//индекс передаваемого блока в пакете данных
	packCount = 0;	//кол-во блоков
	packLastSize = 0;	//размер последнего блока
	
	DP9 |= 0x003B;
	ALTSEL0P9 |= 0x0008;
	ALTSEL1P9 |= 0x0008;
	
	
	CAN_IC0 = 0x005C;	//72Xh
	CAN_IC1 = 0x0060;	//00Xh
	CAN_IC2 = 0x0074;	//20Xh
	CAN_IC3 = 0x0064;	//600h
	CAN_IC4 = 0x0068;	//000h
	
	
	NCR0  = 0x0041;		//разрешаем вносить изменения в конфигурацию
	NPCR0 = 0x0003;		//Выбрали P9.2 для приема в CAN узел 0
	NBTR0 = 0x2344;		//скорость узла 500kb/sec при тактировании 16Мгц
	NCR0  = 0x0000;		//Запрет после реконфигурации
	
	/*
		Список областей сообщений
	
		На приём
			3. 00Хh - аварийное сообщение для CAN
	    2. 20Хh - сообщение с данными от БДГП-С
		На передачу
		  1. 000 - CAN сообщение с настраиваемым ID
			2. 722 - данные (прерывание)
			3. 280 - передача старт/стоп
	*/
	//передача
	//000 - универсальный вар.
	CAN_Message_16[0].MOCTRH = 0x0E08;
	CAN_Message_16[0].MOCTRL = 0x0000;
	CAN_Message_16[0].MOIPRL = 0x0000;	/*выбор линии прерываний для передачи - 0*/
  CAN_Message_16[0].MOFCRH = 0x0802; // DLC = 8, разрешить прерывание на передачу 
	 
	
	//72Xh
	/*CAN_Message_16[1].MOCTRH = 0x0080;
  CAN_Message_16[1].MOCTRL = 0x0000;
  CAN_Message_16[1].MOIPRL = 0x0000;	// выбор линии прерываний по приему - 0
	CAN_Message_16[1].MOFCRH = 0x0801;	// разрешить прерывание на прием
	
*/
	CAN_Message_16[2].MOCTRH = 0x0080;
	CAN_Message_16[2].MOCTRL = 0x0000;
	CAN_Message_16[2].MOIPRL = 0x0003;	//выбор линии прерываний для передачи - 3
  CAN_Message_16[2].MOFCRH = 0x0801; // разрешить прерывание на прием 
	

	//32X
	CAN_Message_16[3].MOCTRH = 0x0080;
  CAN_Message_16[3].MOCTRL = 0x0000;
  CAN_Message_16[3].MOIPRL = 0x0001;	//выбор линии прерываний по приему - 1
	CAN_Message_16[3].MOFCRH = 0x0801;	//разрешить прерываение на прием
	
	//20X
	CAN_Message_16[4].MOCTRH = 0x0080;
  CAN_Message_16[4].MOCTRL = 0x0000;
  CAN_Message_16[4].MOIPRL = 0x0002;	// выбор линии прерываний по приему - 2
	CAN_Message_16[4].MOFCRH = 0x0801;	//разрешить прерываение на прием
	
	//000 - для запуска BSL
	CAN_Message_16[5].MOCTRH = 0x0080;
  CAN_Message_16[5].MOCTRL = 0x0000;
  CAN_Message_16[5].MOIPRL = 0x0004;	// выбор линии прерываний по приему - 4
	CAN_Message_16[5].MOFCRH = 0x0801;	//разрешить прерываение на прием	


	System_Delay(10);
	
	CAN_Message_16[0].MOARH = 0x8000;	 // идентификатор 000(XXX)h - динамически ИД
  CAN_Message_16[0].MOAMRH = 0x3FFF; //	маска идентификатора	
		
	/*CAN_Message_16[1].MOARH = 0x9C80;	 // идентификатор 72Xh (X - 2,3,4)
  CAN_Message_16[1].MOAMRH = 0x3FE3; //	маска идентификатора		
	*/
	CAN_Message_16[2].MOARH = 0x9800;	 // идентификатор 600h
  CAN_Message_16[2].MOAMRH = 0x3FFF; //	маска идентификатора	
	
	CAN_Message_16[3].MOARH = 0x8888 + (HARDWARE_NUMBER << 2);	 // идентификатор 222h
  CAN_Message_16[3].MOAMRH = 0x3FFF; //	маска идентификатора
	
	CAN_Message_16[4].MOARH = 0x8600;	 // идентификатор 180h
  CAN_Message_16[4].MOAMRH = 0x3FFF; //	маска идентификатора	
	
	CAN_Message_16[5].MOARH = 0x8000;	 // идентификатор 000h
  CAN_Message_16[5].MOAMRH = 0x3FFF; //	маска идентификатора	
	

	System_Delay(10);
	
	PANCTR_H = 0x0100;				 // message object 0 -> List 1
  PANCTR = 0x0002;				   
   _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_();
	 
	 
	PANCTR_H = 0x0101;				 //	message object 1 -> List 1
  PANCTR = 0x0002;			    	 
   _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); 
	 
	 
	PANCTR_H = 0x0102;				 //	message object 2 -> List 1
  PANCTR = 0x0002;			    	 
   _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); 	
	 
	 
	PANCTR_H = 0x0103;				 //	message object 3 -> List 1
  PANCTR = 0x0002;			    	 
   _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); 	 


	PANCTR_H = 0x0104;				 //	message object 4 -> List 1
  PANCTR = 0x0002;			    	 
   _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); 	 
	 
	PANCTR_H = 0x0105;				 //	message object 5 -> List 1
  PANCTR = 0x0002;			    	 
   _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); 	 
	 
	  


	CAN_Message_16[0].MOCTRH = 0x0020; 
	CAN_Message_16[0].MOCTRL = 0x0000;
	
	CAN_Message_16[1].MOCTRH = 0x0020; 
	CAN_Message_16[1].MOCTRL = 0x0000;
	
	CAN_Message_16[2].MOCTRH = 0x0020; 
	CAN_Message_16[2].MOCTRL = 0x0000;
	
	CAN_Message_16[3].MOCTRH = 0x0020; 
	CAN_Message_16[3].MOCTRL = 0x0000;
	
	CAN_Message_16[4].MOCTRH = 0x0020; 
	CAN_Message_16[4].MOCTRL = 0x0000;	
	
	CAN_Message_16[5].MOCTRH = 0x0020; 
	CAN_Message_16[5].MOCTRL = 0x0000;	
	
	
}
void CAN_SendDataPackage(uint16 id,uint8	*pData, uint8 packSize)
{
	packIndex = 0;
	packCount = packSize / 8;
	packLastSize = packSize % 8;
		if(packLastSize > 0)
			packCount ++;
	
	packID = id;
	packData = pData;

		CAN0IR = 1;	//инициируем передачу
		
}
void CAN_SendMessage(uint16 id,uint8 *pData,uint8 Len)
{
	CAN_SendMessageIT(id,pData,Len);
		System_Delay(400);
}
void CAN_SendMessageIT(uint16 id,uint8 *pData,uint8 Len)
{ 
	uint8 *moData = 0;
	uint8	index = 0;
	
	/*	while(txEnable == 0)
		{
			//if( ((NSR0 >> 7) & 0x1) == 1)
			if( (NSR0 & 0x7) != 0){	//проверка acknolegment 
				//IMDB_SendCanTransmitError(0);
				break;
			}
		}
*/
	CAN_Message_16[0].MOFCRH &= 0xF0FF;
	CAN_Message_16[0].MOFCRH |= (uint16) (Len<<8);
	
	CAN_Message_16[0].MOARH &= 0x8000;
	CAN_Message_16[0].MOARH |= (uint16)(id<<2);
	
	
	moData = (uint8*)&CAN_Message_16[0].MODATALL;
	
	for(index = 0;index<Len;index++)
	{
		*(moData + (index)) = *(pData + index);
	}
	
	txEnable = 0;
	
	CAN_Message_16[0].MOCTRH = 0x0100;	
	
	
}
/*
void CAN_SendProtocolData(uint8 moNubmer,uint8	*pData, int packSize)
{
	int msgCnt = packSize / 8;
	int lastMsgSize = packSize % 8;
	int i,j = 0;
	
	uint8	arrData[8];
	
	for(i = 0;i < msgCnt;i++)
	{
		for(j = 0;j<8;j++)
			arrData[j] = *(pData + j + (i*8));
		
		CAN_SendMessage(moNubmer,arrData,8);
	}
}
*/

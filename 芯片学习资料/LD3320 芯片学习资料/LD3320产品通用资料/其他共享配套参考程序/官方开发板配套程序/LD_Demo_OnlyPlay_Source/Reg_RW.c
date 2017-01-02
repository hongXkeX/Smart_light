/************************************************************************************/
//	��Ȩ���У�Copyright (c) 2005 - 2010 ICRoute INC.
/************************************************************************************/

#include "STC10F08XE.H"
#include "Reg_RW.h"

extern void _nop_(void);

/************************************************************************************/
//	����MCU����LD3320��д�Ĵ�����ʵ�������֣�
//	#define SOFT_PARA_PORT		//	���ģ�Ⲣ�ж�д
//	#define HARD_PARA_PORT		//	Ӳ��ʵ�ֲ��ж�д ����Ҫ����MCU��Ӳ����WR/RD�˿ڣ�
//	#define SOFT_SPI_PORT		//	���ģ��SPI��д
//	#define HARD_SPI_PORT		//	Ӳ��ʵ��SPI��д	  ����Ҫ����MCU��Ӳ��SPI�ӿڣ�
/************************************************************************************/


#ifdef SOFT_PARA_PORT

		#define DELAY_NOP	_nop_();_nop_();_nop_();
		sbit	LD_WR 	= P3^6;
		sbit	LD_RD	= P3^7;
		sbit	LD_CS	= P2^6;
		sbit	LD_A0	= P2^0;
		void LD_WriteReg( unsigned char address, unsigned char dataout )
		{
			P0 = address;
			LD_A0 = 1;
			LD_CS = 0;
			LD_WR = 0;
			DELAY_NOP;
		
			LD_WR = 1;
			LD_CS = 1;
			DELAY_NOP;
		
			P0 = dataout;
			LD_A0 = 0;
			LD_CS = 0;
			LD_WR = 0;
			DELAY_NOP;
		
			LD_WR = 1;
			LD_CS = 1;
			DELAY_NOP;
		}
		
		unsigned char LD_ReadReg( unsigned char address )
		{
			unsigned char datain;
		
			P0 = address;
			LD_A0 = 1;
			LD_CS = 0;
			LD_WR = 0;
			DELAY_NOP;
		
			LD_WR = 1;
			LD_CS = 1;
			DELAY_NOP;
		
			LD_A0 = 0;
			LD_CS = 0;
			LD_RD = 0;
			DELAY_NOP;

			datain = P0;
			LD_RD = 1;
			LD_CS = 1;
			DELAY_NOP;
		
			return datain;
		}
#endif

#ifdef HARD_PARA_PORT

		#define LD_INDEX_PORT		(*((volatile unsigned char xdata*)(0x8100))) 
		#define LD_DATA_PORT		(*((volatile unsigned char xdata*)(0x8000))) 
		
		//�������� MCU��A8 ���ӵ� LDоƬ��AD
		//         MCU��A14 ���ӵ� LDоƬ��CSB
		//         MCU��RD��WR ���� LDоƬ��RD��WR (xdata ��дʱ�Զ��������ź�)
		//
		//0x8100�Ķ�������10000001 00000000		CSB=0 AD=1
		//                 ^     ^
		//0x8000�Ķ�������10000000 00000000		CSB=0 AD=0
		//                 ^     ^
		
		void LD_WriteReg( unsigned char address, unsigned char dataout )
		{
			LD_INDEX_PORT  = address;         
			LD_DATA_PORT = dataout;          
		}
		
		unsigned char LD_ReadReg( unsigned char address )
		{
			LD_INDEX_PORT = address;         
			return (unsigned char)LD_DATA_PORT;     
		}
#endif

#ifdef SOFT_SPI_PORT
		#define DELAY_NOP	_nop_();_nop_();_nop_();
		
		sbit SCS=P2^6;    //оƬƬѡ�ź�
		sbit SDCK=P0^2;   //SPI ʱ���ź�
		sbit SDI=P0^0;    //SPI ��������
		sbit SDO=P0^1;    //SPI �������
		sbit SPIS=P3^6;   //SPIģʽ���ã�����Ч��

		void LD_WriteReg(unsigned char address,unsigned char dataout)
		{
			unsigned char i = 0;
			unsigned char command=0x04;
			SPIS =0;
			SCS = 0;
			DELAY_NOP;
		
			//write command
			for (i=0;i < 8; i++)
			{
				if ((command & 0x80) == 0x80) 
					SDI = 1;
				else
					SDI = 0;
				
				DELAY_NOP;
				SDCK = 0;
				command = (command << 1);  
				DELAY_NOP;
				SDCK = 1;  
			}
			//write address
			for (i=0;i < 8; i++)
			{
				if ((address & 0x80) == 0x80) 
					SDI = 1;
				else
					SDI = 0;
				DELAY_NOP;
				SDCK = 0;
				address = (address << 1); 
				DELAY_NOP;
				SDCK = 1;  
			}
			//write data
			for (i=0;i < 8; i++)
			{
				if ((dataout & 0x80) == 0x80) 
					SDI = 1;
				else
					SDI = 0;
				DELAY_NOP;
				SDCK = 0;
				dataout = (dataout << 1); 
				DELAY_NOP;
				SDCK = 1;  
			}
			DELAY_NOP;
			SCS = 1;
		}

		unsigned char LD_ReadReg(unsigned char address)
		{
			unsigned char i = 0; 
			unsigned char datain =0 ;
			unsigned char temp = 0; 
			unsigned char command=0x05;
			SPIS =0;
			SCS = 0;
			DELAY_NOP;
		
			//write command
			for (i=0;i < 8; i++)
			{
				if ((command & 0x80) == 0x80) 
					SDI = 1;
				else
					SDI = 0;
				DELAY_NOP;
				SDCK = 0;
				command = (command << 1);  
				DELAY_NOP;
				SDCK = 1;  
			}

			//write address
			for (i=0;i < 8; i++)
			{
				if ((address & 0x80) == 0x80) 
					SDI = 1;
				else
					SDI = 0;
				DELAY_NOP;
				SDCK = 0;
				address = (address << 1); 
				DELAY_NOP;
				SDCK = 1;  
			}
			DELAY_NOP;

			//Read data
			for (i=0;i < 8; i++)
			{
				datain = (datain << 1);
				temp = SDO;
				DELAY_NOP;
				SDCK = 0;  
				if (temp == 1)  
					datain |= 0x01; 
				DELAY_NOP;
				SDCK = 1;  
			}
		
			DELAY_NOP;
			SCS = 1;
			return datain;
		}

#endif

#ifdef HARD_SPI_PORT
		/*
		STC��Ƭ������SPI�ڵ����ࣩ��˶�д�ĺ�������
		unsigend char SPI_TR( unsigned char x ) 
		{ 
			SPSTAT=0xC0;  
			SPDAT=x;  
			while(!(SPSTAT&0x80));  
			return SPDAT;
		}
		
		--------------------------------------------------
		AVR��Ƭ������SPI�ڵ����ࣩ��˶�д�ĺ�������
		unsigend char SPI_TR( unsigned char x ) 
		{ 
			SPDR=x;  
			while(!(SPSR & (1<<SPIF)));  
			return SPDR;
		}

		*/
		void LD_WriteReg( unsigned char address, unsigned char dataout )
		{
			// �������Ӳ��SPI�ڵĲ������룺
			SPI_TR(0x04); // ���� 0x04
			SPI_TR(address); // ���� address
			SPI_TR(dataout); // ���� dataout
		}
		unsigned char LD_ReadReg( unsigned char address )
		{
			// �������Ӳ��SPI�ڵĲ������룺
			SPI_TR(0x05); // ���� 0x05
			SPI_TR(address); // ���� address
			return (SPI_TR(0)); // ��������, ������
		}
#endif





/************************************************************************************
**	CPU: STC89LE52
**	����22.1184MHZ
**	�����ʣ�9600 bit/S
**	���ײ�Ʒ��Ϣ��LD3320 ���ض�����ģ�� ������
**                http://yuesheng001.taobao.com/
**  �汾��zds0.0.3
**  �޸����ڣ�2011.12.10
/************************************************************************************/
#include "config.h"


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


//���ģ�Ⲣ�з�ʽ��д
#ifdef SOFT_PARA_PORT

		#define DELAY_NOP	_nop_();_nop_();_nop_();_nop_(); _nop_(); 

		sbit	LD_WR = P3^6;
		sbit	LD_RD	= P3^7;
		sbit	LD_CS	= P2^1;
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


//���ģ��SPI��ʽ��д
#ifdef SOFT_SPI_PORT
		#define DELAY_NOP	_nop_();_nop_();_nop_();
		
		sbit SCS=P2^1;    //оƬƬѡ�ź�
		sbit SDCK=P0^2;   //SPI ʱ���ź�
		sbit SDI=P0^0;    //SPI ��������
		sbit SDO=P0^1;    //SPI �������
		sbit SPIS=P3^5;   //SPIģʽ���ã�����Ч��

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

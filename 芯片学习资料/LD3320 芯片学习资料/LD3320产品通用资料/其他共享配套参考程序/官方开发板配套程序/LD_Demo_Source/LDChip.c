/************************************************************************************/
//	��Ȩ���У�Copyright (c) 2005 - 2010 ICRoute INC.
/************************************************************************************/



#include "STC10F08XE.H"
#include "LDChip.h"
#include "Reg_RW.h"
#include "FlashDef.h"

sbit RSTB=P3^3;
sbit CSB=P2^6;

uint32 nMp3StartPos=0;
uint32 nMp3Size=0;
uint32 nMp3Pos=0;
uint32 nCurMp3Pos=0;
uint8  idata nLD_Mode = LD_MODE_IDLE;		//	������¼��ǰ���ڽ���ASRʶ�����ڲ���MP3
uint8 bMp3Play=0;							//	������¼����MP3��״̬
uint8 idata ucRegVal;
uint8 idata ucHighInt;
uint8 idata ucLowInt;
uint8 idata ucStatus;
uint8 idata ucSPVol=15; // MAX=15 MIN=0		//	Speaker�������������

extern uint8 idata nAsrStatus;
void ProcessInt0();

/************************************************************************************/
//	���C�ļ���������ĺ������൱��LD3320����������C����Դ�������ʽ�ṩ
//	��������û�е���ͨ��֮ǰ��һ����Ҫ�޸�������ĺ���
//	
//	LD_ReloadMp3Data()��������ǣ�浽����MCU���ⲿ�洢оƬ��ȡMP3���ݵĲ���
//	���Կ�����Ӧ�ø����Լ���ʵ��ʹ�õĴ洢оƬ��ȥ�޸��������
//	����Ҫ��֤����LD3320оƬ�Ĳ������ı�
//
//	LD_GetResult()����Ŀǰֻ��ȡ�˵�һ��ѡ�����Ϊʶ����
//	������Ӧ�ø����Լ���Ʒ��ƣ������Ƿ�Ҫ��ȡ������ʶ���ѡ���
/************************************************************************************/



void LD_reset()
{
	RSTB=1;
	delay(1);
	RSTB=0;
	delay(1);
	RSTB=1;

	delay(1);
	CSB=0;
	delay(1);
	CSB=1;
	delay(1);
}

void LD_Init_Common()
{
	bMp3Play = 0;

	LD_ReadReg(0x06);  
	LD_WriteReg(0x17, 0x35); 
	delay(10);
	LD_ReadReg(0x06);  

	LD_WriteReg(0x89, 0x03);  
	delay(5);
	LD_WriteReg(0xCF, 0x43);   
	delay(5);
	LD_WriteReg(0xCB, 0x02);
	
	/*PLL setting*/
	LD_WriteReg(0x11, LD_PLL_11);       
	if (nLD_Mode == LD_MODE_MP3)
	{
		LD_WriteReg(0x1E, 0x00); 
		LD_WriteReg(0x19, LD_PLL_MP3_19);   
		LD_WriteReg(0x1B, LD_PLL_MP3_1B);   
		LD_WriteReg(0x1D, LD_PLL_MP3_1D);
	}
	else
	{
		LD_WriteReg(0x1E,0x00);
		LD_WriteReg(0x19, LD_PLL_ASR_19); 
		LD_WriteReg(0x1B, LD_PLL_ASR_1B);		
	    LD_WriteReg(0x1D, LD_PLL_ASR_1D);
	}
	delay(10);
	
	LD_WriteReg(0xCD, 0x04);
	LD_WriteReg(0x17, 0x4c); 
	delay(5);
	LD_WriteReg(0xB9, 0x00);
	LD_WriteReg(0xCF, 0x4F); 
	LD_WriteReg(0x6F, 0xFF); 
}

void LD_Init_MP3()
{
	nLD_Mode = LD_MODE_MP3;
	LD_Init_Common();

	LD_WriteReg(0xBD,0x02);
	LD_WriteReg(0x17, 0x48);
	delay(10);

	LD_WriteReg(0x85, 0x52); 
	LD_WriteReg(0x8F, 0x00);  
	LD_WriteReg(0x81, 0x00);
	LD_WriteReg(0x83, 0x00);
	LD_WriteReg(0x8E, 0xff);
	LD_WriteReg(0x8D, 0xff);
    delay(1);
	LD_WriteReg(0x87, 0xff);
	LD_WriteReg(0x89, 0xff);
	delay(1);
	LD_WriteReg(0x22, 0x00);    
	LD_WriteReg(0x23, 0x00);
	LD_WriteReg(0x20, 0xef);    
	LD_WriteReg(0x21, 0x07);
	LD_WriteReg(0x24, 0x77);          
    LD_WriteReg(0x25, 0x03);
    LD_WriteReg(0x26, 0xbb);    
    LD_WriteReg(0x27, 0x01); 
}
	
void LD_Init_ASR()
{
	nLD_Mode=LD_MODE_ASR_RUN;
	LD_Init_Common();

	LD_WriteReg(0xBD, 0x00);
	LD_WriteReg(0x17, 0x48);
	delay( 10 );

	LD_WriteReg(0x3C, 0x80);    
	LD_WriteReg(0x3E, 0x07);
	LD_WriteReg(0x38, 0xff);    
	LD_WriteReg(0x3A, 0x07);
	
	LD_WriteReg(0x40, 0);          
	LD_WriteReg(0x42, 8);
	LD_WriteReg(0x44, 0);    
	LD_WriteReg(0x46, 8); 
	delay( 1 );
}


void ProcessInt0()
{
	uint8 nAsrResCount=0;

	EX0=0;
	
	ucRegVal = LD_ReadReg(0x2B);
	if(nLD_Mode == LD_MODE_ASR_RUN)
	{
		// ����ʶ��������ж�
		// �����������룬����ʶ��ɹ���ʧ�ܶ����жϣ�
		LD_WriteReg(0x29,0) ;
		LD_WriteReg(0x02,0) ;
		if((ucRegVal & 0x10) &&
			LD_ReadReg(0xb2)==0x21 && 
			LD_ReadReg(0xbf)==0x35)
		{
			nAsrResCount = LD_ReadReg(0xba);
			if(nAsrResCount>0 && nAsrResCount<=4) 
			{
				nAsrStatus=LD_ASR_FOUNDOK;
			}
			else
		    {
				nAsrStatus=LD_ASR_FOUNDZERO;
			}	
		}
		else
		{
			nAsrStatus=LD_ASR_FOUNDZERO;
		}
			
		LD_WriteReg(0x2b, 0);
    	LD_WriteReg(0x1C,0);
		return;
	}
	
	// �������Ų������жϣ������֣�
	// A. ����������ȫ�������ꡣ
	// B. ���������ѷ�����ϡ�
	// C. ����������ʱ��Ҫ���꣬��Ҫ�����µ����ݡ�	
	ucHighInt = LD_ReadReg(0x29); 
	ucLowInt=LD_ReadReg(0x02); 
	LD_WriteReg(0x29,0) ;
	LD_WriteReg(0x02,0) ;
    if(LD_ReadReg(0xBA)&CAUSE_MP3_SONG_END)
    {
	// A. ����������ȫ�������ꡣ

		LD_WriteReg(0x2B,  0);
      	LD_WriteReg(0xBA, 0);	
		LD_WriteReg(0xBC,0x0);	
		bMp3Play=0;					// ��������ȫ����������޸�bMp3Play�ı���
		LD_WriteReg(0x08,1);
		delay_2(5);
      	LD_WriteReg(0x08,0);
		LD_WriteReg(0x33, 0);

		return ;
     }

	 if(nMp3Pos>=nMp3Size)
	{
	// B. ���������ѷ�����ϡ�

		LD_WriteReg(0xBC, 0x01);
		LD_WriteReg(0x29, 0x10);
//		bMp3Play=0;				//	��ʱ��ֻ������MCU������MP3���ݷ��͵�LD3320оƬ�ڣ����ǻ�û�а����������ȫ���������
		EX0=1;

		return;	
	}

	// C. ����������ʱ��Ҫ���꣬��Ҫ�����µ����ݡ�	

	LD_ReloadMp3Data_2();
		
	LD_WriteReg(0x29,ucHighInt); 
	LD_WriteReg(0x02,ucLowInt) ;

	delay_2(10);
	EX0=1;

}




void LD_play()
{
	nMp3Pos=0;
	bMp3Play=1;

	if (nMp3Pos >=  nMp3Size)
		return ; 

	LD_ReloadMp3Data();

    LD_WriteReg(0xBA, 0x00);
	LD_WriteReg(0x17, 0x48);
	LD_WriteReg(0x33, 0x01);
	LD_WriteReg(0x29, 0x04);
	
	LD_WriteReg(0x02, 0x01); 
	LD_WriteReg(0x85, 0x5A);

	EX0=1;

}

void LD_AdjustMIX2SPVolume(uint8 val)
{
	ucSPVol = val;
	val = ((15-val)&0x0f) << 2;
	LD_WriteReg(0x8E, val | 0xc3); 
	LD_WriteReg(0x87, 0x78); 
}

void LD_ReloadMp3Data()
{
	uint32 nCurMp3Pos;
	uint8 val;
	uint8 k;

	nCurMp3Pos = nMp3StartPos + nMp3Pos;
	FLASH_CS=1;
	FLASH_CLK=0;
	FLASH_CS=0;

	 IO_Send_Byte(W25P_FastReadData);   
	 IO_Send_Byte(((nCurMp3Pos & 0xFFFFFF) >> 16));  
	 IO_Send_Byte(((nCurMp3Pos & 0xFFFF) >> 8));
	 IO_Send_Byte(nCurMp3Pos & 0xFF);
	 IO_Send_Byte(0xFF);

	ucStatus = LD_ReadReg(0x06);
	while ( !(ucStatus&MASK_FIFO_STATUS_AFULL) && (nMp3Pos<nMp3Size) )
	{
		val=0;
		for(k=0;k<8;k++)
		{
			FLASH_CLK=0;
			val<<=1;
			FLASH_CLK=1;
			val|=FLASH_DO;
		}
		LD_WriteReg(0x01,val);

		nMp3Pos++;

		ucStatus = LD_ReadReg(0x06);
	}
	
	FLASH_CS=1;
	FLASH_CLK=0;

}

/*********************************************************************
//Ϊ����������ʱ�� ���� *** WARNING L15: MULTIPLE CALL TO SEGMENT
//�����жϺ�������Ҫ���õĺ������¸��Ʋ�����
//�������� Warning L15�����ұ������ں���������ܴ���������Bug
/*********************************************************************/

void LD_ReloadMp3Data_2()
{
	uint32 nCurMp3Pos;
	uint8 val;
	uint8 k;

	nCurMp3Pos = nMp3StartPos + nMp3Pos;
	FLASH_CS=1;
	FLASH_CLK=0;
	FLASH_CS=0;

	 IO_Send_Byte(W25P_FastReadData);   
	 IO_Send_Byte(((nCurMp3Pos & 0xFFFFFF) >> 16));  
	 IO_Send_Byte(((nCurMp3Pos & 0xFFFF) >> 8));
	 IO_Send_Byte(nCurMp3Pos & 0xFF);
	 IO_Send_Byte(0xFF);

	ucStatus = LD_ReadReg(0x06);
	while ( !(ucStatus&MASK_FIFO_STATUS_AFULL) && (nMp3Pos<nMp3Size) )
	{
		val=0;
		for(k=0;k<8;k++)
		{
			FLASH_CLK=0;
			val<<=1;
			FLASH_CLK=1;
			val|=FLASH_DO;
		}
		LD_WriteReg(0x01,val);

		nMp3Pos++;

		ucStatus = LD_ReadReg(0x06);
	}
	
	FLASH_CS=1;
	FLASH_CLK=0;

}

// Return 1: success.
uint8 LD_Check_ASRBusyFlag_b2()
{
	uint8 j;
	uint8 flag = 0;
	for (j=0; j<10; j++)
	{
		if (LD_ReadReg(0xb2) == 0x21)
		{
			flag = 1;
			break;
		}
		delay(10);		
	}
	return flag;
}

void LD_AsrStart()
{
	LD_Init_ASR();
}

// Return 1: success.
uint8 LD_AsrRun()
{
	LD_WriteReg(0x35, MIC_VOL);
	LD_WriteReg(0x1C, 0x09);
	LD_WriteReg(0xBD, 0x20);
	LD_WriteReg(0x08, 0x01);
	delay( 1 );
	LD_WriteReg(0x08, 0x00);
	delay( 1 );

	if(LD_Check_ASRBusyFlag_b2() == 0)
	{
		return 0;
	}

	LD_WriteReg(0xB2, 0xff);	
	LD_WriteReg(0x37, 0x06);
	delay( 5 );
	LD_WriteReg(0x1C, 0x0b);
	LD_WriteReg(0x29, 0x10);
	
	LD_WriteReg(0xBD, 0x00);
	EX0=1;
	return 1;
}

// Return 1: success.
//	���ʶ��ؼ���������߿���ѧϰ"����ʶ��оƬLD3320�߽��ؼ�.pdf"�й��������������մ�����÷�
uint8 LD_AsrAddFixed()
{
	uint8 k, flag;
	uint8 nAsrAddLength;
	const char sRecog[5][13] = {"bei jing", "shou du", 
		"shang hai", "tian jin", "chong qing"};
		const uint8 pCode[5] = {CODE_BEIJING, CODE_BEIJING, CODE_SHANGHAI, CODE_TIANJIN, CODE_CHONGQING};
		
	flag = 1;
	for (k=0; k<5; k++)
	{
			
		if(LD_Check_ASRBusyFlag_b2() == 0)
		{
			flag = 0;
			break;
		}
		
		LD_WriteReg(0xc1, pCode[k] );
		LD_WriteReg(0xc3, 0 );
		LD_WriteReg(0x08, 0x04);
		delay(1);
		LD_WriteReg(0x08, 0x00);
		delay(1);

		for (nAsrAddLength=0; nAsrAddLength<20; nAsrAddLength++)
		{
			if (sRecog[k][nAsrAddLength] == 0)
				break;
			LD_WriteReg(0x5, sRecog[k][nAsrAddLength]);
		}
		LD_WriteReg(0xb9, nAsrAddLength);
		LD_WriteReg(0xb2, 0xff);
		LD_WriteReg(0x37, 0x04);
	}
    return flag;
}



uint8 LD_GetResult()
{
	return LD_ReadReg(0xc5 );
}




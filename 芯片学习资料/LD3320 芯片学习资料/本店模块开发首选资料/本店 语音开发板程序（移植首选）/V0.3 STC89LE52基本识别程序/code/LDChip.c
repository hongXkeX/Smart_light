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

extern void  delay(unsigned long uldata);

uint8 idata ucRegVal;
extern uint8 idata nAsrStatus;

void ProcessInt0(void);

/************************************************************************
���������� 	 ��λLDģ��
��ڲ�����	 none
�� �� ֵ�� 	 none
����˵����	 none
**************************************************************************/
void LD_Reset()
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
/************************************************************************
���������� LDģ�������ʼ��
��ڲ����� none
�� �� ֵ�� none
����˵���� �ú���Ϊ�������ã�һ�㲻��Ҫ�޸ģ�
					 ����Ȥ�Ŀͻ��ɶ��տ����ֲ������Ҫ�����޸ġ�
**************************************************************************/
void LD_Init_Common()
{
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

	LD_WriteReg(0x1E,0x00);
	LD_WriteReg(0x19, LD_PLL_ASR_19); 
	LD_WriteReg(0x1B, LD_PLL_ASR_1B);		
  LD_WriteReg(0x1D, LD_PLL_ASR_1D);
	delay(10);
	
	LD_WriteReg(0xCD, 0x04);
	LD_WriteReg(0x17, 0x4c); 
	delay(5);
	LD_WriteReg(0xB9, 0x00);
	LD_WriteReg(0xCF, 0x4F); 
	LD_WriteReg(0x6F, 0xFF); 
}

/************************************************************************
���������� 	 LDģ�� ASR���ܳ�ʼ��
��ڲ�����	 none
�� �� ֵ�� 	 none
����˵����	 �ú���Ϊ�������ã�һ�㲻��Ҫ�޸ģ�
					 ����Ȥ�Ŀͻ��ɶ��տ����ֲ������Ҫ�����޸ġ�
**************************************************************************/
void LD_Init_ASR()
{
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

/************************************************************************
���������� 	�жϴ�����
��ڲ�����	 none
�� �� ֵ�� 	 none
����˵����	��LDģ����յ���Ƶ�ź�ʱ��������ú�����
						�ж�ʶ���Ƿ��н�������û�д������ü�
            ����׼����һ�ε�ʶ��
**************************************************************************/
void ProcessInt0(void)
{
	uint8 nAsrResCount=0;

	EX0=0;
	ucRegVal = LD_ReadReg(0x2B);
	LD_WriteReg(0x29,0) ;
	LD_WriteReg(0x02,0) ;
	if((ucRegVal & 0x10) &&
		LD_ReadReg(0xb2)==0x21 && 
		LD_ReadReg(0xbf)==0x35)			/*ʶ��ɹ�*/
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
	}															 /*û��ʶ����*/
	else
	{	 
		nAsrStatus=LD_ASR_FOUNDZERO;
	}
		
  LD_WriteReg(0x2b, 0);
  LD_WriteReg(0x1C,0);/*д0:ADC������*/

	LD_WriteReg(0x29,0) ;
	LD_WriteReg(0x02,0) ;
	LD_WriteReg(0x2B,  0);
	LD_WriteReg(0xBA, 0);	
	LD_WriteReg(0xBC,0);	
	LD_WriteReg(0x08,1);	 /*���FIFO_DATA*/
	LD_WriteReg(0x08,0);	/*���FIFO_DATA�� �ٴ�д0*/


	EX0=1;
}

/************************************************************************
����������  ���LDģ���Ƿ����
��ڲ�����	none
�� �� ֵ�� 	flag��1-> ����
����˵����	none
**************************************************************************/
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
/************************************************************************
���������� 	����ASR
��ڲ�����	none
�� �� ֵ�� 	none
����˵����	none
**************************************************************************/
void LD_AsrStart()
{
	LD_Init_ASR();
}
/************************************************************************
���������� 	����ASR
��ڲ�����	none
�� �� ֵ�� 	1�������ɹ�
����˵����	none
**************************************************************************/
uint8 LD_AsrRun()
{
	EX0=0;
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
//	LD_WriteReg(0xB6, 0xa); //ʶ��ʱ��	 1S
//	LD_WriteReg(0xB5, 0x1E); //��������ʱ�� 300ms
//	LD_WriteReg(0xB8, 10); //����ʱ��

//	LD_WriteReg(0x1C, 0x07); //����˫ͨ����Ƶ�ź���Ϊ�����ź�
	LD_WriteReg(0x1C, 0x0b); //������˷���Ϊ�����ź�


	LD_WriteReg(0xB2, 0xff);
	delay( 1);	
	LD_WriteReg(0x37, 0x06);
	delay( 1 );
    LD_WriteReg(0x37, 0x06);
		delay( 5 );
	LD_WriteReg(0x29, 0x10);
	
	LD_WriteReg(0xBD, 0x00);
	EX0=1;
	return 1;
}
/************************************************************************
���������� ��LDģ����ӹؼ���
��ڲ����� none
�� �� ֵ�� flag��1->��ӳɹ�
����˵���� �û��޸�.
					 1���������¸�ʽ���ƴ���ؼ��ʣ�ͬʱע���޸�sRecog ��pCode ����ĳ���
					 �Ͷ�Ӧ����k��ѭ���á�ƴ������ʶ������һһ��Ӧ�ġ�
					 2�������߿���ѧϰ"����ʶ��оƬLD3320�߽��ؼ�.pdf"��
           ���������������մ�����÷������ṩʶ��Ч����
**************************************************************************/
uint8 LD_AsrAddFixed()
{
	uint8 k, flag;
	uint8 nAsrAddLength;
	 uint8 code sRecog[4][20] = {"","kai fa ban yan zheng", "dai ma ce shi","jian cha wan bi"};	/*��ӹؼ��ʣ��û��޸�*/
	 uint8 code pCode[4] = {0,CODE_KFBYZ, CODE_DMCS, CODE_JCWB};	/*���ʶ���룬�û��޸�*/	
	flag = 1;
	for (k=0; k<4; k++)
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
/************************************************************************
���������� 	��ȡʶ����
��ڲ�����	none
�� �� ֵ�� 	LD_ReadReg(0xc5 )��  ��ȡ�ڲ��Ĵ�������ʶ���롣
����˵����	none
**************************************************************************/
uint8 LD_GetResult()
{		
	return LD_ReadReg(0xc5 );
}




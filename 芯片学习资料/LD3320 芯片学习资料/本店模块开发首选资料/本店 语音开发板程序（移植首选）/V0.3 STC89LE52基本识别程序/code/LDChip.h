/************************************************************************************/
//	��Ȩ���У�Copyright (c) 2005 - 2010 ICRoute INC.
/************************************************************************************/


#ifndef LD_CHIP_H
#define LD_CHIP_H

#define uint8 unsigned char
#define uint16 unsigned int
#define uint32 unsigned long

//	��������״̬����������¼������������ASRʶ����������MP3����
//#define LD_MODE_IDLE		0x00
//#define LD_MODE_ASR_RUN		0x08
//#define LD_MODE_MP3		 	0x40


//	�������״̬����������¼������������ASRʶ������е��ĸ�״̬
#define LD_ASR_NONE				0x00	//	��ʾû������ASRʶ��
#define LD_ASR_RUNING			0x01	//	��ʾLD3320������ASRʶ����
#define LD_ASR_FOUNDOK			0x10	//	��ʾһ��ʶ�����̽�������һ��ʶ����
#define LD_ASR_FOUNDZERO 		0x11	//	��ʾһ��ʶ�����̽�����û��ʶ����
#define LD_ASR_ERROR	 		0x31	//	��ʾһ��ʶ��������LD3320оƬ�ڲ����ֲ���ȷ��״̬


#define CLK_IN   		22.1184	/* �û�ע���޸�����ľ���ʱ�Ӵ�С */
#define LD_PLL_11			(uint8)((CLK_IN/2.0)-1)
#define LD_PLL_MP3_19		0x0f
#define LD_PLL_MP3_1B		0x18
#define LD_PLL_MP3_1D   	(uint8)(((90.0*((LD_PLL_11)+1))/(CLK_IN))-1)

#define LD_PLL_ASR_19 		(uint8)(CLK_IN*32.0/(LD_PLL_11+1) - 0.51)
#define LD_PLL_ASR_1B 		0x48
#define LD_PLL_ASR_1D 		0x1f

// LD chip fixed values.
#define        RESUM_OF_MUSIC               0x01
#define        CAUSE_MP3_SONG_END           0x20

#define        MASK_INT_SYNC				0x10
#define        MASK_INT_FIFO				0x04
#define    	   MASK_AFIFO_INT				0x01
#define        MASK_FIFO_STATUS_AFULL		0x08


//��������
void LD_Reset();
void LD_Init_Common();
void LD_Init_ASR();
void LD_AsrStart();
uint8 LD_AsrRun();
uint8 LD_AsrAddFixed();
uint8 LD_GetResult();
extern uint8 idata  nLD_Mode;

//ʶ����ͻ��޸Ĵ�
#define CODE_DMCS	1
#define CODE_KFBYZ	2
#define CODE_JCWB	3


#define MIC_VOL 0x43	 //��ͷ����
#endif

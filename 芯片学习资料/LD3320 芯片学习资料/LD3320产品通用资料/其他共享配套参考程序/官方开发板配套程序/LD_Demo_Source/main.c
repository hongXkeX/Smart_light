/************************************************************************************/
//	��Ȩ���У�Copyright (c) 2005 - 2010 ICRoute INC.
/************************************************************************************/

#include "STC10F08XE.H"
#include "LDchip.h"
#include "Reg_RW.h"
#include "FlashDef.h"

sbit LED1=P3^4;
sbit LED2=P3^5;

/************************************************************************************/
//	nAsrStatus ������main�������б�ʾ�������е�״̬������LD3320оƬ�ڲ���״̬�Ĵ���
//	LD_ASR_NONE:		��ʾû������ASRʶ��
//	LD_ASR_RUNING��		��ʾLD3320������ASRʶ����
//	LD_ASR_FOUNDOK:		��ʾһ��ʶ�����̽�������һ��ʶ����
//	LD_ASR_FOUNDZERO:	��ʾһ��ʶ�����̽�����û��ʶ����
//	LD_ASR_ERROR:		��ʾһ��ʶ��������LD3320оƬ�ڲ����ֲ���ȷ��״̬
/***********************************************************************************/
uint8 idata nAsrStatus=0;	


/************************************************************************************/
//	���¶�������ʾ�������У��������ŵ��������ݵ���ʼλ�úͳ���
//	��Щ������������PC����¼�ƺõ�MP3�ļ�����PC���Ϻϳɵ�һ���ļ� voice.dat��
//	���԰����voide.dat�洢���û�ϵͳ�еĴ洢оƬ�У����� spi-flash��
//	����Ҫ����ʱ���û�������MCU���Ե�spi-flash�и���Ҫ�����ļ�����ʼ��ַ�޶�ȡMP3����
//	������LD3320���в���
/************************************************************************************/
#define MP3_BEIJING_START	0x0000;
#define MP3_BEIJING_SIZE	0x08b8;
#define MP3_SHANGHAI_START	0x1000;
#define MP3_SHANGHAI_SIZE	0x0ab0;
#define MP3_TIANJIN_START	0x2000;
#define MP3_TIANJIN_SIZE	0x0a20;
#define MP3_CHONGQING_START	0x3000;
#define MP3_CHONGQING_SIZE	0x0990;
#define MP3_DING_START	0x4000;
#define MP3_DING_SIZE	0x0828;
#define MP3_NIHAO_START	0x5000;
#define MP3_NIHAO_SIZE	0x1170;


extern void          _nop_     (void);

void MCU_init();
void FlashLED(uint8 nTimes);
void PlaySound(uint8 nCode);
uint8 RunASR();
void ProcessInt0();


/************************************************************************************/
//	���ʾ�������ǻ���STC10L08XE����������أ�����ִ�еĳ���
//	������ʶ��Ͳ��ŵĹ���
//	�����û�˵����ͬ�Ĺؼ����ʶ��󲥷Ŷ�Ӧ��mp3����
//
//	��LD3320оƬ�йصĺ������ܣ�����"�����ֲ�.pdf"������ϸ˵��
//	�뿪���߶���"�����ֲ�.pdf���Ķ�
//	
//	��������ѧϰLD3320��ʼʱ������ֱ�Ӱ������main�����������Լ���Ƭ���б�������
//	���ֻʵ��ASR����ʶ���ܣ����԰ѳ�����PlaySound()�����ĵ���ע�͵��Ϳ���
//	
//	��Ȼ���û�Ҫ�����Լ�ʵ�ʵĵ�Ƭ��ȥ�޸� mcu_init ���Լ���Ӧ�Ĺܽ����ӵĶ��壬�ж϶���
/************************************************************************************/

void  main()
{
	uint8 idata nAsrRes;

	FlashLED(3);

	MCU_init();
	LD_reset();

	nAsrStatus = LD_ASR_NONE;		//	��ʼ״̬��û������ASR
	PlaySound(CODE_DEFAULT);		//	������ʾ��
	while(1)
	{
		if (bMp3Play!=0)			//	������ڲ���MP3�У���ȴ���ֱ��MP3������� ��bMp3Play==0
			continue;				//	bMp3Play �Ƕ����һ��ȫ�ֱ���������¼MP3���ŵ�״̬������LD3320оƬ�ڲ��ļĴ���

		switch(nAsrStatus)
		{
			case LD_ASR_RUNING:
			case LD_ASR_ERROR:		
				break;
			case LD_ASR_NONE:
			{
				FlashLED(4);
				nAsrStatus=LD_ASR_RUNING;
				if (RunASR()==0)	//	����һ��ASRʶ�����̣�ASR��ʼ����ASR��ӹؼ��������ASR����
				{
					nAsrStatus = LD_ASR_ERROR;
					LED1=0;
					LED2=0;
				}
				break;
			}
			case LD_ASR_FOUNDOK:
			{
				FlashLED(2);
				nAsrRes = LD_GetResult();	//	һ��ASRʶ�����̽�����ȥȡASRʶ����
				PlaySound(nAsrRes);
				nAsrStatus = LD_ASR_NONE;
				break;
			}
			case LD_ASR_FOUNDZERO:
			default:
			{
				FlashLED(1);
				PlaySound(CODE_DEFAULT);
				nAsrStatus = LD_ASR_NONE;
				break;
			}
		}// switch
	}// while

}

void MCU_init()
{
	P1M0 |= 1;
	P3M0 |= 8;

	LED1=0;
	LED2=0;
	P0 = 0xff;
	P1 = 0xff;
	P2 = 0xff;
	P3 = 0xf7;
	P4 = 0x0f;

#if defined (SOFT_SPI_PORT)		//	���ģ��SPI��д
	LD_MODE = 1;				//	����MD�ܽ�Ϊ��
#elif defined (HARD_SPI_PORT)	//	Ӳ��ʵ��SPI��д
	LD_MODE = 1;				//	����MD�ܽ�Ϊ��
#else							//	���ж�д ��Ӳ��ʵ�ֻ������ģ�⣩
	LD_MODE = 0;				//	����MD�ܽ�Ϊ��
#endif


	IP=0;
	IPH=0;

	PX0=1; 
	PT0=1;
	PS=1;
	IPH |= 1;	
	
	EX0=0;
	EX1=0;
	EA=1;
}

void  delay(unsigned long uldata)
{
	unsigned int j  =  0;
	unsigned int g  =  0;
	for (j=0;j<5;j++)
	{
		for (g=0;g<uldata;g++)
		{
			_nop_();
			_nop_();
			_nop_();
		}
	}
}

/*********************************************************************
//Ϊ����������ʱ�� ���� *** WARNING L15: MULTIPLE CALL TO SEGMENT
//�����жϺ�������Ҫ���õĺ������¸��Ʋ�����
//�������� Warning L15�����ұ������ں���������ܴ���������Bug
/*********************************************************************/
void  delay_2(unsigned long uldata)
{
	unsigned int j  =  0;
	unsigned int g  =  0;
	for (j=0;j<5;j++)
	{
		for (g=0;g<uldata;g++)
		{
			_nop_();
			_nop_();
			_nop_();
		}
	}
}

void FlashLED(uint8 nTimes)
{
	uint8 k;
	for (k=0; k<nTimes; k++)
	{
		LED1=0;
		LED2=0;
		delay(15000);
		LED1=1;
		LED2=1;
		delay(15000);
	}
}

void PlaySound(uint8 nCode)
{
	switch(nCode)
	{
	case CODE_BEIJING:
		nMp3StartPos = MP3_BEIJING_START;
		nMp3Size = MP3_BEIJING_SIZE;
		break;
	case CODE_SHANGHAI:
		nMp3StartPos = MP3_SHANGHAI_START;
		nMp3Size = MP3_SHANGHAI_SIZE;
		break;
	case CODE_TIANJIN:
		nMp3StartPos = MP3_TIANJIN_START;
		nMp3Size = MP3_TIANJIN_SIZE;
		break;
	case CODE_CHONGQING:
		nMp3StartPos = MP3_CHONGQING_START;
		nMp3Size = MP3_CHONGQING_SIZE;
		break;
	default:
		nMp3StartPos = MP3_NIHAO_START;
		nMp3Size = MP3_NIHAO_SIZE;
		break;		
	}

	LD_Init_MP3();
	LD_AdjustMIX2SPVolume(15);
	LD_play();
}


/************************************************************************************/
//	IO_Send_Byte()������MCU��spi-flash (�ͺ�Ϊ����� W25X40)����ָ��
//	������Ӧ�ø����Լ�ʹ�õĴ洢оƬȥд����ĺ���
//
//	ICroute��˾�޷��Դ洢оƬ�Ķ�д�ṩ����֧�֣�
//	��������Ҫ�Լ����Լ�ʹ�õĴ洢оƬ�ĳ��̻�ü���֧��
//	������������������ش�����ĵ�
//
//	�����spi-flash�ļ���֧�ֵ��ĵ�ҳ���ڣ�
//	http://www.winbond.com.tw/hq/cht/ProductAndSales/ProductLines/FlashMemory/SerialFlash/
//	http://www.xtdpj.com/show_hdr.php?xname=915RL41&dname=23T0M41&xpos=84
/************************************************************************************/

void IO_Send_Byte(uint8 dataout)
{
	 uint8 i = 0; 
	 FLASH_CS = 0;
	 for (i=0; i<8; i++)
	 {
		  if ((dataout & 0x80) == 0x80) 
			   FLASH_DIO = 1;
		  else
			   FLASH_DIO = 0;
		  FLASH_CLK = 1;
		  dataout = (dataout << 1); 
		  FLASH_CLK = 0;   
	 }
}


/************************************************************************************/
//	RunASR()����ʵ����һ��������ASR����ʶ������
//	LD_AsrStart() ����ʵ����ASR��ʼ��
//	LD_AsrAddFixed() ����ʵ������ӹؼ����ﵽLD3320оƬ��
//	LD_AsrRun()	����������һ��ASR����ʶ������
//
//	�κ�һ��ASRʶ�����̣�����Ҫ�������˳�򣬴ӳ�ʼ����ʼ����
/************************************************************************************/

uint8 RunASR()
{
	uint8 i=0;
	uint8 asrflag=0;
	for (i=0; i<5; i++)			//	��ֹ����Ӳ��ԭ����LD3320оƬ����������������һ������5������ASRʶ������
	{
		LD_AsrStart();
		delay(100);
		if (LD_AsrAddFixed()==0)
		{
			LD_reset();			//	LD3320оƬ�ڲ����ֲ���������������LD3320оƬ
			delay(100);			//	���ӳ�ʼ����ʼ����ASRʶ������
			continue;
		}
		delay(10);
		if (LD_AsrRun() == 0)
		{
			LD_reset();			//	LD3320оƬ�ڲ����ֲ���������������LD3320оƬ
			delay(100);			//	���ӳ�ʼ����ʼ����ASRʶ������
			continue;
		}

		asrflag=1;
		break;					//	ASR���������ɹ����˳���ǰforѭ������ʼ�ȴ�LD3320�ͳ����ж��ź�
	}

	return asrflag;
}

void ExtInt0Handler(void) interrupt 0  
{
	ProcessInt0();				//	LD3320 �ͳ��ж��źţ�����ASR�Ͳ���MP3���жϣ���Ҫ���жϴ������зֱ���
}




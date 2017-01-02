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

uint8 idata nDemoFlag=0;


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

void PlayDemoSound_Once();
void PlayDemoSound_Continue();


/************************************************************************************/
//	���ʾ�������ǻ���STC10L08XE����������أ�����ִ�еĳ���
//	ʵ���˴������в��ŵĹ���
//  
//  verygood_withHead.mp3(verygood_withoutHead)��һ��MP3��ʽ�������ļ�����������ΪӢ�ĵ���"very good"
//	demosound.h �ж��������� bpDemoSound[]��
//  ��������ݾ��� verygood_withHead.mp3(verygood_withoutHead)�ļ�������
//  �û�������16�����Ķ��������� UltraEdit ����verygood_withHead.mp3�ļ��� bpDemoSound ���������
//
//	��������MP3�ļ��Ĳ��죺  
//	������MP3�ļ�ֻ����û��ID3�ļ�ͷ��MP3�����ݶ���һ���ģ�
//	ID3�ļ�ͷ����LDоƬ���Ӱ�첥��
//  ��Windows�Դ���MediaPlayer����û��ID3ͷ��vertygood_withoutHead.mp3 �󣬻��Զ�����ID3�ļ�ͷ
//  PlayDemoSound_Once/PlayDemoSound_Continue �������ǰ� bpDemoSound�����е��������δ���LD3320оƬ���в���
//
//  ��LD3320оƬ�йصĺ������ܣ�����"�����ֲ�.pdf"������ϸ˵��
//	�뿪���߶���"�����ֲ�.pdf���Ķ�
//	
//	��������ѧϰLD3320��ʼʱ������ֱ�Ӱ������main�����������Լ���Ƭ���б�������
//	
//	��Ȼ���û�Ҫ�����Լ�ʵ�ʵĵ�Ƭ��ȥ�޸� mcu_init ���Լ���Ӧ�Ĺܽ����ӵĶ��壬�ж϶���
/************************************************************************************/

void  main()
{
//	uint8 idata nAsrRes;
	uint8 nTemp;

	FlashLED(3);

	MCU_init();
	LD_reset();

	// ����2��ֻ����һ����Ч��
	// ���ֱܷ��ǲ���һ�κ�������ͣ���š�
	//PlayDemoSound_Once();		//	֮����һ��
	PlayDemoSound_Continue();	//	����ѭ������

	while(1)
	{
		nTemp++;
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

/************************************************************************************/
// ��2�������ṹ��ͬ��ֻ�Ƕ���ֹ������һ������
// nDemoFlag ��һ����־ֵ�����=1����ִ�д��ڴ�������ݣ�����ȥ��Flash�ˡ�
// �������ݺ���LD_ReloadMp3Data()������´��룺
//			val = bpDemoSound[nMp3Pos++];
//			LD_WriteReg(0x01,val);
//			if (nMp3Pos == DEMO_SOUND_SIZE)
//				nMp3Pos = 0;
// ����� nMp3Pos �ۼӺ���� DEMO_SOUND_SIZE ʱ������Ϊ0��
// ������� nMp3Size ��Ϊ DEMO_SOUND_SIZE�� ����Զ��������
//		 if(nMp3Pos>=nMp3Size) {
//			// B. ���������ѷ�����ϡ�
// �����ͻ�һֱ������ȥ��
//
// ��� nMp3Size ��С����һ�β����������������ˣ����ž�ֹͣ�ˡ�
/************************************************************************************/

//#define DEMO_SOUND_SIZE 851	// ��Ӧvertygood_withoutHead.mp3 ���MP3�ļ�û��ID3���ļ�ͷ
#define DEMO_SOUND_SIZE 1235	// ��Ӧverygood_withHead.mp3 ���MP3�ļ���ID3���ļ�ͷ
								//	������MP3�ļ�ֻ����û��ID3�ļ�ͷ��MP3�����ݶ���һ���ģ�
								//	ID3�ļ�ͷ����LDоƬ���Ӱ�첥��
								//  ��Windows�Դ���MediaPlayer����û��ID3ͷ��vertygood_withoutHead.mp3 �󣬻��Զ�����ID3�ļ�ͷ

void PlayDemoSound_Once()
{
	nDemoFlag = 1;
	nMp3StartPos = 0;
	nMp3Size = DEMO_SOUND_SIZE-1;
	LD_Init_MP3();
	LD_AdjustMIX2SPVolume(7);
	LD_play();
}

void PlayDemoSound_Continue()
{
	nDemoFlag = 1;
	nMp3StartPos = 0;
	nMp3Size = DEMO_SOUND_SIZE;
	LD_Init_MP3();
	LD_AdjustMIX2SPVolume(7);
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


void ExtInt0Handler(void) interrupt 0  
{
	ProcessInt0();				//	LD3320 �ͳ��ж��źţ�����ASR�Ͳ���MP3���жϣ���Ҫ���жϴ������зֱ���
}




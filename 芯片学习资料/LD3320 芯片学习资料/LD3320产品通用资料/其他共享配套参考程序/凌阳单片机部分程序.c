#include <3320.h>
#include <sound.h>

uint8  nLD_Mode = LD_MODE_IDLE;		//	������¼��ǰ���ڽ���ASRʶ�����ڲ���MP3						//	������¼����MP3��״̬
uint8  ucRegVal;
uint8  ucHighInt;
uint8  ucLowInt;
uint8  ucStatus;
uint8  ucSPVol=15; // MAX=15 MIN=0 	//	Speaker�������������

extern uint8 nAsrStatus;

void  delay(unsigned int uldata)
{
	unsigned int j  =  0;
	unsigned int g  =  0;
	for (j=0;j<5;j++)
	{
		for (g=0;g<uldata;g++)
		{
			__asm("nop");
			__asm("nop");
			__asm("nop");
			*P_Watchdog_Clear=0x0001;
		}
	}
}

void LD_reset()
{
	*P_IOB_Data=0xff1f;//IOB13(RST) TO LOW
	delay(2);
	*P_IOB_Data=0xff5f;//RST TO high
}

unsigned char LD_ReadReg( unsigned char address )
{
	unsigned char idata;
	
	address=address<<4;
	address=address<<4;
	address=address&0xff00;
	*P_IOB_Data=0x005f;
	*P_IOB_Data=0x005c|address;		//configure IOB3(A0) to high output, IOB1(CS) IOB0(WR) to low output
	asm("nop");
	*P_IOB_Data=0x005f|address;		//configure IOB1(CS) IOB0(WR) to HIGH output
	asm("nop");

	*P_IOB_Dir=0x00ff;			//configure IOB8~IOB15 to input
	*P_IOB_Attrib=0x00ff;
	*P_IOB_Data=0xff45;		//data operation configure IOB1(CS) IOB4(RD) to low output
	asm("nop");
	idata=*P_IOB_Data;
	*P_IOB_Data=0x0057;		//configure configure IOB1(CS) IOB4(RD) to HIGH output
	asm("nop");
	
	*P_IOB_Dir=0xffff;			//configure to output
	*P_IOB_Attrib=0xffff;
	*P_IOB_Data=0x005f;
	
	idata=idata>>4;
	idata=idata>>4;
	idata=idata&0x00ff;
	return idata;
}

void LD_WriteReg(unsigned char address, unsigned char dataout)
{
	address=address<<4;
	address=address<<4;
//	address=address&0xff00;
	*P_IOB_Data=0x005f;
	
	*P_IOB_Data=0x005c|address;		//configure IOB1(CS) IOB0(WR) to low output, to write the address
	asm("nop");
	*P_IOB_Data=0x005f|address;		//configure IOB1(CS) IOB0(WR) to HIGH output
//	asm("nop");

	dataout=dataout<<4;
	dataout=dataout<<4;
//	dataout=dataout&0xff00;
	*P_IOB_Data=0x005f;
	
	*P_IOB_Data=0x054|dataout;		//configure IOB1(CS) IOB0(WR) to low output
//	asm("nop");
	*P_IOB_Data=0x0057|dataout;		//configure IOB1(CS) IOB0(WR) to HIGH output
//	asm("nop");
}

void LD_Init_Common()
{
	LD_ReadReg(0x06);  
	LD_WriteReg(0x17, 0x35); //д35H ��LD3320 ������λ��Soft Reset��
	delay(10);
	LD_ReadReg(0x06);  

	LD_WriteReg(0x89, 0x03);  //ģ���·����   ��ʼ��ʱд 03H 
	delay(5);
	LD_WriteReg(0xCF, 0x43);   // �ڲ�ʡ��ģʽ���� ,��ʼ��ʱ д�� 43H 
	delay(5);
	LD_WriteReg(0xCB, 0x02);	//��ȡ ASR������� 4��
		
	/*PLL setting*/
	LD_WriteReg(0x11, LD_PLL_11);		//ʱ��Ƶ������1
	if (nLD_Mode == LD_MODE_MP3)
	{
		LD_WriteReg(0x1E, 0x00); 
		LD_WriteReg(0x19, LD_PLL_MP3_19);	
		LD_WriteReg(0x1B, LD_PLL_MP3_1B);	
		LD_WriteReg(0x1D, LD_PLL_MP3_1D);
	}
	else
	{
		LD_WriteReg(0x1E,0x00); //ADC ר�ÿ��ƣ�Ӧ��ʼ��Ϊ 00H 
		LD_WriteReg(0x19, LD_PLL_ASR_19); //ʱ��Ƶ������ 2 
		LD_WriteReg(0x1B, LD_PLL_ASR_1B);//ʱ��Ƶ������3	
		LD_WriteReg(0x1D, LD_PLL_ASR_1D);//ʱ��Ƶ������ 4
	}
	delay(10);
	
	LD_WriteReg(0xCD, 0x04);//DSP �������� �� ��ʼ��ʱд�� 04H ����DSP ����
	LD_WriteReg(0x17, 0x4c); //д4C ����ʹDSP ���ߣ��Ƚ�ʡ��
	delay(5);					//0xb9 ��ǰ���ʶ�����ַ������ȣ�ƴ���ַ�����
	LD_WriteReg(0xB9, 0x00);//,��ʼ��ʱд�� 00H,ÿ���һ��ʶ����Ҫ�趨һ��
	LD_WriteReg(0xCF, 0x4F); // MP3 ��ʼ���� ASR��ʼ��ʱд�� 4FH 
	LD_WriteReg(0x6F, 0xFF); //��оƬ���г�ʼ��ʱ����Ϊ 0xFF 
	
	//*P_Watchdog_Clear=0x0001;
}
		
void LD_Init_ASR()
{
	nLD_Mode=LD_MODE_ASR_RUN;
	LD_Init_Common();

	LD_WriteReg(0xBD, 0x00);	//д�� 00H��Ȼ��������Ϊ ASR ģ�飻
	LD_WriteReg(0x17, 0x48);	//д48H ���Լ��� DSP�� 
	delay( 10 );

	LD_WriteReg(0x3C, 0x80);	//FIFO_EXT ���޵� 8λ��LowerBoundary L��
	LD_WriteReg(0x3E, 0x07);	//FIFO_EXT ���޸� 8λ��LowerBoundary H��
	LD_WriteReg(0x38, 0xff);   //FIFO_EXT ���޵� 8λ��UpperBoundary L��
	LD_WriteReg(0x3A, 0x07);	//FIFO_EXT���޸� 8λ��UpperBoundary H�� 
	
	LD_WriteReg(0x40, 0);			//FIFO_EXT MCUˮ�ߵ� 8λ��MCU water mark L�� 
	LD_WriteReg(0x42, 8);		//FIFO_EXT MCUˮ�߸� 8λ��MCU water mark H��
	LD_WriteReg(0x44, 0);		//FIFO_EXT DSPˮ�ߵ� 8λ��DSP water mark L�� 
	LD_WriteReg(0x46, 8);		//FIFO_EXT DSPˮ�߸� 8λ��DSP water mark H��
	delay( 1 );
}

	
void ProcessInt0()
{
	uint8 nAsrResCount=0;

	asm("irq off");
	ucRegVal = LD_ReadReg(0x2B);
	if(nLD_Mode == LD_MODE_ASR_RUN)
	{
		// ����ʶ��������ж�
		// �����������룬����ʶ��ɹ���ʧ�ܶ����жϣ�
		LD_WriteReg(0x29,0) ;//clear
		LD_WriteReg(0x02,0) ;//clear
		if((ucRegVal & 0x10) &&LD_ReadReg(0xb2)==0x21 && LD_ReadReg(0xbf)==0x35)
		{
			nAsrResCount = LD_ReadReg(0xba);
			if(nAsrResCount>0 && nAsrResCount<=4) 
				nAsrStatus=LD_ASR_FOUNDOK;
			else
				nAsrStatus=LD_ASR_FOUNDZERO;
		}
		else
			nAsrStatus=LD_ASR_FOUNDZERO;

		LD_WriteReg(0x2b, 0);//clear???/
		LD_WriteReg(0x1C,0);// д 00H ADC������
		asm("irq on");
		return;
	}	
}

// Return 1: success.
uint8 LD_Check_ASRBusyFlag_b2()
{
	uint8 j;
	uint8 flag = 0;
	for (j=0; j<10; j++)
	{
		if (LD_ReadReg(0xb2) == 0x21)// 0x21 ��ʾ�У���ѯ��Ϊ��״̬���Խ�����һ��ASR ����
		{
			flag = 1;
			break;
		}
		delay(10);
		*P_Watchdog_Clear=0x0001;
	}
	return flag;
}
	
// Return 1: success.
uint8 LD_AsrRun()
{
	LD_WriteReg(0x35, MIC_VOL);//ADC ���棬��������Ϊ��˷磨MIC������
	LD_WriteReg(0x1C, 0x09);	// д 09H Reserve ���������֣�����ʹ�ð��ո����Ĳο��������ʹ��
	LD_WriteReg(0xBD, 0x20);	//д�� 02H��Ȼ��������Ϊ MP3 ģ�飻
	LD_WriteReg(0x08, 0x01);	//��0 λ��д��1�����FIFO_DATA ,��2 λ��д��1�����FIFO_EXT 
	delay( 1 );
	LD_WriteReg(0x08, 0x00);	//���ָ�� FIFO ����д��һ��00H
	delay( 1 );

	if(LD_Check_ASRBusyFlag_b2() == 0)
		return 0;

	LD_WriteReg(0xB2, 0xff);	// 0x21 ��ʾ�У���ѯ��Ϊ��״̬���Խ�����һ��ASR ����
	LD_WriteReg(0x37, 0x06);	//д06H��֪ͨDSP ��ʼʶ�������
	LD_WriteReg(0x37, 0x06);
	delay( 5 );
	//LD_ReadReg(0x37);
	//LD_ReadReg(0xbf);
	LD_WriteReg(0x1C, 0x0b);	// д 0BH ��˷����� ADCͨ������ 
	LD_WriteReg(0x29, 0x10);	//�� 4λ��ͬ���ж�����1 ��ʾ����0��ʾ������
	
	LD_WriteReg(0xBD, 0x00);	//д�� 00H��Ȼ��������Ϊ ASR ģ�飻 
	//EX0=1;
	
	//*P_INT_Ctrl|=C_IRQ3_EXT1;
	//*P_INT_Ctrl|=0x0004;
	//__asm("IRQ ON");
	return 1;
}

uint8 RunASR()
{
	uint8 i=0;
	uint8 asrflag=0;
	for (i=0; i<5; i++)			//	��ֹ����Ӳ��ԭ����LD3320оƬ����������������һ������5������ASRʶ������
	{
		//*P_Watchdog_Clear=0x0001;
		LD_Init_ASR();
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

// Return 1: success.
//	���ʶ��ؼ���������߿���ѧϰ"����ʶ��оƬLD3320�߽��ؼ�.pdf"�й��������������մ�����÷�
uint8 LD_AsrAddFixed()
{
	uint8 k, flag;
	uint8 nAsrAddLength;
	const char sRecog[22][20] = {"yi","yi","er","san","si","wu","liu","qi","ba","jiu","ling",
		"kai ji", "guan ji", "jing yin", "xuan tai","jie mu jia","jie mu jian","da sheng dian","xiao sheng dian","huang zhong hou","a a","en en"};
	const uint8 pCode[22] = {YI,YI,ER,SAN,SI,WU,LIU,QI,BA,JIU,LING,CODE_KAIJI, CODE_GUANJI, CODE_JINGYIN,
		CODE_XUANTAI,CODE_JIEMUJIA,CODE_JIEMUJIAN,CODE_DASD,CODE_XIAOSD,CODE_HZHH,CODE_NONE,CODE_NONE};
			
	flag = 1;
	for (k=0; k<22; k++)
	{
			
		if(LD_Check_ASRBusyFlag_b2() == 0)
		{
			flag = 0;
			break;
		}
			
		LD_WriteReg(0xc1, pCode[k] );//ASR��ʶ���� Index��00H��FFH��
		LD_WriteReg(0xc3, 0 );			//ASR��ʶ�������ʱд�� 00 
		LD_WriteReg(0x08, 0x04);		//��2 λ��д��1�����FIFO_EXT 
		delay(1);
		LD_WriteReg(0x08, 0x00);		//���ָ�� FIFO ����д��һ��00H
		delay(1);
	
		for (nAsrAddLength=0; nAsrAddLength<20; nAsrAddLength++)
		{
			if (sRecog[k][nAsrAddLength] == 0)
				break;
			LD_WriteReg(0x5, sRecog[k][nAsrAddLength]);//FIFO_EXT ���ݿ� 
			*P_Watchdog_Clear=0x0001;
		}
		LD_WriteReg(0xb9, nAsrAddLength);//��ǰ���ʶ�����ַ������ȣ�ƴ���ַ�����
		LD_WriteReg(0xb2, 0xff);		//DSPæ��״̬
		LD_WriteReg(0x37, 0x04);		//д04H��֪ͨDSP Ҫ���һ��ʶ��䡣 
	}
	return flag;
}

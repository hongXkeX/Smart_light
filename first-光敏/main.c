/*-----------------------------------------------
  ���ƣ�IICЭ�� PCF8591 AD/DAת�� ���Թ�������
  �޸ģ���
  ���ݣ�ʹ��4·AD�е�1·����ⲿģ�������룬��ʾ0-255��ֵ
------------------------------------------------*/  
#include <reg52.h>                
#include "i2c.h"
#include "delay.h"
#include "display.h"

#define AddWr 0x90   //д���ݵ�ַ 
#define AddRd 0x91   //�����ݵ�ַ

#define uint unsigned int
#define uchar unsigned char   

uchar num=0;

sbit ir=P3^2;  

bit irok;
bit irprosok;
bit startflag = 0;
uchar irtime;
uchar bitnum;
uchar ircode[4];
uchar irdata[33];

sbit p20 = P2^0;
sbit p21 = P2^1;

extern bit ack;
unsigned char ReadADC(unsigned char Chl);
bit WriteDAC(unsigned char dat);

void int0init()  //�ⲿ�ж�0��ʼ��
{
	IT0 = 1;   //ָ���ⲿ�ж�0�½��ش�����INT0 (P3.2)
	EA = 1;    //�����ж�
	EX0 = 1;   //ʹ���ⲿ�ж�
}

void timer1init()//��ʱ��0��ʼ�� 256*(1/12m)*12=0.256ms
{
	TMOD |= 0x20;//��ʱ��0������ʽ2��TH0����װֵ��TL0�ǳ�ֵ
	TH1 = 0x00; //����ֵ
	TL1 = 0x00; //��ʼ��ֵ
	EA = 1;
	ET1 = 1;    //���ж�
	TR1 = 1;    
}

/*------------------------------------------------
              ������ֵ������  
	  �ӵ�λ��ʼ����  	���ÿ�ε�ircode[4]             
------------------------------------------------*/
void ir_pros()                  // ������ֵ����
{
	uchar mun,k,i,j;
	k=1;
	for(j=0; j<4; j++)
	{
		for(i=0; i<8; i++)
		{
			mun = mun >> 1;
			if(irdata[k] > 6)
			{
				mun = mun | 0x80;
			}
			k++;
		}
		ircode[j] = mun;
		mun = 0;
	}
	irprosok = 1;
}

/*------------------------------------------------
              �����ֵ������  
	   ����ircode[2]    ��ʾ��Ӧ�����            
------------------------------------------------*/
void ir_work() 						     	// �����ֵ����
{
	if( ircode[2] == 0x0c ) {
		TempData[5]=dofly_DuanMa[1]; 
	} 
	if( ircode[2] == 0x18 ) {         // 2 �ֶ�ģʽ �ص�
		TempData[5]=dofly_DuanMa[2]; 
		led0 = 1;
		led7 = 1;
		led6 = 0;
		led5 = 1;
	}
	if( ircode[2] == 0x5e ) {         // 2 �ֶ�ģʽ ���� 
		TempData[5]=dofly_DuanMa[3];
		led0 = 0;
		led7 = 1;
		led6 = 0;
		led5 = 1;
	}
	if( ircode[2] == 0x08 ) {
		TempData[5]=dofly_DuanMa[4];
		led7 = 1;
		led6 = 1;
		led5 = 0;
	}
	if( ircode[2] == 0x1c ) {
		TempData[5]=dofly_DuanMa[5];
	}
	if( ircode[2] == 0x5a ) {
		TempData[5]=dofly_DuanMa[6];
	}
	if( ircode[2] == 0x42 ) {
		TempData[5]=dofly_DuanMa[7];
	}
	if( ircode[2] == 0x52 ) {
		TempData[5]=dofly_DuanMa[8];
	}
	if( ircode[2] == 0x4a ) {
		TempData[5]=dofly_DuanMa[9];
	}
	
}

/*------------------------------------------------
              ������
------------------------------------------------*/
main()
{
	Init_Timer0();
	int0init();    // �ⲿ�жϳ�ʼ��
	timer1init();  // ��ʱ��1��ʼ��
	
	TempData[5]=dofly_DuanMa[0];

	while (1)         //��ѭ��
	{
		num = 255 - ReadADC(3);  //ֵȡ��ֵ��������ʾ��ǿԽС����ֵԽС
		if(irok == 1)            //������������պ���
	    {   			  
			irok = 0;
	    	ir_pros();           //���к�����ֵ����
	    }
		if(irprosok == 1)        //���������ֵ����ú�,
	    {
			irprosok = 0;		
	    	ir_work();           //���й�������
  	  	}
		
		if( ircode[2] == 0x0c ) {   // 1 �Զ�ģʽ
			led7 = 0;
			led5 = 1;
			led6 = 1;
			if( num < 80 ) {
				led0 = 0;
			}
			if( num > 80 ) {
				led0 = 1;
			}
			TempData[5]=dofly_DuanMa[1]; 
		}

		if( ircode[2] == 0x08 ) {   // 4 ����ģʽ
			if ( led4 == 0 ) {
				led0 = 0;
			}
			if ( led4 == 1 ) {
				led0 = 1;
			}
		}
		
		TempData[0]=dofly_DuanMa[num/100];    
		TempData[1]=dofly_DuanMa[(num%100)/10];
		TempData[2]=dofly_DuanMa[(num%100)%10];
		//��ѭ�������������Ҫһֱ�����ĳ���
		DelayMs(100);
	}
}
/*------------------------------------------------
             ��ADתֵ����
������� Chl ��ʾ��Ҫת����ͨ������Χ��0-3
����ֵ��Χ0-255
------------------------------------------------*/
unsigned char ReadADC(unsigned char Chl)
 {
   unsigned char Val;
   Start_I2c();               //��������
   SendByte(AddWr);             //����������ַ
     if(ack==0)return(0);
   SendByte(0x40|Chl);            //���������ӵ�ַ
     if(ack==0)return(0);
   Start_I2c();
   SendByte(AddWr+1);
      if(ack==0)return(0);
   Val=RcvByte();
   NoAck_I2c();                 //���ͷ�Ӧλ
   Stop_I2c();                  //��������
  return(Val);
 }
 
/*------------------------------------------------
       �ⲿ�ж�0   ��ɴ洢һ�����ݵ�ÿλ
------------------------------------------------*/
void int0() interrupt 0   //�ⲿ�ж�0������
{
	if(startflag)
 	{
  		if(irtime>29 && irtime<58)  //8-16ms
		{
	   		bitnum = 0;
		}
		irdata[bitnum] = irtime;
		irtime = 0;
		bitnum++;
		if(bitnum == 33)
	  	{
	   		bitnum=0;
	   		irok=1;
	  	}
 	}
	else
 	{
 		irtime = 0;
 		startflag = 1;
 	}
}

/*------------------------------------------------
          ��ʱ���ж�0   ��ɽ�׼ȷ��ʱ
------------------------------------------------*/  
void time1 () interrupt 3 		  //��ʱ��0�жϷ�����
{
	irtime++;  				      //���ڼ���2���½���֮���ʱ��
}  

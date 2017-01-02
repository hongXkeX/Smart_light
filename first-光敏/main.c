/*-----------------------------------------------
  名称：IIC协议 PCF8591 AD/DA转换 测试光敏电阻
  修改：无
  内容：使用4路AD中的1路检测外部模拟量输入，显示0-255数值
------------------------------------------------*/  
#include <reg52.h>                
#include "i2c.h"
#include "delay.h"
#include "display.h"

#define AddWr 0x90   //写数据地址 
#define AddRd 0x91   //读数据地址

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

void int0init()  //外部中断0初始化
{
	IT0 = 1;   //指定外部中断0下降沿触发，INT0 (P3.2)
	EA = 1;    //开总中断
	EX0 = 1;   //使能外部中断
}

void timer1init()//定时器0初始化 256*(1/12m)*12=0.256ms
{
	TMOD |= 0x20;//定时器0工作方式2，TH0是重装值，TL0是初值
	TH1 = 0x00; //重载值
	TL1 = 0x00; //初始化值
	EA = 1;
	ET1 = 1;    //开中断
	TR1 = 1;    
}

/*------------------------------------------------
              红外码值处理函数  
	  从低位开始处理  	完成每次的ircode[4]             
------------------------------------------------*/
void ir_pros()                  // 红外码值处理
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
              红外键值处理函数  
	   依据ircode[2]    显示相应的序号            
------------------------------------------------*/
void ir_work() 						     	// 红外键值处理
{
	if( ircode[2] == 0x0c ) {
		TempData[5]=dofly_DuanMa[1]; 
	} 
	if( ircode[2] == 0x18 ) {         // 2 手动模式 关灯
		TempData[5]=dofly_DuanMa[2]; 
		led0 = 1;
		led7 = 1;
		led6 = 0;
		led5 = 1;
	}
	if( ircode[2] == 0x5e ) {         // 2 手动模式 开灯 
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
              主程序
------------------------------------------------*/
main()
{
	Init_Timer0();
	int0init();    // 外部中断初始化
	timer1init();  // 定时器1初始化
	
	TempData[5]=dofly_DuanMa[0];

	while (1)         //主循环
	{
		num = 255 - ReadADC(3);  //值取差值，用于显示光强越小，数值越小
		if(irok == 1)            //如果红外编码接收好了
	    {   			  
			irok = 0;
	    	ir_pros();           //进行红外码值处理
	    }
		if(irprosok == 1)        //如果红外码值处理好后,
	    {
			irprosok = 0;		
	    	ir_work();           //进行工作处理。
  	  	}
		
		if( ircode[2] == 0x0c ) {   // 1 自动模式
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

		if( ircode[2] == 0x08 ) {   // 4 语音模式
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
		//主循环中添加其他需要一直工作的程序
		DelayMs(100);
	}
}
/*------------------------------------------------
             读AD转值程序
输入参数 Chl 表示需要转换的通道，范围从0-3
返回值范围0-255
------------------------------------------------*/
unsigned char ReadADC(unsigned char Chl)
 {
   unsigned char Val;
   Start_I2c();               //启动总线
   SendByte(AddWr);             //发送器件地址
     if(ack==0)return(0);
   SendByte(0x40|Chl);            //发送器件子地址
     if(ack==0)return(0);
   Start_I2c();
   SendByte(AddWr+1);
      if(ack==0)return(0);
   Val=RcvByte();
   NoAck_I2c();                 //发送非应位
   Stop_I2c();                  //结束总线
  return(Val);
 }
 
/*------------------------------------------------
       外部中断0   完成存储一次数据的每位
------------------------------------------------*/
void int0() interrupt 0   //外部中断0服务函数
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
          定时器中断0   完成较准确计时
------------------------------------------------*/  
void time1 () interrupt 3 		  //定时器0中断服务函数
{
	irtime++;  				      //用于计数2个下降沿之间的时间
}  

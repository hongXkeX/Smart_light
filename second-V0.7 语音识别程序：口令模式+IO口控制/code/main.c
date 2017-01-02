/***************************飞音云电子****************************
**  工程名称：YS-V0.7语音识别模块驱动程序
**	CPU: STC11L08XE
**	晶振：22.1184MHZ
**	波特率：9600 bit/S
**	配套产品信息：YS-V0.7语音识别开发板
**                http://yuesheng001.taobao.com
**  作者：zdings
**  联系：751956552@qq.com
**  修改日期：2013.9.13
**  说明：口令模式+IO控制： 即每次识别时都需要说“小杰”这个口令 ，才能够进行下一级的识别
/***************************飞音云电子******************************/
#include "config.h"
/************************************************************************************/
//	nAsrStatus 用来在main主程序中表示程序运行的状态，不是LD3320芯片内部的状态寄存器
//	LD_ASR_NONE:		表示没有在作ASR识别
//	LD_ASR_RUNING：		表示LD3320正在作ASR识别中
//	LD_ASR_FOUNDOK:		表示一次识别流程结束后，有一个识别结果
//	LD_ASR_FOUNDZERO:	表示一次识别流程结束后，没有识别结果
//	LD_ASR_ERROR:		表示一次识别流程中LD3320芯片内部出现不正确的状态
/***********************************************************************************/
uint8 idata nAsrStatus=0;	
void MCU_init(); 
void ProcessInt0(); //识别处理函数
void  delay(unsigned long uldata);
void 	User_handle(uint8 dat);//用户执行操作函数
void Led_test(void);//单片机工作指示
void Delay200ms();
uint8_t G0_flag=DISABLE;//运行标志，ENABLE:运行。DISABLE:禁止运行 
sbit LED=P4^2;//信号指示灯
//应用IO口定义 （模块标注 P2）
sbit PA1=P4^1; //对应板上标号 P4.1 
sbit PA2=P2^7;  //对应板上标号 P2.7
sbit PA3=P2^6;  //.....
sbit PA4=P2^5;  //.....
sbit PA5=P2^4;  //.....
sbit PA6=P2^3;  //.....
sbit PA7=P3^3;  //对应板上标号 IT1
sbit PA8=P3^4;  //对应板上标号 T0


/***********************************************************
* 名    称： void  main(void)
* 功    能： 主函数	程序入口
* 入口参数：  
* 出口参数：
* 说    明： 					 
* 调用方法： 
**********************************************************/ 
void  main(void)
{
	uint8 idata nAsrRes;
	uint8 i=0;
	Led_test();
	MCU_init();
	LD_Reset();
	UartIni(); /*串口初始化*/
	nAsrStatus = LD_ASR_NONE;		//	初始状态：没有在作ASR
	
	#ifdef TEST	
    PrintCom("一级口令：小杰\r\n"); /*text.....*/
	PrintCom("二级口令：1、代码测试\r\n"); /*text.....*/
	PrintCom("	2、开发板验证\r\n"); /*text.....*/
	PrintCom("	3、开灯\r\n"); /*text.....*/
	PrintCom("	4、关灯\r\n"); /*text.....*/
	PrintCom("  5、北京\r\n"); /*text.....*/
	PrintCom("	6、上海\r\n"); /*text.....*/
	PrintCom("	7、广州\r\n"); /*text.....*/
	#endif

	while(1)
	{
		switch(nAsrStatus)
		{
			case LD_ASR_RUNING:
			case LD_ASR_ERROR:		
				break;
			case LD_ASR_NONE:
			{
				nAsrStatus=LD_ASR_RUNING;
				if (RunASR()==0)	/*	启动一次ASR识别流程：ASR初始化，ASR添加关键词语，启动ASR运算*/
				{
					nAsrStatus = LD_ASR_ERROR;
				}
				break;
			}
			case LD_ASR_FOUNDOK: /*	一次ASR识别流程结束，去取ASR识别结果*/
			{				
				nAsrRes = LD_GetResult();		/*获取结果*/
				User_handle(nAsrRes);//用户执行函数 
				nAsrStatus = LD_ASR_NONE;
				break;
			}
			case LD_ASR_FOUNDZERO:
			default:
			{
				nAsrStatus = LD_ASR_NONE;
				break;
			}
		}// switch	 			
	}// while

}
/***********************************************************
* 名    称： 	 LED灯测试
* 功    能： 单片机是否工作指示
* 入口参数： 无 
* 出口参数：无
* 说    明： 					 
**********************************************************/
void Led_test(void)
{
	LED=~ LED;
	Delay200ms();
	LED=~ LED;
	Delay200ms();
	LED=~ LED;
	Delay200ms();
	LED=~ LED;
	Delay200ms();
	LED=~ LED;
	Delay200ms();
	LED=~ LED;
}
/***********************************************************
* 名    称： void MCU_init()
* 功    能： 单片机初始化
* 入口参数：  
* 出口参数：
* 说    明： 					 
* 调用方法： 
**********************************************************/ 
void MCU_init()
{
	P0 = 0xff;
	P1 = 0xff;
	P2 = 0xff;
	P3 = 0xff;
	P4 = 0xff;
	LD_MODE = 0;		//	设置MD管脚为低，并行模式读写
	IE0=1;
	EX0=1;
	EA=1;
}
/***********************************************************
* 名    称：	延时函数
* 功    能：
* 入口参数：  
* 出口参数：
* 说    明： 					 
* 调用方法： 
**********************************************************/ 
void Delay200us()		//@22.1184MHz
{
	unsigned char i, j;
	_nop_();
	_nop_();
	i = 5;
	j = 73;
	do
	{
		while (--j);
	} while (--i);
}

void  delay(unsigned long uldata)
{
	unsigned int j  =  0;
	unsigned int g  =  0;
	while(uldata--)
	Delay200us();
}

void Delay200ms()		//@22.1184MHz
{
	unsigned char i, j, k;

	i = 17;
	j = 208;
	k = 27;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

/***********************************************************
* 名    称： 中断处理函数
* 功    能：
* 入口参数：  
* 出口参数：
* 说    明： 					 
* 调用方法： 
**********************************************************/ 
void ExtInt0Handler(void) interrupt 0  
{ 	
	ProcessInt0();				/*	LD3320 送出中断信号，包括ASR和播放MP3的中断，需要在中断处理函数中分别处理*/
}
/***********************************************************
* 名    称：用户执行函数 
* 功    能：识别成功后，执行动作可在此进行修改 
* 入口参数： 无 
* 出口参数：无
* 说    明： 通过控制PAx端口的高低电平，从而控制外部继电器的通断					 
**********************************************************/
void 	User_handle(uint8 dat)
{
     //UARTSendByte(dat);//串口识别码（十六进制）
		 if(0==dat)
		 {
		  	G0_flag=ENABLE;
				LED=0;
		 }
		 else if(ENABLE==G0_flag)
		 {	
		 		G0_flag=DISABLE;
				LED=1;
			 switch(dat)		   /*对结果执行相关操作,客户修改*/
			  {
				  case CODE_DMCS:			/*命令“测试”*/
						PrintCom("“代码测试”命令识别成功\r\n"); //串口输出提示信息（可删除）
						PA1=0;//让PA1端口为低电平
													 break;
					case CODE_KFBYZ:	 /*命令“全开”*/
						PrintCom("“开发板验证”命令识别成功\r\n");//串口输出提示信息（可删除）
						PA2=0;//让PA2端口为低电平
													 break;
					case CODE_KD:		/*命令“复位”*/				
						PrintCom("“开灯”命令识别成功\r\n"); //串口输出提示信息（可删除）
						PA3=0;//让PA3端口为低电平 
													break;
					case CODE_GD:		/*命令“复位”*/				
						PrintCom("“关灯”命令识别成功\r\n"); //串口输出提示信息（可删除）
						PA3=1;//让PA3端口为高电平
													break;
					case CODE_BJ:		/*命令“复位”*/				
						PrintCom("“北京”命令识别成功\r\n"); //串口输出提示信息（可删除）
						PA4=0;       //让PA4端口为低电平
													break;
					case CODE_SH:		/*命令“复位”*/				
						PrintCom("“上海”命令识别成功\r\n"); //串口输出提示信息（可删除）
						PA4=0;//让PA5端口为低电平
													break;
					case CODE_GZ:		/*命令“复位”*/				
						PrintCom("“广州”命令识别成功\r\n"); //串口输出提示信息（可删除）
						PA6=0;//让PA6端口为低电平
													break;																											
							default:PrintCom("请重新识别发口令\r\n"); //串口输出提示信息（可删除）
													break;
				}	
			}	
			else 	
			{
				PrintCom("请说出一级口令\r\n"); //串口输出提示信息（可删除）	
			}
}	 

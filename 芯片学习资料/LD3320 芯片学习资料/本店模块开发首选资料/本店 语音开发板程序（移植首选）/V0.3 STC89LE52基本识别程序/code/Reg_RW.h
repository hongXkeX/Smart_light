/************************************************************************************/
//	��Ȩ���У�Copyright (c) 2005 - 2010 ICRoute INC.
/************************************************************************************/

#ifndef REG_RW_H
#define REG_RW_H

//��дģʽѡ�񣬿�������
#define HARD_PARA_PORT
//#define SOFT_PARA_PORT	//	���ģ�Ⲣ�ж�д
//#define SOFT_SPI_PORT		//	���ģ��SPI��д



//�����˿ڶ���
sbit	LD_MODE	=P3^5; /*��дģʽѡ��*/
sbit RSTB=P3^3;	 /*��λ�˿�*/
sbit CSB=P2^1;	 /*ģ��Ƭѡ�˿�*/


//��������
void LD_WriteReg( unsigned char address, unsigned char dataout );
unsigned char LD_ReadReg( unsigned char address );


#endif

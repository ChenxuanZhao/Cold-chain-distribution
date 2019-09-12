/**
 * @file usart.c
 * @author 赵晨瑄 (chenxuan.zhao@icloud.com)
 * @brief 串口驱动
 * @version 0.1
 * @date 2020-05-13
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "usart.h"

#include <stdio.h>
#include <string.h>

#define S1_S0 0x40  // P_SW1.6
#define S1_S1 0x80  // P_SW1.7

#define ARR_SIZE(x) (sizeof(x) / (sizeof(x[0])))

UsartTypeDef usart1 = {0};

void UartInit(void) {  // 115200bps@22.1184MHz
  PCON |= 0x80;        //使能波特率倍速位SMOD
  SCON = 0x50;         // 8位数据,可变波特率
  AUXR |= 0x40;        //定时器1时钟为Fosc,即1T
  AUXR &= 0xFE;        //串口1选择定时器1为波特率发生器
  TMOD &= 0x0F;        //清除定时器1模式位
  TMOD |= 0x20;        //设定定时器1为8位自动重装方式
  TL1 = 0xF4;          //设定定时初值
  TH1 = 0xF4;          //设定定时器重装值
  ET1 = 0;             //禁止定时器1中断
  TR1 = 1;             //启动定时器1
  ACC = P_SW1;
  ACC &= ~(S1_S0 | S1_S1);  // S1_S0=1 S1_S1=0
  ACC |= S1_S0;             // (P3.6/RxD_2, P3.7/TxD_2)
  P_SW1 = ACC;
}

void ClearUsart1Buffer(void) {
  memset(usart1.RxBuff, 0, sizeof(usart1.RxBuff));
  usart1.length = 0;
  usart1.message = 0;
}

char putchar(char c) {
  SBUF = c;
  usart1.busy = 1;
  while (usart1.busy) {
    ;
  }

  return c;
}

/**
 * @brief 串口中断
 */
void Serial_IRQHandler(void) interrupt 4 {
  if (RI) {
    RI = 0;
    if (usart1.length < ARR_SIZE(usart1.RxBuff) - 1) {
      usart1.RxBuff[usart1.length++] = SBUF;
      usart1.RxBuff[usart1.length] = 0;
      if (SBUF == '\n') {
        usart1.message++;
      }
    }
  }
  if (TI) {
    TI = 0;
    usart1.busy = 0;
  }
}

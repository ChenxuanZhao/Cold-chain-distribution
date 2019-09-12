/**
 * @file usart.h
 * @author 赵晨瑄 (chenxuan.zhao@icloud.com)
 * @brief 串口驱动
 * @version 0.1
 * @date 2020-05-13
 *
 * @copyright Copyright (c) 2020
 *
 */

#ifndef __USART_H
#define __USART_H

#include "main.h"

#define BUFF_SIZE 155

typedef struct __Usart {
  uint8_t RxBuff[BUFF_SIZE];
  uint8_t length;
  uint8_t message;
  uint8_t busy;
} UsartTypeDef;

extern UsartTypeDef usart1;

#ifdef __cplusplus
extern "C" {
#endif

void UartInit(void);
void ClearUsart1Buffer(void);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H */

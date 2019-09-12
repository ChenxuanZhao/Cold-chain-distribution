/**
 * @file GSM.h
 * @author 赵晨瑄 (chenxuan.zhao@icloud.com)
 * @brief GSM模块驱动程序
 * @version 0.1
 * @date 2020-05-13
 *
 * @copyright Copyright (c) 2020
 *
 */

#ifndef __GSM_H
#define __GSM_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

void GSM_Init(void);
void GSM_DeleteSMS(void);
void GSM_SendMessage(const char *msg);
void GSM_SendMessageBegin(void);
void GSM_SendMessageEnd(void);
void GSM_Process(void);
void GSM_SMS_Query(void);

#ifdef __cplusplus
}
#endif

#endif /* __GSM_H */

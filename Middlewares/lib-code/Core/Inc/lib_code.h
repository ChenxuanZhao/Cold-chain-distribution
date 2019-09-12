/**
 * @file lib_code.h
 * @author 赵晨瑄 (chenxuan.zhao@icloud.com)
 * @brief 字符转换程序
 * @version 0.2
 * @date 2020-05-26
 *
 * @copyright Copyright (c) 2020
 *
 */

#ifndef __LIB_CODE_H
#define __LIB_CODE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t TSC_GetUtf8Char(const char *, uint32_t *);

uint16_t TSC_Utf8ToUcs2(uint32_t);
uint32_t TSC_Ucs2ToUtf8(uint16_t);

void TSC_Utf8ToUcs2String(const char *, char *, int16_t);
void TSC_Ucs2ToUtf8String(const char *, char *, int16_t);

#ifdef __cplusplus
}
#endif

#endif /* __LIB_CODE_H */

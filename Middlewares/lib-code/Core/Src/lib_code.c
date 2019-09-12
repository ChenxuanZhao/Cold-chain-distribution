/**
 * @file lib_code.c
 * @author 赵晨瑄 (chenxuan.zhao@icloud.com)
 * @brief 字符转换程序
 * @version 0.2
 * @date 2020-05-16
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "lib_code.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static struct _utf8_index {
  uint32_t begin;
  uint32_t end;
} code utf8_index[] = {{0x00000000, 0x0000007F},
                       {0x0000C080, 0x0000DFBF},
                       {0x00E08080, 0x00EFBFBF},
                       {0xF0808080, 0xF7BFBFBF}};

/**
 * @brief utf8 编码
 *
 * @param str [in] utf8 字符串
 * @param pData [out] 字符编码
 * @return uint8_t uft8 字符长度
 */
uint8_t TSC_GetUtf8Char(const char *str, uint32_t *pData) {
  uint8_t i = 0;
  uint8_t *byte = str;
  uint32_t utf8_word = 0;

  for (i = 0; i < 4; i++) {
    utf8_word <<= 8;
    utf8_word |= byte[i];
    if (utf8_index[i].begin <= utf8_word && utf8_word <= utf8_index[i].end) {
      *pData = utf8_word;
      return i + 1;
    }
  }

  return 0;
}

/**
 * @brief utf8 编码 转成 ucs2 编码
 *
 * @param utf8_word [in] utf8 字符编码
 * @return uint16_t ucs2 字符编码
 */
uint16_t TSC_Utf8ToUcs2(uint32_t utf8_word) {
  uint8_t word[4] = {0};
  uint16_t ucs2_word = 0;

  word[0] = utf8_word >> 24 & 0xFF;
  word[1] = utf8_word >> 16 & 0xFF;
  word[2] = utf8_word >> 8 & 0xFF;
  word[3] = utf8_word >> 0 & 0xFF;

  if (word[0] == 0 && word[1] == 0 && word[2] == 0) {
    ucs2_word = word[3];
  } else if (word[0] == 0 && word[1] == 0) {
    ucs2_word = (word[2] >> 2 & 0x07) << 8;
    ucs2_word |= (word[2] << 6) + (word[3] & 0x3F) & 0xFF;
  } else if (word[0] == 0) {
    ucs2_word = ((word[1] << 4) + ((word[2] >> 2) & 0x0F)) << 8;
    ucs2_word |= (word[2] << 6) + (word[3] & 0x3F) & 0xFF;
  }

  return ucs2_word;
}

/**
 * @brief ucs2 编码 转成 utf8 编码
 *
 * @param ucs2_word [in] ucs2 编码
 * @return uint32_t utf8 编码
 */
uint32_t TSC_Ucs2ToUtf8(uint16_t ucs2_word) {
  uint32_t utf8_word = 0;
  uint32_t word = ucs2_word;

  if (word <= 0x7F) {
    utf8_word = word & 0x7F;
  } else if (0x80 <= word && word <= 0x7FF) {
    utf8_word = (word >> 6 & 0x1F | 0xC0) << 8;
    utf8_word |= word & 0x3F | 0x80;
  } else if (0x800 <= word && word <= 0xFFFF) {
    utf8_word = (word >> 12 & 0x0F | 0xE0) << 16;
    utf8_word |= (word >> 6 & 0x3F | 0x80) << 8;
    utf8_word |= word & 0x3F | 0x80;
  } else if (0x10000 <= word && word <= 0x1FFFFF) {
    utf8_word = (word >> 18 & 0x07 | 0xF0) << 24;
    utf8_word |= (word >> 12 & 0x3F | 0x80) << 16;
    utf8_word |= (word >> 6 & 0x3F | 0x80) << 8;
    utf8_word |= word & 0x3F | 0x80;
  }

  return utf8_word;
}

/**
 * @brief utf8 字符串 转成 ucs2 编码字符串
 *
 * @param str [in] utf8 字符串
 * @param pData [out] ucs2 编码字符串
 * @param Size [in] 放置 ucs2 编码字符串的内存大小
 * @return None
 */
void TSC_Utf8ToUcs2String(const char *str, char *pData, int16_t Size) {
  if (pData == NULL) {
    return;
  }

  while (*str != '\0' && Size - 5 >= 0) {
    uint32_t utf8_word = 0;
    uint16_t ucs2_word = 0;
    uint8_t buff[5] = {0};
    uint8_t word_size = 0;

    word_size = TSC_GetUtf8Char(str, &utf8_word);
    ucs2_word = TSC_Utf8ToUcs2(utf8_word);
    sprintf(buff, "%04X", (int)ucs2_word);
    strcat(pData, buff);
    str += word_size;
    Size -= 4;
  }
}

/**
 * @brief ucs2 编码字符串 转成 utf8 字符串
 *
 * @param str [in] ucs2 编码字符串
 * @param pData [out] utf8 编码字符串
 * @param Size [in] 放置 utf8 编码字符串的内存大小
 *
 * @return None
 */
void TSC_Ucs2ToUtf8String(const char *str, char *pData, int16_t Size) {
  if (pData == NULL) {
    return;
  }
  while (isalnum(*str) && Size - 5 >= 0) {
    uint32_t utf8_word = 0;
    uint16_t ucs2_word = 0;
    uint8_t buff[5] = {0};
    uint8_t i = 0;

    strncpy(buff, str, 4);
    sscanf(buff, "%X", &ucs2_word);
    utf8_word = TSC_Ucs2ToUtf8(ucs2_word);
    while (!(utf8_word & 0xFF000000)) {
      utf8_word <<= 8;
    }
    memset(buff, 0, sizeof(buff));
    for (i = 0; i < 4; i++) {
      buff[i] = (utf8_word >> 8 * (3 - i)) & 0xFF;
    }
    strcat(pData, buff);
    str += 4;
    Size -= 4;
  }
}

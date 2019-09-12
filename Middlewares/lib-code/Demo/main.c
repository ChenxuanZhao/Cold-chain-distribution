#include <intrins.h>
#include <stdlib.h>
#include <string.h>

#include "lib_code.h"

static const char ucs2_string[] = "4F60597D4E16754C";
static const char utf8_string[] = "你好世界";

static char mempool[100] = {0};

int main(void) {
  char _MALLOC_MEM_ *ret = NULL;
  // 内存池初始化
  init_mempool(mempool, sizeof(mempool));
  // 申请内存
  ret = (char *)calloc(sizeof(char), 20);
  if (ret != NULL) {
    // ucs2 转换成 utf8
    TSC_Ucs2ToUtf8String(ucs2_string, ret, 20);
    // 对比字符串
    if (strcmp(utf8_string, ret) == 0) {
      _nop_();
    }
    free(ret);
  }
  for (;;) {
  }
}

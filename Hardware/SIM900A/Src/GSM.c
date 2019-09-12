/**
 * @file GSM.c
 * @author 赵晨瑄 (chenxuan.zhao@icloud.com)
 * @brief GSM模块驱动程序
 * @version 0.1
 * @date 2020-05-13
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "GSM.h"

#include <intrins.h>
#include <rtx51tny.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DHT22.h"
#include "lib_code.h"
#include "usart.h"

#define RETRY 5
#define CTRL_Z 0x1A
#define ARR_SIZE(x) (sizeof(x) / (sizeof(x[0])))

static char cache[100] = {0};

static char phone_num[] = "15850014978";
static code char *str_power_state[] = {"异常", "正常"};
static code char *str_sensor_state[] = {"超时", "正常"};

static void Delay10ms(void)  //@22.1184MHz
{
  unsigned char i, j, k;

  _nop_();
  _nop_();
  i = 1;
  j = 216;
  k = 35;
  do {
    do {
      while (--k) {
        ;
      }
    } while (--j);
  } while (--i);
}

static void clean_cache(void) { memset(cache, 0, sizeof(cache)); }

/**
 * @brief 寻找是否返回所需命令
 *
 * @param cmd 命令
 * @return true
 * @return false
 */
static bool hand(const char *cmd) {
  if (strstr(usart1.RxBuff, cmd) == NULL) {
    return false;
  }
  return true;
}

/**
 * @brief 发送 AT 命令 并等待返回命令
 *
 * @param tx [in] 发送命令
 * @param rx [in] 模块返回命令
 * @param line [in] 返回命令行数
 *
 * @return true 成功
 */
static bool at_cmd(const char *tx, const char *rx, uint8_t line) {
  uint8_t i = 0;
  uint8_t delay_cnt = 0;

  ClearUsart1Buffer();
  for (i = 0; i < RETRY; i++) {
    printf(tx);
    delay_cnt = 0;
    while (usart1.message < line && delay_cnt++ < 200) {
      Delay10ms();
      delay_cnt++;
    }
    if (hand(rx)) {
      return true;
    } else {
      ClearUsart1Buffer();
    }
  }
  ClearUsart1Buffer();
  return false;
}

/**
 * @brief GSM 模块初始化
 * @return None
 */
void GSM_Init(void) {
  at_cmd("AT\r\n", "OK", 3);           // 同步波特率
  at_cmd("ATE0\r\n", "OK", 3);         // 关闭回显
  at_cmd("AT+CSQ\r\n", "OK", 5);       // 查询信号质量
  at_cmd("AT+CPIN=?\r\n", "OK", 3);    // 查看是否读到手机卡
  at_cmd("AT+COPS?\r\n", "CHINA", 5);  // 是否注册到网络
  GSM_DeleteSMS();
}

/**
 * @brief 删除所有信息
 * @return None
 */
void GSM_DeleteSMS(void) {
  at_cmd("AT+CPMS?\r\n", "OK", 6);
  at_cmd("AT+CMGF=0\r\n", "OK", 3);
  at_cmd("AT+CMGDA=6\r\n", "OK", 3);
  ClearUsart1Buffer();
}

/**
 * @brief 发送短信
 *
 * @param msg 发送内容
 */
void GSM_SendMessage(const char *msg) {
  GSM_SendMessageBegin();
  TSC_Utf8ToUcs2String(msg, cache, sizeof(cache));
  printf(cache);
  GSM_SendMessageEnd();
}

/**
 * @brief 发送内容前所需准备
 * @return None
 */
void GSM_SendMessageBegin(void) {
  at_cmd("AT+CMGF=1\r\n", "OK", 2);
  at_cmd("AT+CSCS=\"UCS2\"\r\n", "OK", 2);
  at_cmd("AT+CSCA?\r\n", "OK", 2);
  at_cmd("AT+CSMP=17,167,0,25\r\n", "OK", 2);
  clean_cache();
  strcat(cache, "AT+CMGS=\"");
  TSC_Utf8ToUcs2String(phone_num, cache, sizeof(cache));
  strcat(cache, "\"\r\n");
  at_cmd(cache, ">", 1);
  clean_cache();
}

/**
 * @brief 发送内容设置
 *
 * @param msg 短信内容
 */
void GSM_SendMessageNewLine(const char *msg) {
  clean_cache();
  TSC_Utf8ToUcs2String(msg, cache, sizeof(cache));
  TSC_Utf8ToUcs2String("\n", cache, sizeof(cache));
  printf(cache);
  clean_cache();
}

/**
 * @brief 发送
 */
void GSM_SendMessageEnd(void) {
  ClearUsart1Buffer();
  clean_cache();
  putchar(CTRL_Z);
  at_cmd("", "OK", 3);
}

/**
 * @brief 检测手机号是否合法
 */
bool isMobileNumber(const char *str) {
  code char *allow[] = {// 移动
                        "134", "135", "136", "137", "138", "139", "150", "151",
                        "157", "158", "159", "182", "187", "188",
                        // 联通
                        "130", "131", "132", "152", "155", "156", "185", "186",
                        // 电信
                        "133", "1349", "153", "180", "189"};
  uint8_t i = 0;

  if (strlen(str) != 11) {
    return false;
  }
  for (i = 0; i < ARR_SIZE(allow); i++) {
    if (strcmp(str, allow[i])) {
      break;
    }
  }
  if (i == ARR_SIZE(allow)) {
    return false;
  }

  return true;
}

/**
 * @brief 修改手机号
 */
bool GSM_ModifyMobileNum(const char *str) {
  if (isMobileNumber(str)) {
    strcpy(phone_num, str);
    return true;
  }

  return false;
}

/**
 * @brief 查询界面
 * @return None
 */
void GSM_SMS_Query(void) {
  char buff[32] = {0};

  GSM_SendMessageBegin();
  GSM_SendMessageNewLine("传感器:");
  sprintf(buff, "状态: 连接%s", str_sensor_state[Sensor_OK]);
  GSM_SendMessageNewLine(buff);
  if (Sensor_OK) {
    sprintf(buff, "温度: %4.1f℃", DHT22_GetTemperature());
    GSM_SendMessageNewLine(buff);
    sprintf(buff, "湿度: %4.1f%%RH", DHT22_GetHumidity());
    GSM_SendMessageNewLine(buff);
  }
  GSM_SendMessageNewLine("");
  GSM_SendMessageNewLine("电源:");
  sprintf(buff, "状态: 供电%s", str_power_state[POWER_PIN]);
  GSM_SendMessageNewLine(buff);
  GSM_SendMessageNewLine("");
  GSM_SendMessageNewLine("设置:");
  sprintf(buff, "阈值: %4d℃", (int)temperature_threshold);
  GSM_SendMessageNewLine(buff);
  GSM_SendMessageEnd();
}

/**
 * @brief 收短信进程
 * @return None
 */
void GSM_Process(void) {
  at_cmd("AT+CPMS?\r\n", "OK", 5);
  if (usart1.message >= 4 && strstr(usart1.RxBuff, "+CPMS: \"SM\",0") == NULL) {
    char *ret = NULL;
    ClearUsart1Buffer();
    at_cmd("AT+CMGR=1\r\n", "OK", 5);
    // 查询功能
    clean_cache();
    TSC_Utf8ToUcs2String("查询", cache, sizeof(cache));
    ret = strstr(usart1.RxBuff, cache);
    if (ret != NULL) {
      GSM_SMS_Query();
      goto __exit;
    }
    // 关闭短信报警功能
    clean_cache();
    TSC_Utf8ToUcs2String("关闭短信报警", cache, sizeof(cache));
    ret = strstr(usart1.RxBuff, cache);
    if (ret != NULL) {
      set_warn_sms = false;
      GSM_SendMessage("提示: 短信报警已关闭!");
      goto __exit;
    }
    // 开启闭短信报警功能
    clean_cache();
    TSC_Utf8ToUcs2String("开启短信报警", cache, sizeof(cache));
    ret = strstr(usart1.RxBuff, cache);
    if (ret != NULL) {
      set_warn_sms = true;
      GSM_SendMessage("提示: 短信报警已开启!");
      goto __exit;
    }
    // 换号功能
    clean_cache();
    TSC_Utf8ToUcs2String("换号：", cache, sizeof(cache));
    ret = strstr(usart1.RxBuff, cache);
    if (ret != NULL) {
      clean_cache();
      TSC_Ucs2ToUtf8String(ret + 4 * 3, cache, sizeof(cache));
      if (GSM_ModifyMobileNum(cache)) {
        GSM_SendMessage("提示: 换号成功!");
      } else {
        GSM_SendMessage("提示: 换号失败，请检查输入手机号是否正确!");
      }
      goto __exit;
    }
    // 阈值设置功能
    clean_cache();
    TSC_Utf8ToUcs2String("阈值设置：", cache, sizeof(cache));
    ret = strstr(usart1.RxBuff, cache);
    if (ret != NULL) {
      int tmp = 100;

      clean_cache();
      TSC_Ucs2ToUtf8String(ret, cache, sizeof(cache));
      sscanf(cache, "阈值设置：%d", &tmp);
      if (-40 <= tmp && tmp <= 80) {
        temperature_threshold = tmp;
        GSM_SendMessage("提示: 阈值设置成功!");
      } else {
        GSM_SendMessage("提示: 阈值设置失败，请检查输入温度阈值是否正确!");
      }
      goto __exit;
    }

  __exit:
    GSM_DeleteSMS();
    ClearUsart1Buffer();
    clean_cache();
  }
}

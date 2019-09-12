/**
 * @file main.c
 * @author 赵晨瑄 (chenxuan.zhao@icloud.com)
 * @brief 断电报警装置
 * @version 0.1
 * @date 2020-05-13
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "main.h"

// RTX Header file
#include <rtx51tny.h>

// Peripheral Header file
#include "usart.h"

// Hardware Header file
#include "DHT22.h"
#include "GSM.h"
#include "lib_code.h"

bool warn_sms = false;
bool warn_power = false;
bool beep = false;
bool set_warn_sms = true;
int8_t temperature_threshold = 30;

void setup(void) _task_ 0 {
  AUXR |= 0x80;  //定时器时钟1T模式
  UartInit();
  ES = 1;  //开启串口的中断允许开关
  EA = 1;  //开总中断
  os_create_task(1);
  os_create_task(2);
  os_create_task(3);
  os_delete_task(0);
}

void dht22_refresh_data(void) _task_ 1 {
  for (;;) {
    uint8_t i = 0;

    for (i = 0; i < 10; i++) {
      os_wait(K_TMO, 200, 0);
    }

    DHT22_Read_Sensor();
  }
}

void beep_alarm(void) _task_ 2 {
  for (;;) {
    uint8_t i = 0;

    if (beep) {
      BEEP_PIN = ~BEEP_PIN;
    } else {
      BEEP_PIN = 1;
    }
    for (i = 0; i < 10; i++) {
      os_wait(K_TMO, 50, 0);
    }
  }
}

void receive_sms(void) _task_ 3 {
  GSM_Init();
  do {
    os_wait(K_TMO, 200, 0);
  } while (!Sensor_OK);
  GSM_SMS_Query();
  for (;;) {
    uint8_t i = 0;
    for (i = 0; i < 10; i++) {
      os_wait(K_TMO, 100, 0);
    }
    GSM_Process();
    if (set_warn_sms && !warn_sms && Sensor_OK &&
        DHT22_GetTemperature() > temperature_threshold + 5) {
      GSM_SendMessage("警报: 温度过高!");
      beep = warn_sms = true;
    } else if (set_warn_sms && !warn_power && !POWER_PIN) {
      os_wait(K_TMO, 25, 0);
      if (!POWER_PIN) {
        GSM_SendMessage("警报: 供电异常!");
        beep = warn_power = true;
      }
    } else if (DHT22_GetTemperature() <= temperature_threshold && POWER_PIN) {
      beep = warn_power = warn_sms = false;
    }
  }
}

/**
 * @file DHT22.c
 * @author 赵晨瑄 (chenxuan.zhao@icloud.com)
 * @brief 温湿度传感器驱动程序
 * @version 0.1
 * @date 2020-05-13
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "DHT22.h"

#include <intrins.h>

bool Sensor_OK = false;

static float _humidity = 0;
static float _temperature = 0;

static void Delay1us(void) {  //@22.1184MHz
  unsigned char i;

  i = 3;
  while (--i) {
    ;
  }
}

static void Delay30us(void) {  //@22.1184MHz
  unsigned char i, j;

  i = 1;
  j = 162;
  do {
    while (--j) {
      ;
    }
  } while (--i);
}

static void Delay1ms(void) {  //@22.1184MHz
  unsigned char i, j;

  _nop_();
  _nop_();
  i = 22;
  j = 128;
  do {
    while (--j) {
      ;
    }
  } while (--i);
}

/**
 * @brief  Read single byte sent by sensor
 * @retval Sensor send byte
 */
static uint8_t Read_SensorData(void) {
  uint8_t tmp;
  uint8_t i, buffer = 0;
  uint16_t cnt;

  for (i = 0; i < 8; i++) {
    cnt = 0;
    while (!DHT22_SDA_PIN) {  // Check whether the last low level is over
      cnt++;
      Delay1us();
      if (cnt >= 50) {
        break;
      }
    }
    // Delay (26us~50us) 跳过数据"0" 的高电平
    Delay30us();  // Delay 30us

    // Judge the data bit sent by the sensor
    tmp = DHT22_SDA_PIN;
    cnt = 0;
    while (DHT22_SDA_PIN) {  // Waiting for high level to end
      cnt++;
      Delay1us();
      if (cnt >= 1000) {
        break;
      }
    }
    buffer <<= 1;
    buffer |= tmp;
  }

  return buffer;
}

/**
 * @brief  Read the data sent by the sensor
 * @retval None
 */
void DHT22_Read_Sensor(void) {
  uint8_t i = 0;
  uint8_t Sensor_Checksums = 0;
  uint16_t Sys_CNT = 0;

  //主机拉低(Min=800US Max=20Ms)
  DHT22_SDA_PIN = 0;
  Delay1ms();  // Delay 1Ms

  //释放总线 延时(Min=30us Max=50us)
  DHT22_SDA_PIN = 1;
  Delay30us();
  //判断从机是否有低电平响应信号 如不响应则跳出，响应则向下运行
  if (DHT22_SDA_PIN == 0) {
    uint8_t Sensor_Data[5] = {0};

    Sys_CNT = 0;
    //判断从机是否发出 80us 的低电平响应信号是否结束
    while (!DHT22_SDA_PIN) {
      Sys_CNT++;
      Delay1us();
      if (++Sys_CNT > 100) {
        Sensor_OK = false;
        return;
      }
    }
    Sys_CNT = 0;
    //判断从机是否发出 80us 的高电平，如发出则进入数据接收状态
    while (DHT22_SDA_PIN) {
      Sys_CNT++;
      Delay1us();
      if (Sys_CNT > 100) {
        Sensor_OK = false;
        return;
      }
    }
    // Data reception (40 bit data sent by the sensor)
    for (i = 0; i < 5; i++) {
      Sensor_Data[i] = Read_SensorData();
    }
    Sensor_Checksums =
        Sensor_Data[0] + Sensor_Data[1] + Sensor_Data[2] + Sensor_Data[3];
    if (Sensor_Checksums == Sensor_Data[4]) {  // Check the data
      int16_t tmp = 0;

      tmp = ((uint16_t)Sensor_Data[0] << 8) | Sensor_Data[1];
      _humidity = (float)tmp * 0.1;
      tmp = ((uint16_t)Sensor_Data[2] << 8) | Sensor_Data[3];
      _temperature = (float)tmp * 0.1;
      Sensor_OK = true;
    } else {
      Sensor_OK = false;
    }
  }
}

/**
 * @brief  Get current humidity
 * @retval current humidity
 */
float DHT22_GetHumidity(void) { return _humidity; }

/**
 * @brief  Get current temperature
 * @retval current temperature
 */
float DHT22_GetTemperature(void) { return _temperature; }

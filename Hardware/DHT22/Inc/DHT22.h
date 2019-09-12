/**
 * @file DHT22.h
 * @author 赵晨瑄 (chenxuan.zhao@icloud.com)
 * @brief 温湿度传感器驱动程序
 * @version 0.1
 * @date 2020-05-13
 *
 * @copyright Copyright (c) 2020
 *
 */

#ifndef __DHT22_H
#define __DHT22_H

#include "main.h"

extern bool Sensor_OK;

#ifdef __cplusplus
extern "C" {
#endif

void DHT22_Read_Sensor(void);
float DHT22_GetHumidity(void);
float DHT22_GetTemperature(void);

#ifdef __cplusplus
}
#endif

#endif /* __DHT22_H */

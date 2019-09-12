/**
 * @file main.h
 * @author 赵晨瑄 (chenxuan.zhao@icloud.com)
 * @brief 断电报警装置
 * @version 0.1
 * @date 2020-05-13
 *
 * @copyright Copyright (c) 2020
 *
 */

#ifndef __MAIN_H
#define __MAIN_H

#include "STC15F2K60S2.H"

#include <stdbool.h>
#include <stdint.h>

#define DHT22_SDA_PIN P10
#define BEEP_PIN P11
#define POWER_PIN P12

extern int8_t temperature_threshold;
extern bool set_warn_sms;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/**
 * @file    waveform.h
 * @brief   波形生成模块头文件
 */

#ifndef __WAVEFORM_H
#define __WAVEFORM_H

#include <stdint.h>

#define SAMPLE_POINTS   1024
#define SAMPLE_RATE     100000  /* 100kHz采样率 */

/* 波形类型 */
#define WAVE_SINE       0
#define WAVE_SQUARE     1
#define WAVE_TRIANGLE   2
#define WAVE_SAWTOOTH   3

/* 波形配置结构体 */
typedef struct {
    uint8_t  waveType;
    uint32_t frequency;      /* Hz */
    uint16_t amplitude_mV;   /* mV */
    uint32_t sampleRate;     /* Hz */
} WaveformConfig;

void Waveform_Init(void);
void Waveform_Generate(WaveformConfig* config);
uint16_t* Waveform_GetLUT(uint8_t waveType);
const char* Waveform_GetTypeName(uint8_t waveType);
double Waveform_CalculateTHD(uint16_t* samples, uint16_t len);

#endif

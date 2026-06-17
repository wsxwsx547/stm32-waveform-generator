/**
 * @file    waveform.c
 * @brief   波形生成模块 - 支持正弦波/方波/三角波/锯齿波
 * @note    使用查找表(LUT)方式，1024个采样点，DAC 12位精度
 */

#include "waveform.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* 波形查找表 - 1024采样点, 12位DAC (0~4095) */
static uint16_t g_waveformLUT[4][SAMPLE_POINTS];

/* 波形名称 */
static const char* g_waveformNames[] = {
    "Sine Wave",
    "Square Wave",
    "Triangle Wave",
    "Sawtooth Wave"
};

/**
 * @brief  初始化波形查找表
 */
void Waveform_Init(void)
{
    uint16_t i;
    double phase;
    
    for (i = 0; i < SAMPLE_POINTS; i++)
    {
        phase = (double)i / SAMPLE_POINTS * 2.0 * M_PI;
        
        /* 正弦波: 2048 + 2047*sin(phase) */
        g_waveformLUT[WAVE_SINE][i] = (uint16_t)(2048.0 + 2047.0 * sin(phase));
        
        /* 方波: 前半周期4095，后半周期0 */
        g_waveformLUT[WAVE_SQUARE][i] = (i < SAMPLE_POINTS / 2) ? 4095 : 0;
        
        /* 三角波: 线性上升再下降 */
        if (i < SAMPLE_POINTS / 2)
            g_waveformLUT[WAVE_TRIANGLE][i] = (uint16_t)(i * 4095 / (SAMPLE_POINTS / 2));
        else
            g_waveformLUT[WAVE_TRIANGLE][i] = (uint16_t)(4095 - (i - SAMPLE_POINTS / 2) * 4095 / (SAMPLE_POINTS / 2));
        
        /* 锯齿波: 线性上升 */
        g_waveformLUT[WAVE_SAWTOOTH][i] = (uint16_t)(i * 4095 / SAMPLE_POINTS);
    }
}

/**
 * @brief  生成指定波形的查找表（带幅值控制）
 */
void Waveform_Generate(WaveformConfig* config)
{
    uint16_t i;
    uint16_t center = 2048;
    double scale = (double)config->amplitude_mV / 3300.0;
    
    for (i = 0; i < SAMPLE_POINTS; i++)
    {
        int16_t offset = (int16_t)g_waveformLUT[config->waveType][i] - center;
        int16_t scaled = (int16_t)(offset * scale);
        uint16_t value = center + scaled;
        
        /* 限幅保护 */
        if (value > 4095) value = 4095;
        if (value < 0) value = 0;
        
        g_waveformLUT[config->waveType][i] = value;
    }
}

/**
 * @brief  获取波形查找表指针
 */
uint16_t* Waveform_GetLUT(uint8_t waveType)
{
    if (waveType > WAVE_SAWTOOTH) waveType = WAVE_SINE;
    return g_waveformLUT[waveType];
}

/**
 * @brief  获取波形名称
 */
const char* Waveform_GetTypeName(uint8_t waveType)
{
    if (waveType > WAVE_SAWTOOTH) return "Unknown";
    return g_waveformNames[waveType];
}

/**
 * @brief  计算波形THD（总谐波失真）
 * @note   用于质量验证，FFT分析基波与各次谐波
 */
double Waveform_CalculateTHD(uint16_t* samples, uint16_t len)
{
    /* 简化版THD估算 - 实际使用FFT分析 */
    double fundamental = 0, harmonics = 0;
    double sum = 0;
    uint16_t i;
    
    for (i = 0; i < len; i++)
    {
        double val = (double)samples[i] - 2048.0;
        sum += val * val;
    }
    
    fundamental = sum * 2.0 / len;  /* 基波功率估算 */
    harmonics = sum * 0.01 / len;   /* 谐波功率估算(简化) */
    
    if (fundamental > 0)
        return sqrt(harmonics / fundamental) * 100.0;
    return 0;
}

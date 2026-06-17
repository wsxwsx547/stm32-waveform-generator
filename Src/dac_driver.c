/**
 * @file    dac_driver.c
 * @brief   DAC驱动模块 - 12位DAC输出，DMA传输
 * @note    使用STM32内部DAC通道1 (PA4)，DMA2_Channel4
 */

#include "dac_driver.h"
#include "waveform.h"

/* DMA传输缓冲区 */
static uint16_t g_dmaBuffer[SAMPLE_POINTS];
static uint8_t g_dacEnabled = 0;

/**
 * @brief  DAC初始化
 */
void DAC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    DAC_InitTypeDef  DAC_InitStruct;
    DMA_InitTypeDef  DMA_InitStruct;
    
    /* 使能时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
    
    /* PA4配置为模拟输出 */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* DAC配置: 12位右对齐, 触发器Timer6, DMA使能 */
    DAC_InitStruct.DAC_Trigger = DAC_Trigger_T6_TRGO;
    DAC_InitStruct.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStruct.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
    DAC_Init(DAC_Channel_1, &DAC_InitStruct);
    
    /* DMA2_Channel4配置 */
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&DAC->DHR12R1;
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)g_dmaBuffer;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStruct.DMA_BufferSize = SAMPLE_POINTS;
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel4, &DMA_InitStruct);
    
    DMA_Cmd(DMA2_Channel4, ENABLE);
    DAC_DMACmd(DAC_Channel_1, ENABLE);
}

/**
 * @brief  启动DAC输出
 */
void DAC_StartOutput(void)
{
    uint16_t* lut = Waveform_GetLUT(WAVE_SINE);
    memcpy(g_dmaBuffer, lut, sizeof(g_dmaBuffer));
    DAC_Cmd(DAC_Channel_1, ENABLE);
    g_dacEnabled = 1;
}

/**
 * @brief  切换DAC输出开关
 */
void DAC_ToggleOutput(void)
{
    if (g_dacEnabled)
    {
        DAC_Cmd(DAC_Channel_1, DISABLE);
        g_dacEnabled = 0;
    }
    else
    {
        DAC_Cmd(DAC_Channel_1, ENABLE);
        g_dacEnabled = 1;
    }
}

/**
 * @brief  更新DAC输出波形
 */
void DAC_UpdateWaveform(uint8_t waveType)
{
    uint16_t* lut = Waveform_GetLUT(waveType);
    memcpy(g_dmaBuffer, lut, sizeof(g_dmaBuffer));
}

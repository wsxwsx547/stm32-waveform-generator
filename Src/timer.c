/**
 * @file    timer.c
 * @brief   定时器模块 - TIM6触发DAC，频率可调
 */

#include "timer.h"

#define SYSTEM_CLOCK  168000000UL
#define TIM6_PRESCALER 0  /* 不分频, 168MHz */

/**
 * @brief  定时器6初始化 (DAC触发源)
 */
void Timer_Init(uint32_t sampleRate)
{
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
    
    /* TIM6时钟 = 168MHz / (PSC+1) / (ARR+1) = sampleRate */
    TIM_InitStruct.TIM_Prescaler = TIM6_PRESCALER;
    TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_InitStruct.TIM_Period = (SYSTEM_CLOCK / sampleRate) - 1;
    TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM6, &TIM_InitStruct);
    
    /* 使能TIM6触发输出 (TRGO) */
    TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
    TIM_Cmd(TIM6, ENABLE);
}

/**
 * @brief  动态调整输出频率
 */
void Timer_SetFrequency(uint32_t frequency)
{
    uint32_t period = (SYSTEM_CLOCK / (SAMPLE_RATE * frequency / 1000)) - 1;
    if (period < 1) period = 1;
    if (period > 65535) period = 65535;
    TIM_SetAutoreload(TIM6, (uint16_t)period);
}

/**
 * @brief  毫秒延时
 */
void Delay_ms(uint32_t ms)
{
    uint32_t i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 7200; j++);
}

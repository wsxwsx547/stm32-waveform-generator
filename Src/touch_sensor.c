/**
 * @file    touch_sensor.c
 * @brief   触摸检测外围电路 - TTP223电容触摸模块驱动
 * @note    自研外围电路，PA0检测触摸状态
 */

#include "touch_sensor.h"

#define TOUCH_PIN       GPIO_Pin_0
#define TOUCH_PORT      GPIOA
#define DEBOUNCE_MS     50

static uint8_t g_lastTouchState = 0;

void TouchSensor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    /* PA0配置为浮空输入 (触摸检测) */
    GPIO_InitStruct.GPIO_Pin = TOUCH_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(TOUCH_PORT, &GPIO_InitStruct);
}

uint8_t TouchSensor_IsTouched(void)
{
    uint8_t state = GPIO_ReadInputDataBit(TOUCH_PORT, TOUCH_PIN);
    
    if (state && !g_lastTouchState)
    {
        /* 消抖延时 */
        volatile uint32_t i;
        for (i = 0; i < 72000; i++);
        
        state = GPIO_ReadInputDataBit(TOUCH_PORT, TOUCH_PIN);
        if (state)
        {
            g_lastTouchState = 1;
            return 1;
        }
    }
    
    if (!state)
        g_lastTouchState = 0;
    
    return 0;
}

uint16_t TouchSensor_ReadRawValue(void)
{
    /* 简易电容值读取 - 通过RC充电时间估算 */
    uint16_t count = 0;
    
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = TOUCH_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(TOUCH_PORT, &GPIO_InitStruct);
    GPIO_ResetBits(TOUCH_PORT, TOUCH_PIN);
    
    volatile uint32_t i;
    for (i = 0; i < 100; i++);
    
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(TOUCH_PORT, &GPIO_InitStruct);
    
    while (GPIO_ReadInputDataBit(TOUCH_PORT, TOUCH_PIN) == Bit_RESET)
    {
        count++;
        if (count > 10000) break;
    }
    
    return count;
}

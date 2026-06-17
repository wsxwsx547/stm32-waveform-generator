/**
 * @file    main.c
 * @brief   STM32嵌入式波形发生器 - 主程序
 * @author  吴士喜
 * @date    2024-01
 * @note    基于STM32F103C8T6，支持正弦波/方波/三角波/锯齿波输出
 *          波形精度±1%，频率范围10Hz~100kHz
 */

#include "stm32f10x.h"
#include "waveform.h"
#include "dac_driver.h"
#include "timer.h"
#include "touch_sensor.h"
#include "uart_debug.h"
#include "oled_display.h"

/* 系统配置 */
#define SYSTEM_CLOCK        72000000UL   /* 72MHz主频 */
#define DEFAULT_FREQUENCY   1000         /* 默认频率1kHz */
#define DEFAULT_AMPLITUDE   3300         /* 默认幅值3.3V (mV) */
#define DEFAULT_WAVEFORM    WAVE_SINE    /* 默认波形：正弦 */

/* 全局变量 */
static WaveformConfig g_waveConfig;
static uint8_t g_currentWaveform = DEFAULT_WAVEFORM;
static uint32_t g_currentFrequency = DEFAULT_FREQUENCY;
static uint16_t g_currentAmplitude = DEFAULT_AMPLITUDE;

/**
 * @brief  系统时钟配置 - 72MHz HSE + PLL
 */
static void SystemClock_Config(void)
{
    ErrorStatus HSEStartUpStatus;
    
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    HSEStartUpStatus = RCC_WaitForHSEStartUp();
    
    if (HSEStartUpStatus == SUCCESS)
    {
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
        FLASH_SetLatency(FLASH_Latency_2);
        
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        RCC_PCLK2Config(RCC_HCLK_Div1);
        RCC_PCLK1Config(RCC_HCLK_Div2);
        
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
        RCC_PLLCmd(ENABLE);
        
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        while (RCC_GetSYSCLKSource() != 0x08);
    }
}

/**
 * @brief  按键扫描 - 波形切换
 */
static uint8_t Key_Scan(void)
{
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == Bit_RESET)
    {
        Delay_ms(20);
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == Bit_RESET)
        {
            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == Bit_RESET);
            return 1;
        }
    }
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == Bit_RESET)
    {
        Delay_ms(20);
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == Bit_RESET)
        {
            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == Bit_RESET);
            return 2;
        }
    }
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == Bit_RESET)
    {
        Delay_ms(20);
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == Bit_RESET)
        {
            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == Bit_RESET);
            return 3;
        }
    }
    return 0;
}

/**
 * @brief  更新波形配置并切换
 */
static void UpdateWaveformConfig(void)
{
    g_waveConfig.waveType = g_currentWaveform;
    g_waveConfig.frequency = g_currentFrequency;
    g_waveConfig.amplitude_mV = g_currentAmplitude;
    g_waveConfig.sampleRate = SAMPLE_RATE;
    
    Waveform_Generate(&g_waveConfig);
    Timer_SetFrequency(g_currentFrequency);
    
    /* OLED显示当前状态 */
    OLED_Clear();
    OLED_ShowString(0, 0, "Waveform Generator");
    OLED_ShowString(0, 2, Waveform_GetTypeName(g_currentWaveform));
    
    char freqStr[20];
    if (g_currentFrequency >= 1000)
        sprintf(freqStr, "Freq: %d kHz", g_currentFrequency / 1000);
    else
        sprintf(freqStr, "Freq: %d Hz", g_currentFrequency);
    OLED_ShowString(0, 4, freqStr);
    
    char ampStr[20];
    sprintf(ampStr, "Amp: %d.%d V", g_currentAmplitude / 1000, (g_currentAmplitude % 1000) / 100);
    OLED_ShowString(0, 6, ampStr);
}

/**
 * @brief  主函数
 */
int main(void)
{
    /* 系统初始化 */
    SystemClock_Config();
    
    /* 外设初始化 */
    DAC_Init();
    Timer_Init(SAMPLE_RATE);
    TouchSensor_Init();
    UART_Debug_Init(115200);
    OLED_Init();
    
    /* 生成初始波形查找表 */
    Waveform_Init();
    
    /* 显示开机信息 */
    OLED_Clear();
    OLED_ShowString(0, 0, "Waveform Generator");
    OLED_ShowString(0, 2, "Initializing...");
    UART_Debug_Printf("STM32 Waveform Generator v1.0\r\n");
    UART_Debug_Printf("Author: Wu Shixi\r\n");
    UART_Debug_Printf("System Clock: 72MHz\r\n");
    
    Delay_ms(1000);
    
    /* 设置默认配置 */
    UpdateWaveformConfig();
    
    /* 启动DAC输出 */
    DAC_StartOutput();
    UART_Debug_Printf("DAC output started\r\n");
    
    /* 主循环 */
    while (1)
    {
        uint8_t key = Key_Scan();
        
        switch (key)
        {
            case 1: /* 波形切换: 正弦→方波→三角波→锯齿波 */
                g_currentWaveform = (g_currentWaveform + 1) % 4;
                UART_Debug_Printf("Waveform changed to: %s\r\n", 
                                  Waveform_GetTypeName(g_currentWaveform));
                UpdateWaveformConfig();
                break;
                
            case 2: /* 频率增加 */
                if (g_currentFrequency < 100000)
                {
                    if (g_currentFrequency < 1000)
                        g_currentFrequency += 100;
                    else if (g_currentFrequency < 10000)
                        g_currentFrequency += 1000;
                    else
                        g_currentFrequency += 10000;
                }
                UART_Debug_Printf("Frequency: %d Hz\r\n", g_currentFrequency);
                UpdateWaveformConfig();
                break;
                
            case 3: /* 频率减小 */
                if (g_currentFrequency > 10)
                {
                    if (g_currentFrequency <= 1000)
                        g_currentFrequency -= 100;
                    else if (g_currentFrequency <= 10000)
                        g_currentFrequency -= 1000;
                    else
                        g_currentFrequency -= 10000;
                    if (g_currentFrequency < 10) g_currentFrequency = 10;
                }
                UART_Debug_Printf("Frequency: %d Hz\r\n", g_currentFrequency);
                UpdateWaveformConfig();
                break;
        }
        
        /* 触摸检测 - 切换待机/工作状态 */
        if (TouchSensor_IsTouched())
        {
            DAC_ToggleOutput();
            UART_Debug_Printf("Output toggled by touch\r\n");
            Delay_ms(500);
        }
    }
}

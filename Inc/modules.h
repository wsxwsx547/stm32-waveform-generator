#ifndef __DAC_DRIVER_H
#define __DAC_DRIVER_H
#include "stm32f10x.h"
#include "waveform.h"
void DAC_Init(void);
void DAC_StartOutput(void);
void DAC_ToggleOutput(void);
void DAC_UpdateWaveform(uint8_t waveType);
#endif

#ifndef __TIMER_H
#define __TIMER_H
#include "stm32f10x.h"
#include "waveform.h"
void Timer_Init(uint32_t sampleRate);
void Timer_SetFrequency(uint32_t frequency);
void Delay_ms(uint32_t ms);
#endif

#ifndef __TOUCH_SENSOR_H
#define __TOUCH_SENSOR_H
#include "stm32f10x.h"
void TouchSensor_Init(void);
uint8_t TouchSensor_IsTouched(void);
uint16_t TouchSensor_ReadRawValue(void);
#endif

#ifndef __UART_DEBUG_H
#define __UART_DEBUG_H
#include "stm32f10x.h"
void UART_Debug_Init(uint32_t baudRate);
void UART_Debug_SendChar(char c);
void UART_Debug_Printf(const char* fmt, ...);
#endif

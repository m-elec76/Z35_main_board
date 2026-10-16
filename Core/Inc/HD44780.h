#ifndef HD44780_H
#define HD44780_H

#include "stm32G4xx_hal.h"

// Define GPIO ports and pins for LCD control and data lines
#define LCD_RS_PORT GPIOA
#define LCD_RS_PIN  GPIO_PIN_0

#define LCD_RW_PORT GPIOA
#define LCD_RW_PIN  GPIO_PIN_1

#define LCD_EN_PORT GPIOA
#define LCD_EN_PIN  GPIO_PIN_2

#define LCD_D4_PORT GPIOA
#define LCD_D4_PIN  GPIO_PIN_3

#define LCD_D5_PORT GPIOA
#define LCD_D5_PIN  GPIO_PIN_4

#define LCD_D6_PORT GPIOA
#define LCD_D6_PIN  GPIO_PIN_5

#define LCD_D7_PORT GPIOA
#define LCD_D7_PIN  GPIO_PIN_6

// Public functions
void HD44780_Init(void);
void HD44780_SendCommand(uint8_t cmd);
void HD44780_SendData(uint8_t data);
void HD44780_PrintStr(const char* str);
void HD44780_Clear(void);
void HD44780_SetCursor(uint8_t row, uint8_t col);

#endif // HD44780_H

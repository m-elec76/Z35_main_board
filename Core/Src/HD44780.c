#include "hd44780.h"

/**
 * @brief Write 4 bits to the LCD data pins (D4-D7).
 * @param data Lower 4 bits represent the data to send.
 */
static void HD44780_Write4Bits(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_D4_PORT, LCD_D4_PIN, (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5_PORT, LCD_D5_PIN, (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6_PORT, LCD_D6_PIN, (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7_PORT, LCD_D7_PIN, (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // Pulse Enable pin to latch data into LCD
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_SET);
    // Short delay to meet LCD timing requirements
    for (volatile int i = 0; i < 50; i++);
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_RESET);
    for (volatile int i = 0; i < 50; i++);
}

/**
 * @brief Send a byte to LCD with RS control.
 * @param data The byte to send.
 * @param rs Register Select flag (0 = command, 1 = data).
 */
static void HD44780_Send(uint8_t data, uint8_t rs)
{
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, rs ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_RESET); // RW = 0 for write

    HD44780_Write4Bits((data >> 4) & 0x0F);  // Send high nibble
    HD44780_Write4Bits(data & 0x0F);         // Send low nibble

    // Minimum delay for LCD to process data/command
    HAL_Delay(1);
}

/**
 * @brief Send a command byte to LCD.
 * @param cmd The command byte.
 */
void HD44780_SendCommand(uint8_t cmd)
{
    HD44780_Send(cmd, 0);
}

/**
 * @brief Send a data byte (character) to LCD.
 * @param data The data byte.
 */
void HD44780_SendData(uint8_t data)
{
    HD44780_Send(data, 1);
}

/**
 * @brief Initialize the LCD in 4-bit mode.
 */
void HD44780_Init(void)
{
    // Wait for LCD to power up
    HAL_Delay(50);

    // Set control pins low
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_RW_PORT, LCD_RW_PIN, GPIO_PIN_RESET);

    // Initialization sequence for 4-bit mode
    HD44780_Write4Bits(0x03);
    HAL_Delay(5);
    HD44780_Write4Bits(0x03);
    HAL_Delay(5);
    HD44780_Write4Bits(0x03);
    HAL_Delay(1);
    HD44780_Write4Bits(0x02);  // Set to 4-bit mode
    HAL_Delay(1);

    HD44780_SendCommand(0x28); // Function set: 4-bit, 2 lines, 5x8 dots
    HD44780_SendCommand(0x0C); // Display ON, cursor OFF, blink OFF
    HD44780_SendCommand(0x06); // Entry mode: increment cursor, no shift

    HD44780_Clear();           // Clear display and set cursor to home
}

/**
 * @brief Clear the LCD screen and reset cursor to home.
 */
void HD44780_Clear(void)
{
    HD44780_SendCommand(0x01); // Clear display command
    HAL_Delay(2);              // Clearing requires >1.52ms delay
}

/**
 * @brief Set cursor position on LCD.
 * @param row Row index (0 or 1).
 * @param col Column index (0 to 15).
 */
void HD44780_SetCursor(uint8_t row, uint8_t col)
{
    static const uint8_t row_offsets[] = {0x00, 0x40};

    if (row > 1) row = 1; // Limit to 2 rows

    HD44780_SendCommand(0x80 | (col + row_offsets[row]));
}

/**
 * @brief Print a null-terminated string on the LCD.
 * @param str Pointer to string.
 */
void HD44780_PrintStr(const char* str)
{
    while(*str)
    {
        HD44780_SendData((uint8_t)(*str));
        str++;
    }
}

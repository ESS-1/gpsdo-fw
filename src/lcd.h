#ifndef _LCD_H_
#define _LCD_H_

#include "fonts.h"
#include "main.h"
#include "st7735.h"

//TODO: Temprorary impl

inline static void LCD_Init(void)
{
    ST7735_Init();
}

inline static void LCD_Clear(void)
{
    ST7735_FillScreen(ST7735_COLOR565(50, 50, 50));
}

inline static void LCD_Puts(uint8_t x, uint8_t y, char* str)
{
    ST7735_WriteString(1 + x * 14, 1 + y * 10, str, Font_7x10, ST7735_WHITE, ST7735_BLACK);
}

inline static void LCD_CreateChar(uint8_t location, uint8_t* data)
{
    (void)location;
    (void)data;
}

inline static void LCD_PutCustom(uint8_t x, uint8_t y, uint8_t location)
{
    (void)x;
    (void)y;
    (void)location;
}

#endif

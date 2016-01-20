#ifndef UTILS_H
#define UTILS_H
#include <stm32f10x.h>
#include <stdarg.h>

struct USART_TypeDef;
void delay(int n);

void usart_putc(USART_TypeDef* usart, char ch);
char usart_getc(USART_TypeDef* usart);
void usart_print(USART_TypeDef* usart, const char*s);
//Read data from usart until \r\n found
int readline(USART_TypeDef* usart, char* buf, int size);
//Read data from usart
int read(USART_TypeDef* usart, char* buf, int size);
void itoa(int val, char* out, int out_size);
int strlen(const char* s);
int usart_printf(USART_TypeDef* usart, const char* fmt, ...);
int usart_vprintf(USART_TypeDef* usart, const char* fmt, va_list va);
int dbg_printf(const char* fmt, ...);

#endif

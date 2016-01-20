#include "utils.h"
#include <stm32f10x_usart.h>
#include <stdarg.h>
void assert_failed(uint8_t* file, uint32_t line) { }
void delay(int n)
{
    while(n > 0)
        n--;
}

void usart_putc(USART_TypeDef* usart, char ch)
{
    while(USART_GetFlagStatus(usart, USART_FLAG_TXE) == RESET);
    usart->DR = (ch & 0xff);
}
char usart_getc(USART_TypeDef* usart)
{
    while(USART_GetFlagStatus(usart, USART_FLAG_RXNE) == RESET);
    return USART_ReceiveData(usart);
}
void usart_print(USART_TypeDef* usart, const char*s)
{
    if(!s)
    {
        usart_print(usart, "<null>");
        return;
    }
    while(*s)
        usart_putc(usart, *s++);
}


int readline(USART_TypeDef* usart, char* buf, int size)
{
    int count = 0;
    do
    {
        char ch = usart_getc(usart);
        *buf++ = ch;
        count++;
    } while(buf[-1] != '\n' && count < size);
    if(count > 1 && buf[-2] == '\r')
        buf[-2] = 0;
    else if(count > 0 && buf[-1] == '\n')
        buf[-1] = 0;
    return count;
}
int read(USART_TypeDef* usart, char* buf, int size)
{
    int i;
    for(i = 0; i < size; i++)
    {
        buf[i] = usart_getc(usart);
    }
    return size;
}

void reverse(char* start, char * end)
{
    int len = end - start;
    int mid = len >> 1;
    int i;
    for(i = 0; i < mid; i++)
    {
        char t = start[i];
        int next = len - i - 1;
        start[i] = start[next];
        start[next] = t;
    }
}
void itoa(int val, char* out, int out_size)
{
#define WRITE(val) if(out_size > 0) {*out++ = val; out_size--;}
    char* start;
    if(val < 0)
    {
        WRITE('-');
        val = -val;
    }
    if(val == 0)
    {
        WRITE('0');
    }
    else
    {
        start = out;
        while(val > 0)
        {
            int digit = val % 10;
            WRITE(digit + '0');
            val /= 10;
        }
        reverse(start, out);
    }
    WRITE(0);
#undef WRITE
}

int dbg_printf(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int ret = usart_vprintf(USART1, fmt, va);
    va_end(va);
    usart_print(USART1, "\r\n");
    return ret;
}
int usart_printf(USART_TypeDef* usart, const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int ret = usart_vprintf(usart, fmt, va);
    va_end(va);
    return ret;
}
void itox(int val, char* out, int out_size)
{
    static const char* alphabet = "0123456789ABCDEF";
    int output = 0;
    for(int n = 32 - 4; n >= 0; n-= 4)
    {
        int v = (val >> n) & 0xf;
        output |= v;
        if(output)
            *out++ = alphabet[v];
    }
    if(!output)
        *out++ = '0';
    *out++ = 0;
}
int strlen(const char* s)
{
    int ret = 0;
    while(*s++)
        ret++;
    return ret;
}
int usart_vprintf(USART_TypeDef* usart, const char* fmt, va_list va)
{
    char tmp[10];
    for(const char*p = fmt; *p; p++)
    {
        char ch = *p;
        if(ch != '%')
        {
            usart_putc(usart, ch);
            continue;
        }
        p++;
        switch(*p)
        {
            case 'd':
                itoa(va_arg(va, int), tmp, sizeof(tmp));
                usart_print(usart, tmp);
                break;
            case 'c':
                usart_putc(usart, va_arg(va, int));
                break;
            case 's':
                usart_print(usart, va_arg(va, char*));
                break;
            case 'x':
                itox(va_arg(va, int), tmp, sizeof(tmp));
                usart_print(usart, tmp);
                break;
            case '%':
                usart_putc(usart, '%');
                break;
            default:
                break;
        }
    }
    va_end(va);
    return 0;
}

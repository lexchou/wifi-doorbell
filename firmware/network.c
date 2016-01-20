#include "network.h"
#include "utils.h"
#include <stm32f10x_usart.h>

#define RAK410 USART2
#define RAK410_OK 0
void rak410_write(const char* cmd)
{
    usart_print(RAK410, cmd);
}
void rak410_printf(const char*cmd, ...)
{
    va_list va;
    va_start(va, cmd);
    usart_vprintf(RAK410, cmd, va);
    va_end(va);
    usart_print(RAK410, "\r\n");
}


int rak410_result(int* error_code)
{
    char buf[10];
    read(RAK410, buf, 2);
    if(buf[0] == 'O' && buf[1] == 'K')
        *error_code = RAK410_OK;
    else if(buf[0] == 'E' && buf[1] == 'R')
    {
        //consume ROR
        read(RAK410, buf, 6);
        *error_code = buf[3];
        //read until \n
    }
    else//invalid response
    {
        *error_code = 123;
        buf[2] = 0;
        dbg_printf("ERROR: rak410_result unrecognized response: %x %c %x %c", buf[0], buf[0], buf[1], buf[1]);
        return 0;
    }
    return 1;
}


void rak410_checkresult()
{
    int ec = 0;
    char ch;
    if(!rak410_result(&ec))
        return;
    if(ec)
    {
        dbg_printf("Error detected, code: %x", ec);
        return;
    }
    //read until \r \n
    do
    {
        ch = usart_getc(RAK410);
    } while(ch != '\n');
    dbg_printf("Successed");
}

int readline_debug(char* buf, int size)
{

    int count = 0;
    dbg_printf("Debug readline");
    do
    {
        char ch = usart_getc(RAK410);
        USART1->DR = (ch & 0xff);
        *buf++ = ch;
        count++;
    } while(buf[-1] != '\n' && count < size);
    if(count > 1 && buf[-2] == '\r')
        buf[-2] = 0;
    else if(count > 0 && buf[-1] == '\n')
        buf[-1] = 0;
    return count;
    
}
void rak410_reset()
{
    char buf[20];
    dbg_printf("Resetting ");
    //set MCU_RST to low
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
    //delay
    delay(100000);
    dbg_printf("Wait for RAK410 response");
    //set MCU_RST to high
    GPIO_SetBits(GPIOA, GPIO_Pin_1);

    readline_debug(buf, sizeof(buf));
    dbg_printf("Received: %s", buf);
    dbg_printf("Scanning network");
    rak410_printf("at+scan=0");
    rak410_checkresult();

    dbg_printf("Entering high performance mode");
    rak410_printf("at+pwrmode=0");
    rak410_checkresult();
}

void rak410_connect(const char* ssid, const char* password)
{
    dbg_printf("Setting password...");
    rak410_printf("at+psk=%s", password);
    rak410_checkresult();

    dbg_printf("Connecting...");
    rak410_printf("at+connect=%s", ssid);
    rak410_checkresult();

    dbg_printf("DHCP...");
    rak410_printf("at+ipdhcp=0");
    int ec = 0;
    if(!rak410_result(&ec))
        return;
    if(ec == RAK410_OK)//OK
    {
        char buf[22];
        char*p = buf;
        read(RAK410, buf, sizeof(buf));//IP
        dbg_printf("IP=%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
        p+=4;
        dbg_printf("Mask=%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
        p+=4;
        dbg_printf("Gateway=%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
        p+=4;
        dbg_printf("DNS1=%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
        p+=4;
        dbg_printf("DNS2=%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
    }
    rak410_rssi();
}
void rak410_rssi()
{
    int ec = 0;
    dbg_printf("Loading RSSI... ");
    rak410_printf("at+rssi");
    if(!rak410_result(&ec))
        return;
    if(ec == RAK410_OK)
    {
        char buf[3];
        read(RAK410, buf, 3);
        int rssi = buf[0] - 256;
        dbg_printf("RSSI=%ddb", rssi);
    }
}
void rak410_scan()
{
    char buf[10];
    dbg_printf("Scanning");
    rak410_printf("at+get_scan=10");
    readline(RAK410, buf, sizeof(buf));
    //rak410_result(&ec, &buf, 20);
    dbg_printf("Scan result: %s", buf);
}
void rak410_http_get(const char* url)
{
    int res = 0;
    char buf[10];
    dbg_printf("Sending http get request:%s", url);
    rak410_printf("at+http_get=%s", url);
    rak410_result(&res);
    {
        if(res == 0)
        {
            short status, len;
            //read(RAK410, (char*)&status, 2);//status
            //read(USART2, (char*)&len, 2);//page_length
            read(RAK410, buf, 4);
            status = (buf[0] << 16) | buf[1];
            len = (buf[2] << 16) | buf[3];
            //dbg_printf("GET: %x %x %x %x", buf[0], buf[1], buf[2], buf[3]);
            for(int i = 0; i < len; i++)
            {
                usart_getc(RAK410);
            }
            dbg_printf("HTTP Get status:%d, length: %d", status, len);
        }
        else
        {
            dbg_printf("HTTP Get response error code: %x", res);
        }
        dbg_printf("GET done");
    }
}
int rak410_tcp(const char* dest_ip, int dest_port)
{
    static int module_port = 1000;
    int ec = 0;
    dbg_printf("dest_ip = %s, dest_port = %d, module_port = %d", dest_ip, dest_port, module_port);
    rak410_printf("at+tcp=%s,%d,%d", dest_ip, dest_port, module_port);
    module_port++;
    rak410_result(&ec);
    if(ec == RAK410_OK)
    {
        char buf[3];
        read(RAK410, buf, sizeof(buf));
        return buf[0];
    }
    dbg_printf("Failed to create tcp client, error = %x", ec);
    return -1;
}
void rak410_close(int socket)
{
    int ec = 0;
    char buf[2];
    rak410_printf("at+cls=%d", socket);
    rak410_result(&ec);
    if(ec == RAK410_OK)
        read(RAK410, buf, sizeof(buf));
}
void rak410_send(int socket, int len, const char* data)
{
    int ec = 0;
    char buf[2];
    rak410_printf("at+send_data=%d,%d,%s", socket, len, data);
    rak410_result(&ec);
    if(ec == RAK410_OK)
        read(RAK410, buf, sizeof(buf));
}

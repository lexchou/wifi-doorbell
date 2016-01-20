#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_usart.h>
#include "utils.h"
#include "driver.h"
#include "network.h"

#define SSID "MOZILLA"
#define PASSWORD "18602757626"
#define HOST "10.0.0.242"
#define PORT 3000

volatile int pressed = 0;
volatile int data = 0;
volatile int interupt = 0;

char receiveBuffer[200];
int receiveCount = 0;

void EXTI1_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        pressed = 1;
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}
void __USART2_IRQHandler (void)
{
    char ch;
    interupt = 1;
    if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET)
    {
        ch = USART_ReceiveData(USART2);
        receiveBuffer[receiveCount++] = ch;
        receiveBuffer[receiveCount] = 0;
        data = 1;
    }
}


void doorbell(const char* cmd)
{
    int socket;
    socket = rak410_tcp(HOST, PORT);
    if(socket == -1)
    {
        dbg_printf("Cannot create socket, failed to send %s", cmd);
        return;
    }
    rak410_send(socket, strlen(cmd), cmd);
    rak410_close(socket);
    dbg_printf("Command %s sent", cmd);
}


int main(void)
{
    uint8_t last_state = 0;
    uint8_t state = 0;
    
    Init_Drivers();
    dbg_printf("\r\r\r\nWiFi Doorbell started!");
    rak410_reset();
    rak410_connect(SSID, PASSWORD);
    doorbell("up\n");
    doorbell("up2\n");
    rak410_rssi();
    dbg_printf("Main loop");
    while(1)
    {
        
        state = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
        if(state != last_state)
        {
            dbg_printf("PB0=%d", state);
            last_state = state;
        }
        if(data)
        {
            data = 0;
            dbg_printf("RAk received:%s", receiveBuffer);
            receiveCount = 0;
        }
        if(interupt)
        {
            dbg_printf("RAK410 interrupted");
            interupt = 0;
        }
        if(pressed)
        {
            dbg_printf("Key pressed");
            doorbell("ring\n");
            delay(10000000);
            pressed = 0;
        }
    }
}


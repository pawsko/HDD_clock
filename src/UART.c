/*
 * UART.c
 *
 *  Created on: 24.08.2018
 *      Author: DjPifPaf
 */

#include "stm32f10x.h"


void send_char(char c)
{
 while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
 USART_SendData(USART2, c);
}

int __io_putchar(int c)
{
    send_char(c);
    return c;
}

void init_UART(void)
{
	 GPIO_InitTypeDef gpio;
	 USART_InitTypeDef uart;

	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	 GPIO_StructInit(&gpio);
	 gpio.GPIO_Pin = GPIO_Pin_2;
	 gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	 GPIO_Init(GPIOA, &gpio);

	 gpio.GPIO_Pin = GPIO_Pin_3;
	 gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	 GPIO_Init(GPIOA, &gpio);

	 USART_StructInit(&uart);
	 uart.USART_BaudRate = 115200;
	 USART_Init(USART2, &uart);

	 USART_Cmd(USART2, ENABLE);

}

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dma.h>
#include "responsive_adc.h"
#include "defines.h"






//*************************************************************
// things for handling strings and printing them

void uart_puts(char *string) 
{
    while (*string) 
	{
        usart_send_blocking(USART_CONSOLE, *string);
        string++;
    }
}


void uart_putln(char *string) 
{
    uart_puts(string);
    uart_puts("\r\n");
}

void printValue(uint32_t value)
{
	char MyString[] = "12345678901234567890";
    itoa(value, MyString, 10);
    uart_puts(MyString);
}

/*
 * MIT License
 *
 * Copyright (c) 2021 Md. Mahmudul Hasan Sumon
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */





#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>


#include "usart.h"

#include "FreeRTOS.h"
#include "queue.h"

#ifndef F_CPU 
#error F_CPU "isn't defined. Please define F_CPU to 8000000UL to make this code working properly."
#else
#if F_CPU != 8000000UL
#error "Please set F_CPU to 8000000UL or add logic for other clock speed."
#endif
#endif


typedef struct
{
	UsartId id;
	BaudRate baud;
	size_t rx_bf_len;
	size_t tx_bf_len;
	xQueueHandle rx_queue;
	xQueueHandle tx_queue;
	bool is_initialised;	
}Usart;

Usart * usart[CONFIG_MAX_NUMBER_OF_USART];

UsartHandle UsartInit(UsartId id, BaudRate baud, size_t rx_buf_len, size_t tx_buf_len)
{
	Usart * usrt = NULL;
	if (id == USART_ID_0)
	{
		UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
		UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
		UBRR0 = 51;
		
		if (usart[0] != NULL)
		{
			if(usart[0]->is_initialised)
			{
				return usart[0];
			}
			usrt = usart[0];
		}
		else
		{
			usart[0] = pvPortMalloc(sizeof(Usart));
			usrt = usart[0];
		}
	}
	usrt->is_initialised = true;
	usrt->id = id;
	usrt->baud = BAUDRATE_9600;
	usrt->rx_bf_len = rx_buf_len;
	usrt->tx_bf_len = tx_buf_len;
	usrt->rx_queue = xQueueCreate(rx_buf_len, sizeof(uint8_t));
	usrt->tx_queue = xQueueCreate(tx_buf_len, sizeof(uint8_t));
	return usrt;
}

size_t UsartRead(UsartHandle handle, uint8_t * buffer, uint16_t len)
{
	if(handle == NULL)
		return 0;
	size_t ret;
	Usart * urt = handle;
	for(size_t i = 0; i < len; i ++)
	{
		if (xQueueReceive(urt->rx_queue, &buffer[i], 1000) != pdTRUE)
		{
			ret = i;
			break;
		}
	}
	ret = len;
	return ret;
}


bool UsartReadByte(UsartHandle handle, uint8_t * buffer)
{
	if(handle == NULL)
	return false;
	Usart * urt = handle;
	if (xQueueReceive(urt->rx_queue, buffer, 1000) == pdTRUE)
	{
		return true;
	}
	return false;
}

size_t UsartWrite(UsartHandle handle, const uint8_t * data, uint16_t len)
{
	if(handle == NULL)
	return 0;
	size_t ret;
	Usart * urt = handle;
	for (size_t i = 0; i < len; i++)
	{
		if(xQueueSend(urt->tx_queue, &data[i], 1000) != pdTRUE)
		{
			ret = i;
			break;
		}
		else
		{
			UCSR0B |= 1 << UDRE0;
		}
	}
	ret = len;
	return ret;
}


bool UsartWriteByte(UsartHandle handle, const uint8_t data)
{
	if(handle == NULL)
	return false;
	Usart * urt = handle;
	if(xQueueSend(urt->tx_queue, &data, 1000) == pdTRUE)
	{ 
		UCSR0B |= 1 << UDRE0;
		return true;
	}
	return false;
}

size_t UsartWriteString(UsartHandle handle, const char * str)
{
	return UsartWrite(handle, (const uint8_t *) str, strlen(str));
}


ISR(USART_UDRE_vect)
{
	BaseType_t wake_token = pdFALSE;
	uint8_t data;
	if (xQueueReceiveFromISR(usart[0]->tx_queue, &data, &wake_token) == pdTRUE)
	{
		UDR0 = data;
	}
	else
	{
		UCSR0B &= ~(1 << UDRE0);
	}
	if (wake_token != pdFALSE)
	{
		taskYIELD();
	}
}

ISR(USART_RX_vect)
{
	BaseType_t wake_token = pdFALSE;
	uint8_t data = UDR0;
	xQueueSendFromISR(usart[0]->rx_queue, &data, &wake_token);
	if(wake_token == pdTRUE)
		taskYIELD();
}
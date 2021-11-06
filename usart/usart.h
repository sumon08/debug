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


#ifndef USART_INCLUDE_H_
#define USART_INCLUDE_H_


#include <stdbool.h>
#include <stddef.h>

#include "config.h"



typedef void * UsartHandle;

typedef enum 
{
	USART_ID_0,
#if CONFIG_MAX_NUMBER_OF_USART > 1
	USART_ID_1,
#endif
#if CONFIG_MAX_NUMBER_OF_USART > 2	
	USART_ID_2
#endif
}UsartId;

typedef enum 
{
	BAUDRATE_2400,
	BAUDRATE_4800,
	BAUDRATE_9600,
	BAUDRATE_19200,
	BAUDRATE_38400,
	BAUDRATE_57600,
	BAUDRATE_115200
}BaudRate;



UsartHandle UsartInit(UsartId id, BaudRate baud, size_t rx_buf_len, size_t tx_buf_len);

size_t UsartRead(UsartHandle handle, uint8_t * buffer, uint16_t len);
bool UsartReadByte(UsartHandle handle, uint8_t * buffer);

size_t UsartWrite(UsartHandle handle, const uint8_t * data, uint16_t len);
bool UsartWriteByte(UsartHandle handle, const uint8_t data);
size_t UsartWriteString(UsartHandle handle, const char * str);




#endif /* USART_INCLUDE_H_ */
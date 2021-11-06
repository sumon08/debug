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

#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "console.h"


ConsoleChannel main_con;

void TestTask(void * param)
{
	DDRB |= 1 << PORTB0;
	while(1)
	{
		ConsoleInfo(main_con, "Hello! This is a test.");
		PORTB ^= 1 << PORTB0;
		vTaskDelay(1000);
	}
}

void MainDebugHandler(char * reply, const char ** lst, uint16_t len)
{
	if(strcmp(lst[0], "echo") == 0)
	{
		strcpy(reply, lst[1]);
	}
	else
	{
		strcpy(reply, "Unknown command <");
		strcat(reply, lst[0]);
		strcat(reply, ">.");
	}
}

int main(void)
{
	ConsoleInit();
	main_con = ConsoleCreate("MAIN", MainDebugHandler);
	xTaskCreate(TestTask, "", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	vTaskStartScheduler();    
    while (1) 
    {
    }
}


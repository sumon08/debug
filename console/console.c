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


#include <stdarg.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "console.h"
#include <stdbool.h>
#include <string.h>
#include "usart.h"

typedef enum
{
	CONSOLE_MESSAGE_INFO,
	CONSOLE_MESSAGE_WARN,
	CONSOLE_MESSAGE_ERROR,
	CONSOLE_MESSAGE_REPLY
} ConsoleMessageType;

typedef struct _dbg
{
	char key[10];
	ConsoleHandler handler;
	bool enable_error_log;
	bool enable_warning;
	bool enable_info;

	struct _dbg *pNext;
} ConsoleNode;

typedef struct
{
	bool enable_error_log;
	bool enable_warning;
	bool enable_info;

	xSemaphoreHandle lock;

	UsartHandle port;

	ConsoleNode *pHead;
	ConsoleNode *con_node;

	char reply[CONFIG_CONSOLE_REPLY_BUFFER_LENGTH];
	uint8_t buffer[CONFIG_CONSOLE_COMMAND_BUFFER_LENGTH];
	uint8_t index;
} ConsoleManager;

ConsoleManager con_man;

void ConsoleTask(void *param);
void ConsoleKeyHandler(char *reply, const char **param, uint16_t count);
void ConsoleSendByte(uint8_t byte);

void ConsoleInit()
{
	con_man.lock = xSemaphoreCreateBinary();
	xSemaphoreGive(con_man.lock);

	ConsoleNode *node = pvPortMalloc(sizeof(ConsoleNode));
	node->handler = ConsoleKeyHandler;
	node->enable_error_log = true;
	node->enable_info = true;
	node->enable_warning = true;
	strcpy(node->key, "CONSOLE");
	node->pNext = NULL;

	con_man.pHead = node;
	con_man.con_node = node;

	con_man.enable_error_log = true;
	con_man.enable_info = true;
	con_man.enable_warning = true;
	con_man.port = UsartInit(USART_ID_0, BAUDRATE_9600, 64, 64);
	xTaskCreate(ConsoleTask, "Con", 164, NULL, 3, NULL);
}

ConsoleChannel ConsoleCreate(const char *key, ConsoleHandler handler)
{
	ConsoleNode *node = pvPortMalloc(sizeof(ConsoleNode));
	node->handler = handler;
	strcpy(node->key, key);
	node->pNext = NULL;
	node->pNext = con_man.pHead;
	con_man.pHead = node;

	node->enable_error_log = true;
	node->enable_info = true;
	node->enable_warning = true;

	return node;
}

void ConsoleSendKey(ConsoleMessageType type, const char *module)
{	UsartWriteByte(con_man.port, '>');
	UsartWriteString(con_man.port, module);
	UsartWriteByte(con_man.port, '[');
	switch (type)
	{
		case CONSOLE_MESSAGE_INFO:
		UsartWriteString(con_man.port, "INFO");
		break;
		case CONSOLE_MESSAGE_WARN:
		UsartWriteString(con_man.port, "WARN");
		break;

		case CONSOLE_MESSAGE_ERROR:
		UsartWriteString(con_man.port, "ERROR");
		break;

		case CONSOLE_MESSAGE_REPLY:
		UsartWriteString(con_man.port, "REPLY");
		break;

		default:
		break;
	}

	UsartWriteByte(con_man.port, ']');
	UsartWriteByte(con_man.port, ':');
	UsartWriteByte(con_man.port, ' ');
}

void ConsoleError(ConsoleChannel ch, const char *error)
{
	if (ch == NULL)
	return;
	ConsoleNode *nch = (ConsoleNode *)ch;
	if (con_man.enable_error_log != true || nch->enable_error_log != true)
	return;
	if (xSemaphoreTake(con_man.lock, 10000) != pdFALSE)
	{
		ConsoleSendKey(CONSOLE_MESSAGE_ERROR, nch->key);
		UsartWriteString(con_man.port, error);
		UsartWriteByte(con_man.port, CONFIG_CONSOLE_LINE_ENDING_CHAR);
		xSemaphoreGive(con_man.lock);
	}
}

void ConsoleInfo(ConsoleChannel ch, const char *info)
{
	if (ch == NULL)
	return;
	ConsoleNode *nch = (ConsoleNode *)ch;
	if (con_man.enable_info != true || nch->enable_info != true)
	return;

	if (xSemaphoreTake(con_man.lock, 10000) != pdFALSE)
	{
		ConsoleSendKey(CONSOLE_MESSAGE_INFO, nch->key);
		UsartWriteString(con_man.port, info);
		UsartWriteByte(con_man.port, CONFIG_CONSOLE_LINE_ENDING_CHAR);
		xSemaphoreGive(con_man.lock);
	}
}

void ConsoleWarning(ConsoleChannel ch, const char *warning)
{
	if (ch == NULL)
	return;
	ConsoleNode *nch = (ConsoleNode *)ch;
	if (con_man.enable_warning != true || nch->enable_warning != true)
	return;
	if (xSemaphoreTake(con_man.lock, 10000) != pdFALSE)
	{
		ConsoleSendKey(CONSOLE_MESSAGE_WARN, nch->key);
		UsartWriteString(con_man.port, warning);
		UsartWriteByte(con_man.port, CONFIG_CONSOLE_LINE_ENDING_CHAR);
		xSemaphoreGive(con_man.lock);
	}
}


void ToUpperCase(char * input)
{
	size_t len = strlen(input);
	for (int i = 0; i < len; i++)
	{
		if(input[i] >= 'a' && input[i] <= 'z')
		{
			input[i] -= 32;
		}
	}
}

void HandleInputKey(char *str)
{
	char *ptr = str;
	uint8_t len = strlen(str);
	char *lst[20];
	uint8_t count = 0;
	for (int i = 0; i < len; i++)
	{
		if (ptr[i] == ' ')
		{
			ptr[i] = 0;
			i++;
			if (count > 19)
			return;
			lst[count++] = &ptr[i];
		}
	}

	ConsoleNode *node = con_man.pHead;
	bool flag = 0;

	while (node != NULL)
	{
		//DebugSendString(node->key);
		ToUpperCase(str);
		if (strcmp(node->key, str) == 0)
		{
			if (node->handler != NULL)
			{
				char temp[10];
				memset(temp, 0, 10);
				strncpy(temp, lst[0], 9);
				ToUpperCase(temp);
				if (strcmp(temp, "ERROR") == 0)
				{
					ToUpperCase(lst[1]);
					if (strcmp(lst[1], "ON") == 0)
					{
						node->enable_error_log = true;
						strcpy(con_man.reply, "Error log of <");
						strcat(con_man.reply, node->key);
						strcat(con_man.reply, "> turned on.\r\n");
					}
					else if (strcmp(lst[1], "OFF") == 0)
					{
						node->enable_error_log = false;
						strcpy(con_man.reply, "Error log of <");
						strcat(con_man.reply, node->key);
						strcat(con_man.reply, "> turned off.\r\n");
					}
					else
					{
						strcpy(con_man.reply, "Unknown option for ");
						strcat(con_man.reply, node->key);
						strcat(con_man.reply, " -> ");
						strcat(con_man.reply, lst[0]);
						strcat(con_man.reply, " command.\r\n");
					}
				}
				else if (strcmp(temp, "WARN") == 0)
				{
					if (strcmp(lst[1], "ON") == 0)
					{
						node->enable_warning = true;
						strcpy(con_man.reply, "Warning log of <");
						strcat(con_man.reply, node->key);
						strcat(con_man.reply, "> turned on.\r\n");
					}
					else if (strcmp(lst[1], "OFF") == 0)
					{
						node->enable_warning = false;
						strcpy(con_man.reply, "Warning log of <");
						strcat(con_man.reply, node->key);
						strcat(con_man.reply, "> turned off.\r\n");
					}
					else
					{
						strcpy(con_man.reply, "Unknown option for ");
						strcat(con_man.reply, node->key);
						strcat(con_man.reply, " -> ");
						strcat(con_man.reply, lst[0]);
						strcat(con_man.reply, " command.\r\n");
					}
				}
				else if (strcmp(temp, "INFO") == 0)
				{
					if (strcmp(lst[1], "ON") == 0)
					{
						node->enable_info = true;
						strcpy(con_man.reply, "Info log of <");
						strcat(con_man.reply, node->key);
						strcat(con_man.reply, "> turned on.\r\n");
					}
					else if (strcmp(lst[1], "OFF") == 0)
					{
						node->enable_info = false;
						strcpy(con_man.reply, "Info log of <");
						strcat(con_man.reply, node->key);
						strcat(con_man.reply, "> turned off.\r\n");
					}
					else
					{
						strcpy(con_man.reply, "Unknown option for ");
						strcat(con_man.reply, node->key);
						strcat(con_man.reply, " -> ");
						strcat(con_man.reply, lst[0]);
						strcat(con_man.reply, " command.\r\n");
					}
				}
				else if(strcmp(temp, "ALL") == 0)
				{
					if (strcmp(lst[1], "ON") == 0)
					{
						node->enable_info = true;
						node->enable_warning = true;
						node->enable_error_log = true;
						strcpy(con_man.reply, "Info, Warning and Error log of <");
						strcat(con_man.reply, node->key);
						strcat(con_man.reply, "> turned on.\r\n");
					}
					else if (strcmp(lst[1], "OFF") == 0)
					{
						node->enable_info = false;
						node->enable_warning = false;
						node->enable_error_log = false;
						strcpy(con_man.reply, "Info, Warning and Error log of <");
						strcat(con_man.reply, node->key);
						strcat(con_man.reply, "> turned off.\r\n");
					}
					else
					{
						strcpy(con_man.reply, "Unknown option for ");
						strcat(con_man.reply, node->key);
						strcat(con_man.reply, " -> ");
						strcat(con_man.reply, lst[0]);
						strcat(con_man.reply, " command.\r\n");
					}
				}
				else
				{
					memset(con_man.reply, 0, CONFIG_CONSOLE_REPLY_BUFFER_LENGTH);
					node->handler((char *)con_man.reply, (const char **)lst, count);
				}

				flag = true;
				break;
			}
		}
		node = node->pNext;
	}

	if (flag == false)
	strcpy((char *)con_man.reply, "Command module not registered or Not implemented.\r\n");

	if (xSemaphoreTake(con_man.lock, 1000) != pdFALSE)
	{
		if (node == NULL)
		{
			node = con_man.con_node;
		}
		ConsoleSendKey(CONSOLE_MESSAGE_REPLY, node->key); 
		UsartWriteString(con_man.port, (const char *)con_man.reply);
		UsartWriteByte(con_man.port, CONFIG_CONSOLE_LINE_ENDING_CHAR);
		xSemaphoreGive(con_man.lock);
	}
}

void ConsoleTask(void *param)
{
	while (true)
	{
		uint8_t data = 0;
		if (UsartReadByte(con_man.port, &data) != false)
		{
			if (data == '\n')
			{
				if (con_man.index != 0)
				{
					HandleInputKey((char *)con_man.buffer);
					memset(con_man.buffer, 0, CONFIG_CONSOLE_COMMAND_BUFFER_LENGTH);
					con_man.index = 0;
				}
			}
			else
			{
				con_man.buffer[con_man.index++] = data;
				if (con_man.index >= CONFIG_CONSOLE_COMMAND_BUFFER_LENGTH)
				{
					memset(con_man.buffer, 0, CONFIG_CONSOLE_COMMAND_BUFFER_LENGTH);
					con_man.index = 0;
				}
			}
		}
	}
}

void ConsoleKeyHandler(char *reply, const char **param, uint16_t count)
{
	if (count == 2)
	{
		if (strcmp(param[0], "ERROR") == 0)
		{
			if (strcmp(param[1], "ON") == 0)
			{
				con_man.enable_error_log = true;
				strcpy(reply, "Error logging turned on!");
			}
			else if (strcmp(param[1], "OFF") == 0)
			{
				con_man.enable_error_log = false;
				strcpy(reply, "Error logging turned off!");
			}
			else
			{
				strcpy(reply, "Unknown error log setting!");
			}
		}
		else if (strcmp(param[0], "WARN") == 0)
		{
			if (strcmp(param[1], "ON") == 0)
			{
				con_man.enable_warning = true;
				strcpy(reply, "Warning logging turned on!");
			}
			else if (strcmp(param[1], "OFF") == 0)
			{
				con_man.enable_warning = false;
				strcpy(reply, "Warning logging turned off!");
			}
			else
			{
				strcpy(reply, "Unknown error log setting!");
			}
		}
		else if (strcmp(param[0], "INFO") == 0)
		{
			if (strcmp(param[1], "ON") == 0)
			{
				con_man.enable_info = true;
				strcpy(reply, "Info logging turned on!");
			}
			else if (strcmp(param[1], "OFF") == 0)
			{
				con_man.enable_info = false;
				strcpy(reply, "Info logging turned off!");
			}
			else
			{
				strcpy(reply, "Unknown info log setting!");
			}
		}
	}
}


// Code borrowed from Internet. Unfortunately I forgot the name of the author. Below code isn't written by me

#define PAD_RIGHT 1
#define PAD_ZERO 2

static void printchar(char **out, unsigned int c)
{
	UsartWriteByte(con_man.port, (uint8_t)c);
}

static int prints(char **out, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

	if (width > 0)
	{
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr)
		++len;
		if (len >= width)
		width = 0;
		else
		width -= len;
		if (pad & PAD_ZERO)
		padchar = '0';
	}
	if (!(pad & PAD_RIGHT))
	{
		for (; width > 0; --width)
		{
			printchar(out, padchar);
			++pc;
		}
	}
	for (; *string; ++string)
	{
		printchar(out, *string);
		++pc;
	}
	for (; width > 0; --width)
	{
		printchar(out, padchar);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0)
	{
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints(out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0)
	{
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN - 1;
	*s = '\0';

	while (u)
	{
		t = u % b;
		if (t >= 10)
		t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg)
	{
		if (width && (pad & PAD_ZERO))
		{
			printchar(out, '-');
			++pc;
			--width;
		}
		else
		{
			*--s = '-';
		}
	}

	return pc + prints(out, s, width, pad);
}

static int print(char **out, const char *format, va_list args)
{
	register int width, pad;
	register int pc = 0;
	char scr[2];

	for (; *format != 0; ++format)
	{
		if (*format == '%')
		{
			++format;
			width = pad = 0;
			if (*format == '\0')
			break;
			if (*format == '%')
			goto out;
			if (*format == '-')
			{
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0')
			{
				++format;
				pad |= PAD_ZERO;
			}
			for (; *format >= '0' && *format <= '9'; ++format)
			{
				width *= 10;
				width += *format - '0';
			}
			if (*format == 's')
			{
				register char *s = (char *)va_arg(args, int);
				pc += prints(out, s ? s : "(null)", width, pad);
				continue;
			}
			if (*format == 'd')
			{
				pc += printi(out, va_arg(args, int), 10, 1, width, pad, 'a');
				continue;
			}
			if (*format == 'x')
			{
				pc += printi(out, va_arg(args, int), 16, 0, width, pad, 'a');
				continue;
			}
			if (*format == 'X')
			{
				pc += printi(out, va_arg(args, int), 16, 0, width, pad, 'A');
				continue;
			}
			if (*format == 'u')
			{
				pc += printi(out, va_arg(args, int), 10, 0, width, pad, 'a');
				continue;
			}
			if (*format == 'c')
			{
				/* char are converted to int then pushed on the stack */
				scr[0] = (char)va_arg(args, int);
				scr[1] = '\0';
				pc += prints(out, scr, width, pad);
				continue;
			}
		}
		else
		{
			out:
			printchar(out, *format);
			++pc;
		}
	}
	if (out)
	**out = '\0';
	va_end(args);
	return pc;
}

int printf(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	return print(0, format, args);
}

// Below code is based on formatted output code from internet

void ConsoleInfof(ConsoleChannel ch, const char *format, ...)
{

	if (ch == NULL)
	return;
	ConsoleNode *nch = (ConsoleNode *)ch;
	if (con_man.enable_info != true || nch->enable_info != true)
	return;
	if (xSemaphoreTake(con_man.lock, 10000) != pdFALSE)
	{
		ConsoleSendKey(CONSOLE_MESSAGE_INFO, nch->key);
		va_list args;

		va_start(args, format);
		print(0, format, args);
		UsartWriteByte(con_man.port, CONFIG_CONSOLE_LINE_ENDING_CHAR);
		xSemaphoreGive(con_man.lock);
	}
}

void ConsoleWarnf(ConsoleChannel ch, const char *format, ...)
{
	if (ch == NULL)
	return;
	ConsoleNode *nch = (ConsoleNode *)ch;
	if (con_man.enable_warning != true || nch->enable_warning != true)
	return;
	if (xSemaphoreTake(con_man.lock, 10000) != pdFALSE)
	{
		ConsoleSendKey(CONSOLE_MESSAGE_WARN, nch->key);
		va_list args;

		va_start(args, format);
		print(0, format, args);
		UsartWriteByte(con_man.port, CONFIG_CONSOLE_LINE_ENDING_CHAR);
		xSemaphoreGive(con_man.lock);
	}
}

void ConsoleErrorf(ConsoleChannel ch, const char *format, ...)
{
	if (ch == NULL)
	return;
	ConsoleNode *nch = (ConsoleNode *)ch;
	if (con_man.enable_error_log != true || nch->enable_error_log != true)
	return;
	if (xSemaphoreTake(con_man.lock, 10000) != pdFALSE)
	{
		ConsoleSendKey(CONSOLE_MESSAGE_ERROR, nch->key);
		va_list args;

		va_start(args, format);
		print(0, format, args);
		UsartWriteByte(con_man.port, CONFIG_CONSOLE_LINE_ENDING_CHAR);
		xSemaphoreGive(con_man.lock);
	}
}
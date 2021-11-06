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


#ifndef CONSOLE_INCLUDE_H_
#define CONSOLE_INCLUDE_H_



#include <stdint.h>


typedef void * ConsoleChannel;

typedef void (*ConsoleHandler)(char *reply, const char **param_list, uint16_t count);

void ConsoleInit();
ConsoleChannel ConsoleCreate(const char *key, ConsoleHandler handler);

void ConsoleError(ConsoleChannel ch, const char *error);
void ConsoleInfo(ConsoleChannel ch, const char *info);
void ConsoleWarning(ConsoleChannel ch, const char *warning);
void ConsoleInfof(ConsoleChannel ch, const char *format, ...);
void ConsoleErrorf(ConsoleChannel ch, const char *format, ...);
void ConsoleWarnf(ConsoleChannel ch, const char *format, ...);




#endif /* CONSOLE_INCLUDE_H_ */
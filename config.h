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

#ifndef CONFIG_INCLUDE_H_
#define CONFIG_INCLUDE_H_

#ifndef CONFIG_MAX_NUMBER_OF_USART
#define CONFIG_MAX_NUMBER_OF_USART	1
#endif

#ifndef CONFIG_CONSOLE_LINE_ENDING_CHAR	
#define CONFIG_CONSOLE_LINE_ENDING_CHAR	10
#endif


#ifndef CONFIG_CONSOLE_REPLY_BUFFER_LENGTH
#define CONFIG_CONSOLE_REPLY_BUFFER_LENGTH		64
#endif

#ifndef CONFIG_CONSOLE_COMMAND_BUFFER_LENGTH	
#define CONFIG_CONSOLE_COMMAND_BUFFER_LENGTH	48
#endif


#endif /* CONFIG_INCLUDE_H_ */
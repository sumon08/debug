Simple Console interface for Microcontroller like AVR and CortexM.

Setup:
	1.	Download FreeRTOS from FreeRTOS website. I tested with FreeRTOS Kernel V10.2.1
	2. 	Create a project in Atmel Studio. I am using Atmel studio 7
	3. 	Add FreeRTOS source to Atmel studio project.
	4. 	Configure FreeRTOS Kernel according to FreeRTOSconfig file.
	5.	Implement USART driver in folder usart. This code is written with ATmega328P controller.
	6. 	Build the project and flash hex to your controller.
	7.	You are up now. You could see a test message come from Main module. see main.c 
	
Usage:
	1.	Create a Console channel using ConsoleCreate function. 
	2.	You need to provide a key string and a Handler function.
	3.	Write debug message using API provided. 
	4. 	Message will appended with key and Message type.
	5. 	Handler function will called when a command starting with key is received from the serial port.
	6.	API for formatted output also added.
	7. 	You can turn on or off of output from individual key or for all.
	
Example: Create a channel adding follwing code.

	ConsoleChannel main_con;
	
	
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
	
	
	Initialize the channel.
	main_con = ConsoleCreate("MAIN", MainDebugHandler);

	Now you can send different message to Console using API provided.
	
	ConsoleInfo(main_con, "Hello! This is a test.");
	
	It will generate output like below:
	
	>MAIN[INFO]: Hello! This is a test.
	
	Above code implement echo command on channel main. You can send follwing command to channel main lik bellow
	
	main echo hello\n
	
	It will reply:
	
	>MAIN[REPLY]: hello

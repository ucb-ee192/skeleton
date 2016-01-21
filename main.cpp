/* !!! needs full licened compiler version !!!
*/
#include "mbed.h"
#include "rtos.h"
#include "MODSERIAL.h"
#include "stdint.h"

MODSERIAL serial(USBTX, USBRX);

DigitalOut led_green(LED_GREEN);
DigitalOut led_red(LED_RED);
PwmOut led_blue(LED_BLUE);



void led_fade_thread(void const *args) {
  // Note that this function doesn't terminate, which is fine since it runs in
  // a thread.
  while (1) {
    // Since the internal LED is active low, inert the duty cycle.
    led_blue.write(1 - 0);
    Thread::wait(250);
    led_blue.write(1 - 0.25);
    Thread::wait(250);
    led_blue.write(1 - 0.5);
    Thread::wait(250);
    led_blue.write(1 - 0.75);
    Thread::wait(250);
  }
}

void putchar_thread(void const *args)
{   char c= 'x';
	while(1)
	{ Thread::wait(1000);
		serial.putc(c);
	}
}

void putchar_thread1(void const *args)
{   char c= 'y'; 
	long start, end;
	while(1)
	{ Thread::wait(1001);
		serial.putc(c);
		start = SysTick->VAL; //before printf
		serial.printf("SysTick < %ld . |", start);
		end = SysTick->VAL;
		serial.printf("SysTick >  %ld . |", end);
	}
}

void led_blink_periodic(void const *args) {
  // Toggle the red LED when this function is called.
  led_red = !led_red;
}


void print_thread_status(long value)
{
		switch(value)
		{ case 0: serial.printf("Inactive "); break;
		  case 1: serial.printf("Ready "); break;
			case 2: serial.printf("Running "); break;
			case 3: serial.printf("WaitingDelay "); break;
			case 4: serial.printf("WaitingInterval "); break;
			default: serial.putc('?');
		
		}
}




int main() {
	long value1;
	long value2;
	uint32_t ticks;
	uint32_t returnCode;
	
	returnCode = SysTick_Config ((uint32_t)(SystemCoreClock / 10000)); //100us granularity
	
  // It's always nice to know what version is deployed.
  serial.printf("\n Built " __DATE__ " " __TIME__ "\r\n");
  
  // Quick blink on startup.
  led_green = 0;  // Note that the internal LED is active low.
  wait(0.25);
  led_green = 1;
  wait(0.25);
  
  // Mandatory "Hello, world!".
  serial.printf("Hello, world!\r\n");
	ticks = SysTick->VAL;
  serial.printf("starting time %ld |", ticks);
  // Start a thread running led_fade_thread().
  Thread ledFadeThread(led_fade_thread);
  
	// Start a thread running character printing
	Thread putcharThread(putchar_thread);
	Thread putcharThread1(putchar_thread1);
	
  // Set a timer to periodically call led_blink_periodic().
  RtosTimer ledBlinkTimer(led_blink_periodic);
  ledBlinkTimer.start(1000);
  serial.printf("Hello, again!\r\n");
	
	// look at status of a thread
	
	value1 = putcharThread.get_state();
	value2 = putcharThread1.get_state();
	//serial.printf("putcharThread: %d |", (long) value1);
	//serial.printf("putcharThread1 %d", (long) value2);
	
	 serial.printf("putcharThread: ");
	 print_thread_status(value1);
	 serial.printf("putcharThread1: ");
	 print_thread_status(value2);
	
	
	//serial.printf("get_state() %d", (long) value);
  // Work is done in the threads, so main() can sleep.
  Thread::wait(osWaitForever);
}

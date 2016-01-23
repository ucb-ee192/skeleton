/* !!! needs full licened compiler version !!!
*/
// see https://developer.mbed.org/handbook/RTOS  for RTOS documentation 

#include "mbed.h"
#include "rtos.h"
#include "rt_Time.h"
#include "MODSERIAL.h"
#include "stdint.h"
// using rtos builtin timer
uint32_t systime = 0; // systime updated every 100 us for timing purposes
uint32_t systime1 = 0; // systime updated every 100 us for timing purposes

extern int printfNB(const char *format, ...);
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
{ char c= 'x';
	long start, end;
	while(1)
	{
		Thread::wait(20000); // should be 2.0 sec
  	serial.putc(c);
		start = (long) os_time;
		serial.printf("putchar_thread: bbbbbbbbbbbb \r\n");
		end = (long) os_time;
		serial.printf("systime = %ld, start = %ld, end = %ld, elapsed time %ld .\r\n", 
					systime, start, end, end-start);
	}
}

void putchar_thread1(void const *args)
{   char c= 'y'; 
	long start, end;
	while(1)
	{ Thread::wait(20000); // should be 2.0 sec
    serial.putc(c);
		start = (long) os_time;
		serial.printf(" putchar_thread1: aaaaaaaaaaaaa \r\n");
		end = (long) os_time;
		serial.printf("systime = %ld, start = %ld, end = %ld, elapsed time %ld .\r\n", 
					systime, start, end, end-start);
	}
}


// print thread - non blocking method for handling printing
void print_thread(void const *args)
{ Thread::wait(1000); // should be 0.1 sec}
}
	
	
void print_thread_status(long value)
{		switch(value)
		{ case 0: serial.printf("Inactive "); break;
		  case 1: serial.printf("Ready "); break;
			case 2: serial.printf("Running "); break;
			case 3: serial.printf("WaitingDelay "); break;
			case 4: serial.printf("WaitingInterval "); break;
			default: serial.putc('?');
		}
}

/// using RTOS built in timer:

void led_blink_periodic(void const *args) {
  // Toggle the red LED when this function is called.
  led_red = !led_red;
}


// systime is not needed since internal ostime runs at same 100 us rate.
void RealTime(void const *args)
{ uint32_t timeval; // read just once in case updated by another interrupt
	systime++;
	timeval=os_time; // assume atomic?
	if ((timeval % 100) == 0) 
	{ serial.putc('.'); }    // every 10 ms- lets see if blocks
	if((timeval % 10000) == 0) 
		{ //serial.putc('!');
			serial.printf("\r\n RealTime: systime - os_time=%ld, systime1 - os_time %ld",
					systime-timeval, systime1 - timeval);
		 }    // every 1000 ms- lets see if delays
}

// systime is not needed since internal ostime runs at same 100 us rate.
void RealTime1(void const *args)
{ systime1++;
//	if ((os_time % 100) == 0) 
//	{ serial.putc('.'); }    // every 10 ms- lets see if blocks
//	if((os_time % 10000) == 0) 
//		{ serial.printf("RealTime1: systime1 - os_time=%ld .", systime1-os_time);
//		 }    // every 1000 ms- lets see if delays
}



int main() {
	long value1;
	long value2;
	uint32_t returnCode;
	
  // It's always nice to know what version is deployed.
  serial.printf("\n\r Built " __DATE__ " " __TIME__ "\r\n");
  returnCode = SysTick_Config ((uint32_t)(SystemCoreClock / 10000)); //100us granularity
	serial.printf("SysTick: returnCode: 0x%lx ", returnCode);
	serial.printf("SysTick: Load %lx ", SysTick->LOAD);
	serial.printf("starting time %ld |", os_time);
	
  // Quick blink on startup.
  led_green = 0;  // Note that the internal LED is active low.
  wait(0.25);
  led_green = 1;
  wait(0.25);
  
  // Mandatory "Hello, world!".
  serial.printf("\r\n Hello, world!\r\n");
	
  // start a thread to handle printing without blocking
	Thread printThread(print_thread);
 
 
  // Start a thread running led_fade_thread().
  Thread ledFadeThread(led_fade_thread);
  
	// Start a thread running character printing
	Thread putcharThread(putchar_thread);
	Thread putcharThread1(putchar_thread1);
	
  // Set a timer to periodically call led_blink_periodic().
  RtosTimer ledBlinkTimer(led_blink_periodic);
  ledBlinkTimer.start(1000);
	
	// RtosTimer realTimeTimer(RealTime,osTimerPeriodic, NULL);
	RtosTimer realTimeTimer(RealTime);
	realTimeTimer.start(2); // .2 millisecond timing
	// RtosTimer realTimeTimer(RealTime,osTimerPeriodic, NULL);
	RtosTimer realTimeTimer1(RealTime1);
	realTimeTimer1.start(2); // 0.2 millisecond timing
	
  serial.printf("Hello, again!\r\n");
	
	// look at status of a thread
	
	value1 = putcharThread.get_state();
	value2 = putcharThread1.get_state();
		
	 serial.printf("putcharThread: ");
	 print_thread_status(value1);
	 serial.printf("putcharThread1: ");
	 print_thread_status(value2);
	
	
	//serial.printf("get_state() %d", (long) value);
  // Work is done in the threads, so main() can sleep.
  Thread::wait(osWaitForever);
}

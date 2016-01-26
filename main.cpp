/* !!! needs full licened compiler version !!!
*/
// see https://developer.mbed.org/handbook/RTOS  for RTOS documentation 


/*********************************
printfNB: if using printf-stdarg, this is a minimal printf for high speed, and compact space.
types recognized: %d, %s, %x, %X

*****************************/


#include "mbed.h"
#include "rtos.h"
#include "rt_Time.h"
#include "MODSERIAL.h"
#include "stdint.h"
// using rtos builtin timer
uint32_t systime = 0; // systime updated every 100 us for timing purposes
uint32_t systime1 = 0; // systime updated every 100 us for timing purposes

extern "C" { extern int printfNB(const char *format, ...); }
extern "C" {extern int putcharNB(int);}

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

void test_thread(void const *args)
{ char c= 'x';
	long start, end;
	while(1)
	{
		Thread::wait(20000); // should be 2.0 sec
  	putcharNB(c); // non blocking putchar
		start = (long) os_time;
		serial.printf("\r\n test_thread: bbbbbbbbbbbb start=%ld\r\n",start);
		end = (long) os_time;
		serial.printf("test_thread: serial.printf delay= %ld\r\n", end-start);
		// now check delay using sprintf
		start = (long) os_time;
		printfNB("test_thread: printfNB: ccccccccccccc \r\n");
		end = (long) os_time;
		serial.printf("test_thread: printfNB delay= %ld, end =%ld\r\n", end-start,end);
	}
}

void test_thread1(void const *args)
{   char c= 'y'; 
	long start, end;
	while(1)
	{ Thread::wait(20000); // should be 2.0 sec
    serial.putc(c);
		start = (long) os_time;
		serial.printf("\r\n test_thread1: aaaaaaaaaaaaa start=%ld\r\n", start);
		end = (long) os_time;
		serial.printf("test_thread1: serial.printf delay= %ld\r\n", end-start);
	}
}


// print thread - non blocking method for handling printing
#define PRNBUFSZ 0x100
extern char printbuffer[PRNBUFSZ];
extern int prnbuf_count;   /* number of characters in buffer */
extern int prnbuf_pos;   /* location to store characters */

void print_thread(void const *args)
{ char c; int index;
	while(1)
	{	Thread::wait(1000); // should be 0.1 sec}
	// process print buffer in thread, so won't block
		while (prnbuf_count > 0)  /* there are characters to print */
		{  index = prnbuf_pos - prnbuf_count;
       if(index < 0) index = index +PRNBUFSZ;  /* wrap around */
       c = printbuffer[index];
		   serial.putc(c); // print one character, blocking or not
       prnbuf_count--;
		}
	}
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
{ long timeval, diff1, diff2; // read just once in case updated by another interrupt
	systime++;
	timeval= (long) os_time; // assume atomic?
	if ((timeval % 100) == 0) 
	{ putcharNB('.'); }    // don't use regular printf in interrupt routine since mutex...
	if((systime % 10000) == 0) 
		{ diff1 = (systime<<1) - timeval;  // main clock at 100 us
			diff2 = (systime1<<1) - timeval; // user timer at 200 us
				printfNB("\r\n RealTime: ostime=%ld systime - os_time=%ld, systime1 - os_time= %ld \n",
					timeval,diff1, diff2);
		 }    // every 1000 ms- lets see if delays
}

// systime is not needed since internal ostime runs at same 100 us rate.
void RealTime1(void const *args)
{ long timeval;
	systime1++;
	timeval= (long) os_time; // assume atomic?
	if((systime1 % 10000) == 0) 
		{ printfNB("\r\n RealTime1: ostime=%ld systime1 %ld\n",
					timeval,systime1);
		 }    // every 1000 ms- lets see if delays

}



int main() {
	long value1;
	long value2;
	uint32_t returnCode;
	
  // It's always nice to know what version is deployed.
  serial.printf("\n\r Built " __DATE__ " " __TIME__ "\r\n");
  returnCode = SysTick_Config ((uint32_t)(SystemCoreClock / 10000)); //100us granularity
	serial.printf("SysTick: returnCode: 0x%lx ", returnCode);
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
  
	// Start some threads running character printing
	Thread testThread(test_thread);
	Thread testThread1(test_thread1);
	
  // Set a timer to periodically call led_blink_periodic().
  RtosTimer ledBlinkTimer(led_blink_periodic);
  ledBlinkTimer.start(1000);
	
	// start real time clock at 200 us NOTE only need 1 main RT ``super loop''
	/*** CAUTION - this gets BLOD at 200 us ***/
	
	RtosTimer realTimeTimer(RealTime);
	realTimeTimer.start(2); // .2 millisecond timing
	// second rtos with no calculations
	RtosTimer realTimeTimer1(RealTime1);
	realTimeTimer1.start(2); // 0.2 millisecond timing
	
  serial.printf("Hello, again!\r\n");
	serial.printf("sizeof(int) = %d, sizeof(long) = %d\n\r", sizeof(int), sizeof(long));
	
	// turn off interrupt to allow only one process to print
	__disable_irq();
	printfNB("\n using printNFB: int %d, long %ld \n", (int) 0x8000, (long) 0x7fffffff);
	__enable_irq();
	
	// look at status of a thread
	
	value1 = testThread.get_state();
	value2 = testThread1.get_state();
		
	 serial.printf("testThread: ");
	 print_thread_status(value1);
	 serial.printf("testThread1: ");
	 print_thread_status(value2);
	
	
	//serial.printf("get_state() %d", (long) value);
  // Work is done in the threads, so main() can sleep.
  Thread::wait(osWaitForever);
}

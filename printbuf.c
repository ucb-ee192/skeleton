/***********************************************************************/
/*                                                                     */
/*  printbuf.c:  Low Level print ring buffer  Routines                               */
/*                                                                     */
/***********************************************************************/
#include <stdio.h>
#include <stdarg.h>

#define CR     0x0D
#define TEMT 0x40	/* serial COMTX empty */
extern int write (int file, char * ptr, int len);
int putchar_buf(int);
int putcharNB(int);


/* Background non-blocking print using ring buffer */
#define PRNBUFSZ 0x400
char printbuffer[PRNBUFSZ];
int prnbuf_count = 0;   /* number of characters in buffer */
int prnbuf_pos = 0;   /* location to store characters */


int printfNB(const char *format, ...)
{  char buffer[128]; int i;
	char c;
	int val;
  va_list args;     
  va_start( args, format );
  val = vsprintf(buffer, format, args );   // print to string using variable arguments
	for(i = 0; i < 128 && buffer[i]!= '\0'; i++)    // copy string to print buffer
	{ putcharNB((int)buffer[i]); }
	return(val);
}


// overload putchar to use non-blocking print to ring buffer instead
int putcharNB(int c)  {
  	if (c == '\n') putchar_buf(CR);
	putchar_buf(c);
	return(c);	} 

	
/* routine to print using print ring buffer */
/* does not block - allows over runs */
int putchar_buf(int c)
{
  if(prnbuf_count >= PRNBUFSZ) return(0);   /* no room - drop character */
  printbuffer[prnbuf_pos] = c;
  prnbuf_pos++;
  prnbuf_count++;   /* need to make uninterruptable? */
  if(prnbuf_pos > (PRNBUFSZ-1))  prnbuf_pos = 0;   /* wrap index to beginning of
 buffer */
  return(c);
}




int write (int file, char * ptr, int len) {
  int i;

  for (i = 0; i < len; i++) putcharNB(*ptr++);
  return len;
}


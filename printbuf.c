/***********************************************************************/
/*                                                                     */
/*  SERIAL.C:  Low Level Serial Routines                               */
/*                                                                     */
/***********************************************************************/


#define CR     0x0D
#define TEMT 0x40	/* serial COMTX empty */
extern int write (int file, char * ptr, int len);
int putchar_buf(int);


/* Background non-blocking print using ring buffer */
#define PRNBUFSZ 0x100
char printbuffer[PRNBUFSZ];
int prnbuf_count = 0;   /* number of characters in buffer */
int prnbuf_pos = 0;   /* location to store characters */

/* called from interrupt routine */
void print_function(void)
{
  char c; int index;
  if (prnbuf_count > 0)  /* there are characters to print */
  {  index = prnbuf_pos - prnbuf_count;
     if(index < 0) index = index +PRNBUFSZ;  /* wrap around */
     c = printbuffer[index];
/*	 if (c == '\n') c = CR;		/* output CR */
		
     if(COMSTA0 & TEMT)  /* check in COMTX is empty (ready) */
	 {	 COMTX = c;
		  prnbuf_count--;
	  }
  }
}

  /* routine to print using print ring buffer */
/* does not block - allows over runs */
int putchar(int c)  {
  	if (c == '\n') putchar_buf(CR);
	putchar_buf(c);
	return(c);	} 

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



int putchar_block(int ch)  {                   /* Write character to Serial Port  */

	if (ch == '\n')  {
    	while(!(0x020==(COMSTA0 & 0x020)))
    	{}
		COMTX = CR;							/* output CR */
		}
    while(!(0x020==(COMSTA0 & 0x020)))
    {}
 
 	return (COMTX = ch);
}


int getchar (void)  {                      /* Read character from Serial Port */

   	while(!(0x01==(COMSTA0 & 0x01)))
   	{}
  	return (COMRX);
}

int write (int file, char * ptr, int len) {
  int i;

  for (i = 0; i < len; i++) putchar_block(*ptr++);
  return len;
}


/***********************************************************************/
/*                                                                     */
/*  printbuf.c:  Low Level print ring buffer  Routines                               */
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


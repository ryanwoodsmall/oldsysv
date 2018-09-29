/*	@(#)lib.h	1.2	*/
/*
 *  Library call external declarations
 */

extern STRING environ;
extern int    errno;

extern char *ctime();
extern int  exect();
extern int  fprintf();
extern void longjmp();
extern void perror();
extern int  setjmp();
extern int  sprintf();

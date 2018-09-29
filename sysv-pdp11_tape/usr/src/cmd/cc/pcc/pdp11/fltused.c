/*	@(#)fltused.c	1.2	*/
/* define the symbol fltused to bring in shorter printf from library */
static void fltused(){
asm(".globl fltused");
asm("fltused:");}

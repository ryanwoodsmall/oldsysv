static char ID[] = "@(#) code.c: 1.4 11/5/82";
#include <stdio.h>
#include "systems.h"
#include "symbols.h"
#include "codeout.h"
#include "gendefs.h"

short bitpos = 0;
extern symbol symtab[];
extern short Oflag;
extern long
	newdot;		/* up-to-date value of "." */
extern symbol
	*dot;
extern unsigned short line;
   
codebuf textbuf[TBUFSIZE],
	databuf[TBUFSIZE];

static short dbufrot=0;
static short tbufrot=0;
   

extern FILE *fdtext,
	*fddata;

generate (nbits,action,value,sym)
		BYTE nbits;
		unsigned short action;
		long value;
		symbol *sym;
{
	FILE  *fd;
	register codebuf *bufptr1,
		*bufptr2;
	register short *count;

	switch (dot->styp){
		case TXT:{
			fd = fdtext;
			bufptr2 = textbuf;
			bufptr1 =  &textbuf[tbufrot++];
			count = &tbufrot;
			break;
		}
		case BSS: {
			if (nbits != 0) {
				yyerror("Attempt to initialize bss");
				return;
			}
			/* otherwise, can write on any file, so treat as DAT */
		}
		case DAT:{
			fd=fddata;
			bufptr2 = databuf;
			bufptr1 = &databuf[dbufrot++];
			count = &dbufrot;
			break;
		}
	}
	bufptr1->caction = action;
	bufptr1->cnbits = nbits;
	bufptr1->cvalue = value;
#if VAX
	bufptr1->cindex = (int)sym;
#else
	bufptr1->cindex = sym==NULL ? 0 : 1+(sym-symtab);
#endif
	if (*count == TBUFSIZE) {
		fwrite((char *)bufptr2, sizeof(*bufptr2), TBUFSIZE, fd);
		(*count)= 0;
	}
#if DEBUG
 		if (Oflag) {
 			if (bitpos == 0)
 				printf("(%d:%6lx)	",dot->styp,newdot);
 			else
 				printf("		");
 			printf("%hd	%d	%hd	%.13lx	%hd\n",
 				line,(short)nbits,action,value,bufptr1->cindex);
 		}
#endif
	bitpos += nbits;
	newdot += bitpos/BITSPBY;
	bitpos %= BITSPBY;
}
    
flushbuf()
{
	fwrite((char *)textbuf, sizeof(*textbuf), tbufrot, fdtext);
	fwrite((char *)databuf, sizeof(*databuf), dbufrot, fddata);
	tbufrot = 0;
	dbufrot = 0;

}
   
long dottxt = 0L,
	dotdat = 0L,
	dotbss = 0L;

cgsect(newtype)
	register short newtype;

{
	switch(dot->styp){
		case TXT:{
			dottxt = newdot;
			break;
		}
		case DAT:{
			dotdat = newdot;
			break;
		}
		case BSS:{
			dotbss = newdot;
			break;
		}
	}

	dot->styp = newtype;
	switch(newtype){
		case TXT:{
			newdot = dottxt;
			break;
		}
		case DAT:{
			newdot = dotdat;
			break;
		}
		case BSS:{
			newdot = dotbss;
			break;
		}
	}
	dot->value = newdot;
}

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:util.c	1.11"
/*
 * This file contains code for utilities used by more than one crash function.
 */

#include "sys/param.h"
#include "a.out.h"
#include "signal.h"
#include "stdio.h"
#include "setjmp.h"
#include "ctype.h"
#include "sys/var.h"
#include "sys/types.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "crash.h"

extern jmp_buf jmp;
extern struct syment *Curproc, *Proc, *V;
extern long vtop();
extern int opipe;
extern FILE *rp;
void exit();
void free();

/* close pipe and reset file pointers */
int
resetfp()
{
	extern long pipesig;

	if(opipe == 1) {
		pclose(fp);
		signal(SIGPIPE,pipesig);
		opipe = 0;
		fp = stdout;
	}
	else {
		if((fp != stdout) && (fp != rp) && (fp != NULL)) {
			fclose(fp);
			fp = stdout;
		}
	}
}

/* signal handling */
void 
sigint()
{
	extern int *stk_bptr;

	signal(SIGINT, sigint);		
	if(stk_bptr)
		free((char *)stk_bptr);
	fflush(fp);
	resetfp();
	fprintf(fp,"\n");
	longjmp(jmp, 0);
}

/* used in init.c to exit program */
/*VARARGS1*/
int
fatal(string,arg1,arg2,arg3)
char *string;
int arg1,arg2,arg3;
{
	fprintf(stderr,"crash:  ");
	fprintf(stderr,string,arg1,arg2,arg3);
	exit(1);
}

/* string to hexadecimal long conversion */
long
hextol(s)
char *s;
{
	int	i,j;
		
	for(j = 0; s[j] != '\0'; j++)
		if((s[j] < '0' || s[j] > '9') && (s[j] < 'a' || s[j] > 'f')
			&& (s[j] < 'A' || s[j] > 'F'))
			break ;
	if(s[j] != '\0' || sscanf(s, "%x", &i) != 1) {
		prerrmes("%c is not a digit or letter a - f\n",s[j]);
		return(-1);
	}
	return(i);
}


/* string to decimal long conversion */
long
stol(s)
char *s;
{
	int	i,j;
		
	for(j = 0; s[j] != '\0'; j++)
		if((s[j] < '0' || s[j] > '9'))
			break ;
	if(s[j] != '\0' || sscanf(s, "%d", &i) != 1) {
		prerrmes("%c is not a digit 0 - 9\n",s[j]);
		return(-1);
	}
	return(i);
}


/* string to octal long conversion */
long
octol(s)
char *s;
{
	int	i,j;
		
	for(j = 0; s[j] != '\0'; j++)
		if((s[j] < '0' || s[j] > '7')) 
			break ;
	if(s[j] != '\0' || sscanf(s, "%o", &i) != 1) {
		prerrmes("%c is not a digit 0 - 7\n",s[j]);
		return(-1);
	}
	return(i);
}


/* string to binary long conversion */
long
btol(s)
char *s;
{
	int	i,j;
		
	i = 0;
	for(j = 0; s[j] != '\0'; j++)
		switch(s[j]) {
			case '0' :	i = i << 1;
					break;
			case '1' :	i = (i << 1) + 1;
					break;
			default  :	prerrmes("%c is not a 0 or 1\n",s[j]);
					return(-1);
		}
	return(i);
}

/* string to number conversion */
long
strcon(string,format)
char *string;
char format;
{
	char *s;

	s = string;
	if(*s == '0') {
		if(strlen(s) == 1)
			return(0); 
		switch(*++s) {
			case 'x' :	format = 'h';
					s++;
					break;
			case 'b' :	format = 'b';
					s++;
					break;
			case 'd' :	format = 'd';
					s++;
					break;
			default  :	format = 'o';
		}
	}
	if(!format)
		format = 'd';
	switch(format) {
		case 'h' :	return(hextol(s));
		case 'd' :	return(stol(s));
		case 'o' :	return(octol(s));
		case 'b' :	return(btol(s));
		default  :	return(-1);
	}
}


/* lseek */
int
seekmem(addr,mode,proc)
long addr;
int mode,proc;
{
	long paddr;
	extern long lseek();

	if(mode)
		paddr = vtop(addr,proc);
	else paddr = addr;
	if(paddr == -1)
		error("%x is an invalid address\n",addr);
	if(lseek(mem,paddr,0) == -1)
		error("seek error on address %x\n",addr);
}

/* lseek and read */
int
readmem(addr,mode,proc,buffer,size,name)
long addr;
int mode,proc;
char *buffer;
unsigned size;
char *name;
{
	seekmem(addr,mode,proc);
	if(read(mem,buffer,size) != size)
		error("read error on %s\n",name);
}

/* lseek and read into given buffer */
int
readbuf(addr,offset,phys,proc,buffer,size,name)
long addr,offset;
char *buffer;
unsigned size;
int phys,proc;
char *name;
{
	if((phys || !Virtmode) && (addr != -1))
		readmem(addr&~MAINSTORE,0,proc,buffer,size,name);
	else if(addr != -1)
		readmem(addr,1,proc,buffer,size,name);
		else readmem(offset,1,proc,buffer,size,name);
}


/* error handling */
/*VARARGS1*/
int
error(string,arg1,arg2,arg3)
char *string;
int arg1,arg2,arg3;
{

	if(rp) 
		fprintf(stdout,string,arg1,arg2,arg3);
	fprintf(fp,string,arg1,arg2,arg3);
	fflush(fp);
	resetfp();
	longjmp(jmp,0);
}


/* print error message */
/*VARARGS1*/
int
prerrmes(string,arg1,arg2,arg3)
char *string;
int arg1,arg2,arg3;
{

	if((rp && (rp != stdout)) || (fp != stdout)) 
		fprintf(stdout,string,arg1,arg2,arg3);
	fprintf(fp,string,arg1,arg2,arg3);
	fflush(fp);
}


/* simple arithmetic expression evaluation ( +  - & | * /) */
long
eval(string)
char *string;
{
	int j,i;
	char rand1[ARGLEN];
	char rand2[ARGLEN];
	char *op;
	long addr1,addr2;
	struct syment *sp;
	extern char *strpbrk();

	if(string[strlen(string)-1] != ')') {
		prerrmes("(%s is not a well-formed expression\n",string);
		return(-1);
	}
	if(!(op = strpbrk(string,"+-&|*/"))) {
		prerrmes("(%s is not an expression\n",string);
		return(-1);
	}
	for(j=0,i=0; string[j] != *op; j++,i++) {
		if(string[j] == ' ')
			--i;
		else rand1[i] = string[j];
	}
	rand1[i] = '\0';
	j++;
	for(i = 0; string[j] != ')'; j++,i++) {
		if(string[j] == ' ')
			--i;
		else rand2[i] = string[j];
	}
	rand2[i] = '\0';
	if(!strlen(rand2) || strpbrk(rand2,"+-&|*/")) {
		prerrmes("(%s is not a well-formed expression\n",string);
		return(-1);
	}
	if(sp = symsrch(rand1))
		addr1 = sp->n_value;
	else if((addr1 = strcon(rand1,NULL)) == -1)
		return(-1);
	if(sp = symsrch(rand2))
		addr2 = sp->n_value;
	else if((addr2 = strcon(rand2,NULL)) == -1)
		return(-1);
	switch(*op) {
		case '+' : return(addr1 + addr2);
		case '-' : if((addr1 - addr2) < 0) {
				prerrmes("%x is not an valid address\n",addr1-addr2);
				return(-1);
			   }
			   return(addr1 - addr2);
		case '&' : return(addr1 & addr2);
		case '|' : return(addr1 | addr2);
		case '*' : return(addr1 * addr2);
		case '/' : if(addr2 == 0) {
				prerrmes("cannot divide by 0\n");
				return(-1);
			   }
			   return(addr1 / addr2);
	}
	return(-1);
}
		
/* get current process slot number */
int
getcurproc()
{
	int curproc;

	readmem((long)Curproc->n_value,1,-1,(char *)&curproc,
		sizeof curproc,"current process");
	return((curproc - Proc->n_value) / sizeof(struct proc));
}


/* determine valid process table entry */
int
procntry(slot,prbuf)
int slot;
struct  proc *prbuf;
{
	if(slot == -1) 
		slot = getcurproc();

	if((slot > vbuf.v_proc) || (slot < 0))
		error("%d out of range\n",slot);

	readmem((long)(Proc->n_value+slot*sizeof(struct proc)),1,-1,
		(char *)prbuf, sizeof(struct proc),"process table");
	if(!prbuf->p_stat)
		error(" %d is not a valid process\n",slot);
}

/* argument processing from **args */
long
getargs(max,arg1,arg2)
int max;
long *arg1,*arg2;
{	
	struct syment *sp;
	long slot;

	/* range */
	if(strpbrk(args[optind],"-")) {
		range(max,arg1,arg2);
		return;
	}
	/* expression */
	if(*args[optind] == '(') {
		*arg1 = (eval(++args[optind]));
		return;
	}
	/* symbol */
	if((sp = symsrch(args[optind])) != NULL) {
		*arg1 = (sp->n_value);
		return;
	}
	if(isasymbol(args[optind])) {
		prerrmes("%s not found in symbol table\n",args[optind]);
		*arg1 = -1;
		return;
	}
	/* slot number */
	if((slot = strcon(args[optind],'h')) == -1) {
		*arg1 = -1;
		return;
	}
	if(slot < MAINSTORE) {
		if((slot = strcon(args[optind],'d')) == -1) {
			*arg1 = -1;
			return;
		}
		if((slot < max) && (slot >= 0)) {
			*arg1 = slot;
			return;
		}
		else {
			prerrmes("%d is out of range\n",slot);
			*arg1 = -1;
			return;
		}
	}
	/* address */
	*arg1 = slot;
} 

/* get slot number in table from address */
int
getslot(addr,base,size,phys)
long addr,base;
int size,phys;
{
	long pbase;
	
	if(phys || !Virtmode) {
		pbase = vtop(base,-1);
		if(pbase == -1)
			error("%x is an invalid address\n",base);
		return(((addr&~MAINSTORE) - pbase) / size);
	}
	else return((addr - base) / size);
}


/* file redirection */
int
redirect()
{
	int i;
	FILE *ofp;

	ofp = fp;
	if(opipe == 1) {
		fprintf(stdout,"file redirection (-w) option ignored\n");
		return;
	}
	if(fp = fopen(optarg,"a")) {
		fprintf(fp,"\n> ");
		for(i=0;i<argcnt;i++)
			fprintf(fp,"%s ",args[i]);
		fprintf(fp,"\n");
	}
	else {
		fp = ofp;
		error("unable to open %s\n",optarg);
	}
}


/*
 * putch() recognizes escape sequences as well as characters and prints the
 * character or equivalent action of the sequence.
 */
int
putch(c)
char  c;
{
	c &= 0377;
	if(c < 040 || c > 0176) {
		fprintf(fp,"\\");
		switch(c) {
		case '\0': c = '0'; break;
		case '\t': c = 't'; break;
		case '\n': c = 'n'; break;
		case '\r': c = 'r'; break;
		case '\b': c = 'b'; break;
		default:   c = '?'; break;
		}
	}
	else fprintf(fp," ");
	fprintf(fp,"%c ",c);
}

/* sets process to input argument */
int
setproc()
{
	int slot;

	if((slot = strcon(optarg,'d')) == -1)
		error("\n");
	if((slot > vbuf.v_proc) || (slot < 0))
		error("%d out of range\n",slot);
	return(slot);
}

/* check to see if string is a symbol or a hexadecimal number */
int
isasymbol(string)
char *string;
{
	int i;

	for(i = strlen(string); i > 0; i--)
		if(!isxdigit(*string++))
			return(1);
	return(0);
}


/* convert a string into a range of slot numbers */
int
range(max,begin,end)
int max;
long *begin,*end;
{
	int i,j,len,pos;
	char string[ARGLEN];
	char temp1[ARGLEN];
	char temp2[ARGLEN];

	strcpy(string,args[optind]);
	len = strlen(string);
	if((*string == '-') || (string[len-1] == '-')){
		fprintf(fp,"%s is an invalid range\n",string);
		*begin = -1;
		return;
	}
	pos = strcspn(string,"-");
	for(i = 0; i < pos; i++)
		temp1[i] = string[i];
	temp1[i] = '\0';
	for(j = 0, i = pos+1; i < len; j++,i++)
		temp2[j] = string[i];
	temp2[j] = '\0';
	if((*begin = (int)stol(temp1)) == -1)
		return;
	if((*end = (int)stol(temp2)) == -1) {
		*begin = -1;
		return;
	}
	if(*begin > *end) {
		fprintf(fp,"%d-%d is an invalid range\n",*begin,*end);
		*begin = -1;
		return;
	}
	if(*end >= max) 
		*end = max - 1;
}

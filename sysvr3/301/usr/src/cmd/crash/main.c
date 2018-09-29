/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:main.c	1.18"
/*
 * This file contains code for the crash functions:  ?, help, redirect,
 * and quit, as well as the command interpreter.
 */

#include "sys/param.h"
#include "a.out.h"
#include "stdio.h"
#include "signal.h"
#include "sys/types.h"
#include "sys/fs/s5dir.h"
#include "sys/var.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "setjmp.h"
#include "crash.h"

#define NARGS 25		/* number of arguments to one function */
#define LINESIZE 256		/* size of function input line */

int mem;			/* file descriptor for dump file */
char *namelist = "/unix";
char *dumpfile = "/dev/mem";
struct user *ubp;			/* pointer to ublock buffer */
FILE *fp;				/* output file pointer */
FILE *rp;				/* redirect file pointer */
int opipe = 0;				/* open pipe flag */
struct var vbuf;			/* var structure buffer */
int Procslot;				/* current process slot */
int Virtmode = 1;			/* virtual or physical mode flag*/
long pipesig;				/* pipe signal catch */

struct syment *File,*Inode,*Mount,*Proc,*V,*Panic,
	*Curproc,*Streams;		/* namelist symbol pointers */

short N_TEXT;		/* used in symtab.c */
short N_DATA;		/* used in symtab.c */
short N_BSS;		/* used in symtab.c */
int active;	/* Flag set if crash is examining an active system */
jmp_buf	jmp,syn;	/* labels for jump */
void exit();


/* function calls */
extern int getadv(),getbufhdr(),getbuffer(),getcallout(),getdblock(),getdis();
	getgdp(),getinode(),getkfp(),getlcks(),getpfdat(),getdblk(),
	getmap(),getmess(),getmount(),getnm(),getnvram(),getod(),getpcb(),
	getproc(),getqrun(),getqueue(),getquit(),getrcvd(),getpdt(),
	getregion(),getmmu(),getsdt(),getsndd(),getsrams(),getsrmount(),
	getstack(),getstat(),getstream(),getstrstat(),gettrace(),getsymbol(),
	gettty(),getuser(),getvar(),getvtop(),getfuncs(),getbase(),gethelp(),
	getsearch(),getdbfree(),getmbfree(),getsearch(),getfile(),getdefproc(),
	getmode(),getredirect(),getmajor(),getsize(),getfindslot(),
	getfindaddr(),getfs(),getlinkblk(),getdballoc();

/* function definition */
struct func {
	char *name;
	char *syntax;
	int (*call)();
	char *description;
};

/* function table */
/* entries with NA description fields should be removed in 3.1 */
struct func functab[] = {
	"adv","[-e] [-p] [-wfilename] [tbl_entry[s]]",
		getadv,"advertise table",
	"b"," ",getbuffer,"(buffer)",
	"base","[-wfilename] number[s]",
		getbase,"base conversions",
	"buf"," ",getbufhdr,"(bufhdr)",
	"buffer","[-wfilename] [-b|-c|-d|-x|-o|-i|-r] (bufferslot |[-p] st_addr)",
		getbuffer,"buffer data",
	"bufhdr","[-f] [-p] [-wfilename] [tbl_entry[s]]",
		getbufhdr,"buffer headers",
	"c"," ",getcallout,"(callout)",
	"callout","[-wfilename]",
		getcallout,"callout table",
	"dballoc","[-wfilename] [class[es]]",
		getdballoc,"dballoc table",
	"dbfree","[-wfilename] [class[es]]",
		getdbfree,"free data block headers",
	"dblock","[-e] [-p] [-wfilename] ([-c class[es] | tbl_entry[s]])",
		getdblk,"allocated stream data block headers",
	"defproc","[-wfilename] [-c | slot]",
		getdefproc,"set default process slot",
	"dis","[-wfilename] [-a] st_addr [count]",
		getdis,"disassembler",
	"ds","[-wfilename] virtual_address[es]",
		getsymbol,"data address namelist search",
	"f"," ",getfile,"(file)",
	"file","[-e] [-p] [-wfilename] [tbl_entry[s]]",
		getfile,"file table",
	"findaddr","[-wfilename] table slot",
		getfindaddr,"find address for given table and slot",
	"findslot","[-wfilename] virtual_address[es]",
		getfindslot,"find table and slot number for given address",
	"fs","[-p] [-wfilename] [tbl_entry[s]]",
		getfs,"file system information table",
	"gdp","[-e] [-f] [-p] [-wfilename] [tbl_entry[s]]",
		getgdp,"gdp structure",
	"help","[-wfilename] function[s]",
		gethelp,"help function",
	"i"," ",getinode,"(inode)",
	"inode","[-e] [-f] [-p] [-wfilename] [tbl_entry[s]]",
		getinode,"inode table",
	"kfp","[-wfilename] [-sprocess] [-r | value]",
		getkfp,"frame pointer for start of stack trace",
	"l"," ",getlcks,"(lck)",
	"lck","[-e] [-p] [-wfilename] [tbl_entry[s]]",
		getlcks,"record lock tables",
	"linkblk","[-e] [-p] [-wfilename] [tbl_entry[s]]",
		getlinkblk,"linkblk table",
	"m"," ",getmount,"(mount)",
	"major","[-wfilename] [entry[ies]]",
		getmajor,"MAJOR table",
	"map","[-wfilename] mapname[s]",
		getmap,"map structures",
	"mbfree","[-wfilename]",
		getmbfree,"free message block headers",
	"mblock","[-e] [-p] [-wfilename] [tbl_entry[s]]",
		getmess,"allocated stream message block headers",
	"mmu","[-wfilename]",
		getmmu,"mmu registers",
	"mode","[-wfilename] [v | p]",
		getmode,"address mode",
	"mount","[-e] [-p] [-wfilename] [tbl_entry[s]]",
		getmount,"mount table",
	"nm","[-wfilename] symbol[s]",
		getnm,"name search",
	"nvram","[-wfilename] fwnvr|unxnvr|systate|errlog",
		getnvram,"non-volatile RAM",
	"od","[-p] [-wfilename] [-c|-d|-x|-o|-a|-h] [-l|-t|-b] [-sprocess] st_addr [count]",
		getod,"dump symbol values",
	"p"," ",getproc,"(proc)",
	"pcb","[-p] [-wfilename] [[-u | -k] [process] | -i st_addr]",
		getpcb,"process control block",
	"pdt","[-e] [-wfilename] [-sprocess] (section segment|[-p] st_addr [count])",
		getpdt,"page descriptor tables",
	"pfdat","[-e] [-p] [-wfilename] [tbl_entry[s]]",
		getpfdat,"pfdat structure",
	"proc","[-f] [-wfilename] [([-p] tbl_entry | #procid)[s] | -r)]",
		getproc,"process table",
	"q"," ",getquit,"(quit)",
	"qrun","[-wfilename]",
		getqrun,"list of servicable stream queues",
	"queue","[-e] [-p] [-wfilename] [tbl_entry[s]]",
		getqueue,"allocated stream queues",
	"quit"," ",
		getquit,"exit",
	"rcvd","[-e] [-f] [-p] [-wfilename] [tbl_entry[s]]",
		getrcvd,"receive descriptor",
	"rd"," ",getod,"(od)",
	"redirect","[-wfilename] [-c | filename]",
		getredirect,"output redirection",
	"region","[-e] [-f] [-p] [-wfilename] [tbl_entry[s]]",
		getregion,"region table",
	"regs"," ",getmmu,"(mmu)",
	"s"," ",getstack,"(stack)",
	"sdt","[-e] [-wfilename] [-sprocess] (section |[-p] st_addr [count])",
		getsdt,"segment descriptor table",
	"search","[-p] [-wfilename] [-mmask] [-sprocess] pattern st_addr length",
		getsearch,"memory search",
	"size","[-x] [-wfilename] structurename[s]",
		getsize,"symbol size",
	"sndd","[-e] [-p] [-wfilename] [tbl_entry[s]]",
		getsndd,"send descriptor",
	"srams","[-wfilename]",
		getsrams,"srams",
	"srmount","[-e] [-p] [-wfilename] [tbl_entry[s]]",
		getsrmount,"server mount table",
	"stack","[-p] [-wfilename] [[-u | -k] [process] | -i st_addr]",
		getstack,"stack dump",
	"stat","[-wfilename]",
		getstat,"dump statistics",
	"stream","[-e] [-f] [-p] [-wfilename] [tbl_entry[s]]",
		getstream,"allocated stream table slots",
	"strstat","[-wfilename]",
		getstrstat,"streams statistics",
	"t"," ",gettrace,"(trace)",
	"trace","[-p] [-wfilename] [[-r] [process] | -i st_addr]",
		gettrace,"kernel stack trace",
	"ts","[-wfilename] virtual_address[es]",
		getsymbol,"text address namelist search",
	"tty","[-e] [-f] [-p] [-wfilename] [-ttype [tbl_entry[s]] | st_addr]",
		gettty,"tty structures (valid types: pp, iu)",
	"u"," ",getuser,"(user)",
	"user","[-f] [-wfilename] [process]",
		getuser,"uarea",
	"v"," ",getvar,"(var)",
	"var","[-wfilename]",
		getvar,"system variables",
	"vtop","[-wfilename] [-sprocess] st_addr[s]",
		getvtop,"virtual to physical address",
	"?","[-wfilename]",
		getfuncs,"print list of available commands",
	"!cmd"," ",NULL,"escape to shell",
	"hdr"," ",getbufhdr,"NA",
	"files"," ",getfile,"NA",
	"fp"," ",getkfp,"NA",
	"r9"," ",getkfp,"NA",
	"mnt"," ",getmount,"NA",
	"dump"," ",getod,"NA",
	"ps"," ",getproc,"NA",
	"k"," ",getstack,"NA",
	"kernel"," ",getstack,"NA",
	"stk"," ",getstack,"NA",
	"ad"," ",gettty,"NA",
	"con"," ",gettty,"NA",
	"term"," ",gettty,"NA",
	"u_area"," ",getuser,"NA",
	"uarea"," ",getuser,"NA",
	"ublock"," ",getuser,"NA",
	"tunable"," ",getvar,"NA",
	"tunables"," ",getvar,"NA",
	"tune"," ",getvar,"NA",
	"calls"," ",getcallout,"NA",
	"call"," ",getcallout,"NA",
	"timeout"," ",getcallout,"NA",
	"time"," ",getcallout,"NA",
	"tout"," ",getcallout,"NA",
	"freelist"," ",getmess,"NA",
	NULL,NULL,NULL,NULL
};

char *args[NARGS];		/* argument array */
int argcnt;			/* argument count */
char outfile[100];		/* output file for redirection */
static int tabsize;		/* size of function table */

/* main program with call to functions */
main(argc,argv)
int argc;
char **argv;
{
	struct func *a,*f;
	int c,i,found;
	extern int opterr;
	int arglength;

	if(setjmp(jmp))
		exit(1);
	fp = stdout;
	strcpy(outfile,"stdout");
	optind = 1;		/* remove in 3.1 */
	opterr = 0;		/* suppress getopt error messages */

	for(tabsize = 0,f = functab; f->name; f++,tabsize++) 
		if(!strcmp(f->description,"NA"))  /* remove in 3.1 */
			break;

	while((c = getopt(argc,argv,"d:n:w:")) !=EOF) {
		switch(c) {
			case 'd' :	dumpfile = optarg;
				 	break;
			case 'n' : 	namelist = optarg;
					break;
			case 'w' : 	strncpy(outfile,optarg,ARGLEN);
					if(!(rp = fopen(outfile,"a")))
						fatal("unable to open %s\n",
							outfile);
					break;
			default  :	fatal("usage: crash [-d dumpfile] [-n namelist] [-w outfile]\n");
		}
	}
	/* backward compatible code */
	if(argv[optind]) {
		dumpfile = argv[optind++];
		if(argv[optind])
			namelist = argv[optind++];
		if(argv[optind])
			fatal("usage: crash [-d dumpfile] [-n namelist] [-w outfile]\n");
	}
	/* remove in SV3.1 */
	if(rp)
		fprintf(rp,"dumpfile = %s, namelist = %s, outfile = %s\n",dumpfile,namelist,outfile);
	fprintf(fp,"dumpfile = %s, namelist = %s, outfile = %s\n",dumpfile,namelist,outfile);
	init();

	setjmp(jmp);

	for(;;) {
		getcmd();
		if(argcnt == 0)
			continue;
		if(rp) {
			fp = rp;
			fprintf(fp,"\n> ");
			for(i = 0;i<argcnt;i++)
				fprintf(fp,"%s ",args[i]);
			fprintf(fp,"\n");
		}
		found = 0;
		for(f = functab; f->name; f++) 
			if(!strcmp(f->name,args[0])) {
				found = 1;
				break;
			}
		if(!found) {
			arglength = strlen(args[0]);
			for(f = functab; f->name; f++) {
				if(!strcmp(f->description,"NA"))
					break;     /* remove in 3.1 */
				if(!strncmp(f->name,args[0],arglength)) {
					found++;
					a = f;
				}
			}
			if(found) {
				if(found > 1)
					error("%s is an ambiguous function name\n",args[0]);
				else f = a;
			}	
		}
		if(found) {
			if(!strcmp(f->description,"NA")) /* remove in 3.1 */
				pralias(f);
			if(setjmp(syn)) {
				while(getopt(argcnt,args,"") != EOF);
				if(*f->syntax == ' ') {
					for(a = functab;a->name;a++)
						if((a->call == f->call) &&
						(*a->syntax != ' '))
							error("%s: usage: %s %s\n",f->name,f->name,a->syntax);
				}
				else error("%s: usage: %s %s\n",f->name,f->name,f->syntax);
			}
			else (*(f->call))();
		}
		else fprintf(fp,"eh?\n");
		fflush(fp);
		resetfp();
	}
}

/* returns argcnt, and args contains all arguments */
int
getcmd()
{
	char *p;
	int i;
	static char line[LINESIZE+1];
	FILE *ofp;
	
	ofp = fp;
	printf("> ");
	fflush(stdout);
	if(fgets(line,LINESIZE,stdin) == NULL)
		exit(0);
	line[LINESIZE] = '\n';
	p = line;
	while(*p == ' ' || *p == '\t') {
		p++;
	}
	if(*p == '!') {
		system(p+1);
		argcnt = 0;
	}
	else {
		for(i = 0; i < NARGS; i++) {
			if(*p == '\n') {
				*p = '\0';
				break;
			}
			while(*p == ' ' || *p == '\t')
				p++;
			args[i] = p;
			if(strlen(args[i]) == 1)
				break;
			if(*p == '!') {
				p = args[i];
				if(strlen(++args[i]) == 1)
					error("no shell command after '!'\n");
				pipesig = (long)signal(SIGPIPE,SIG_IGN);
				if((fp = popen(++p,"w")) == NULL) {
					fp = ofp;
					error("cannot open pipe\n");
				}
				if(rp != NULL)
					error("cannot use pipe with redirected output\n");
				opipe = 1;
				break;
			}
			if(*p == '(')
				while((*p != ')') && (*p != '\n'))
					p++;
			while(*p != ' ' && *p != '\n')
				p++;
			if(*p == ' ' || *p == '\t')
				*p++ = '\0';
		}
		args[i] = NULL;
		argcnt = i;
	}
}


/* get arguments for ? function */
int
getfuncs()
{
	int c;

	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	prfuncs();
}

/* print all function names in columns */
int
prfuncs()
{
	int i,j,len;
	struct func *ff;
	char tempbuf[20];

	len = (tabsize + 3) / 4;
	for(i = 0; i < len; i++) {
		ff = functab + i;
		for(j = 0; j < 4; j++) {
			if(*ff->description != '(')
				fprintf(fp,"%-15s",ff->name);
			else {
				tempbuf[0] = 0;
				strcat(tempbuf,ff->name);
				strcat(tempbuf," ");
				strcat(tempbuf,ff->description);
				fprintf(fp,"%-15s",tempbuf);
			}
			ff += len;
			if((ff - functab) >= tabsize)
				break;
		}
		fprintf(fp,"\n");
	}
	fprintf(fp,"\n");
}

/* get arguments for help function */
int
gethelp()
{
	int c;

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		do {
			prhelp(args[optind++]);
		}while(args[optind]);
	}
	else prhelp("help");
}

/* print function information */
int
prhelp(string)
char *string;
{
	int found = 0;
	struct func *ff,*a,*aa;

	for(ff=functab;ff->name;ff++) {
		if(!strcmp(ff->name,string)){
			found = 1;
			break;
		}
	}
	if(!found)
		error("%s does not match in function list\n",string);
	if(!strcmp(ff->description,"NA"))  /* remove in 3.1 */
		pralias(ff);
	if(*ff->description == '(') {
		for(a = functab;a->name != NULL;a++)
			if((a->call == ff->call) && (*a->description != '('))
					break;
		fprintf(fp,"%s %s\n",ff->name,a->syntax);
		fprintf(fp,"\ttbl_entry = slot number | address | symbol | expression | range\n");
		fprintf(fp,"\tst_addr = address | symbol | expression\n");
		fprintf(fp,"%s\n",a->description);
	}
	else {
		fprintf(fp,"%s %s\n",ff->name,ff->syntax);
		fprintf(fp,"\ttbl_entry = slot number | address | symbol | expression | range\n");
		fprintf(fp,"\tst_addr = address | symbol | expression\n");
		fprintf(fp,"%s\n",ff->description);
	}
	fprintf(fp,"alias: ");
	for(aa = functab;aa->name != NULL;aa++)
		if((aa->call == ff->call) && (strcmp(aa->name,ff->name)) &&
			strcmp(aa->description,"NA"))
				fprintf(fp,"%s ",aa->name);
	fprintf(fp,"\n");
	fprintf(fp,"\tacceptable aliases are uniquely identifiable initial substrings\n");
}

/* this function and all obsolete aliases should be removed in 3.1 */
/* print valid function names for obsolete aliases */
int
pralias(ff)
struct func *ff;
{
	struct func *a;

	fprintf(fp,"Valid calls to this function are:  ");
	for(a = functab;a->name;a++)
		if((a->call == ff->call) && (strcmp(a->name,ff->name)) &&
			(strcmp(a->description,"NA")))
				fprintf(fp,"%s ",a->name);
	error("\nThe alias %s is not supported on this processor\n",
		ff->name);
}


/* terminate crash session */
int
getquit()
{
	if(rp)
		fclose(rp);
	exit(0);
}

/* get arguments for redirect function */
int
getredirect()
{
	int c;
	int close = 0;

	optind = 1;
	while((c = getopt(argcnt,args,"w:c")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 'c' :	close = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) 
		prredirect(args[optind],close);
	else prredirect(NULL,close);
}

/* print results of redirect function */
int
prredirect(string,close)
char *string;
int close;
{
	if(close)
		if(rp) {
			fclose(rp);
			rp = NULL;
			strcpy(outfile,"stdout");
			fp = stdout;
		}
	if(string) {
		if(rp) {
			fclose(rp);
			rp = NULL;
		}
		if(!(rp = fopen(string,"a")))
			error("unable to open %s\n",string);
		fp = rp;
		strncpy(outfile,string,ARGLEN);
	}
	fprintf(fp,"outfile = %s\n",outfile);
	if(rp)
		fprintf(stdout,"outfile = %s\n",outfile);
}

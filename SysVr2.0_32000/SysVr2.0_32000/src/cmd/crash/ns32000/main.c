/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/* @(#)main.c	1.9 */
#include	"crash.h"
#include	"cmd.h"
#include	"setjmp.h"
#include	"ctype.h"

#define	CURPROC	-1	/* Use the current or last running process's data */

#define	readinto(number)	sscanf(arg, NBPW==4 ? "%x" : "%o", &number)

int		mem, kmem;
unsigned	Kfp;
char	*namelist = "/unix";
char	*dumpfile = "/dev/mem";
char	line[256], *p = line;
char	sbuf[BUFSIZ];		/* For setbuf() */
struct	var	v;
int	tok;
jmp_buf	jmp;
struct	syment	*File, *Inode, *Mount, *Swap, *Core, *Proc, *Sbuf,
		*Sys, *Time, *Panic, *Etext, *Text, *V,
#ifdef	ns32000
		*Kpte2,
#else
		*Sbrpte,
#endif
		*Buf, *End, *Callout, *Lbolt, *Dmpstk, *Curproc, *U,
		*Region;
struct	uarea	x;
extern	int	nmfd;

main(argc, argv)
	int	argc;
	char	**argv;
{
	char	*token();
	long	atol();
	int	sigint();
	register  struct  tsw	*tp;
	register  char  *c, *arg;
	int	cnt, units, r, prdef = (NBPW==4 ? HEX : OCTAL);
	unsigned	addr;
	struct	syment	*nmsrch(), *search(), *sp;
	struct	prmode	*prptr;
	extern	unsigned  ttycnt;

	switch(argc) {
	default:
		fatal("usage: crash  [ dump ]  [ namelist ]");
	case 3:
		if(*argv[2] != '-')
			namelist = argv[2];
	case 2:
		if(*argv[1] != '-')
			dumpfile = argv[1];
	case 1:
		;
	}

	setbuf(stdout,sbuf);
	init();
	setjmp(jmp);

	for(;;) {
		fflush(stdout);
		fflush(stdin);
		c = token();
		for(tp = t; tp->t_sw != 0; tp++)
			if(strcmp(tp->t_nm, c) == 0)
				break;
		switch(tp->t_sw) {

		default:
		case NULL:
			prompt("eh?\n");
			while(token() != NULL);
			continue;

		case UAREA:
			/* Uses the Process Table Slot */
			if((arg = token()) == NULL)
				pruarea(CURPROC);	/* Use _curproc */
			else do {
				pruarea(atoi(arg));
			} while((arg = token()) != NULL);
			continue;

		case PCBLK:
			/* Uses the Process Table Slot */
			if((arg = token()) == NULL)
				prpcb(CURPROC);	/* Use _curproc */
			else do {
				prpcb(atoi(arg));
			} while((arg = token()) != NULL);
			continue;

		case STACK:
			/* Uses the Process Table Slot */
			if((arg = token()) == NULL)
				prstack(CURPROC);	/* Use _curproc */
			else do {
				prstack(atoi(arg));
			} while((arg = token()) != NULL);
			continue;

		case TRACE:
			r = 0;
			if((arg=token()) == NULL) {
				prtrace(0,CURPROC);
				continue;
			}

			if(arg[0] == '-' && arg[1] == 'r') {
				r = 1;	/* Use kfp for stack trace */
				arg = token();
			}

			if(arg == NULL)
				prtrace(r,CURPROC);
			else do {
				if(isdigit(*arg))
					prtrace(r,atoi(arg));
				else {
					printf("bad arg: %s\n",arg);
					continue;
				}
			} while((arg = token()) != NULL);
			continue;

		case FILES:
			printf("SLOT  REF  INODE  FLAGS\n");
			if((arg = token()) == NULL)
				for(cnt = 0; cnt < (int)v.ve_file; cnt++)
					prfile(cnt, 0);
			else do {
				prfile(atoi(arg), 1);
			} while((arg = token()) != NULL);
			continue;

		case KFP:
			if((arg = token()) != NULL) {
				readinto(Kfp);
				prkfp(Kfp);
				while(token() != NULL);
			}
			else {
				prkfp(Kfp);
			}
			continue;

		case INODE:
			printf("SLOT MAJ  MIN INUMB REF LINK  UID  ");
			printf("GID   SIZE   MODE  SMAJ SMIN FLAGS\n");
			if((arg = token()) == NULL)
				for(cnt = 0; cnt < (int)v.ve_inode; cnt++)
					prinode(cnt, 0, 0);
			else if(*arg == '-')
				if((arg = token()) == NULL)
					for(cnt = 0; cnt < (int)v.ve_inode; cnt++)
						prinode(cnt, 1, 0);
				else
					do prinode(atoi(arg), 1, 1);
					while((arg = token()) != NULL);
			else
				do prinode(atoi(arg), 0, 1);
				while((arg = token()) != NULL);
			continue;

		case PROC:
			printf("SLT ST PID   PPID  PGRP   UID  EUID PRI CPU");
#ifndef ns32000
			printf("   SPT   NSPT");
#endif
			printf("   EVENT  NAME     FLAGS\n");
			if((arg = token()) == NULL)
				for(cnt = 0; cnt < (int)v.ve_proc; cnt++)
					prproc(cnt, 0, 0, 0);
			else if(*arg == '-') {
				r = arg[1] == 'r' ? 0 : 1;
				if((arg = token()) == NULL)
					for(cnt=0; cnt < (int)v.ve_proc; cnt++)
					prproc(cnt, r, !r, 0);
				else
					do prproc(atoi(arg), r, !r, 1);
					while((arg = token()) != NULL);
			} else
				do prproc(atoi(arg), 0, 0, 1);
				while((arg = token()) != NULL);
			continue;

		case TTY:
			printf("SLOT  RAW DEL CAN OUT  PGRP COL STATE\n");
			l1: if((arg = token()) == NULL)
				for(cnt = 0; cnt < ttycnt; cnt++)
					prtty(cnt, 0, 0);
			else if(*arg == '-')
				if((arg = token()) == NULL)
					for(cnt = 0; cnt < ttycnt; cnt++)
						prtty(cnt, 1, 0);
				else do prtty(atoi(arg), 1, 1);
					while((arg = token()) != NULL);
			else if(*arg >= '0' && *arg <= '9')
				do prtty(atoi(arg), 0, 1);
				while((arg = token()) != NULL);
			else if(!settty(arg)) {
				printf("invalid tty structure\n");
				while(token() != NULL);
				continue;
			} else goto l1;
			continue;

		case TEXT:
#ifdef	ns32000
			printf("text table not implemented on the ns32000\n");
			while(token() != NULL);
			continue;
#else
			printf("SLOT  INODE  REF  LDREF  ");
#ifdef	pdp11
			printf("TXTADDR  ");
#endif
#ifdef	vax
			printf("PROC  ");
#endif
			printf("SWAPLO+   SIZE  FLAGS\n");
			if((arg = token()) == NULL)
				for(cnt = 0; cnt < (int)v.ve_text; cnt++)
					prtext(cnt, 0);
			else do {
				prtext(atoi(arg), 1);
			} while((arg = token()) != NULL);
			continue;
#endif

		case MOUNT:
			printf("SLOT  MAJ  MIN  INODE  ");
			printf("BUF  VOLUME  PACK   BLOCKS INODES");
			printf("  BFREE  IFREE\n");
			if((arg = token()) == NULL)
				for(cnt = 0; cnt < (int)v.v_mount; cnt++)
					prmount(cnt, 0);
			else do {
				prmount(atoi(arg), 1);
			} while((arg = token()) != NULL);
			continue;

		case TS:
			while((arg = token()) != NULL) {
				readinto(addr);
#if vax || ns32000
				sp = search(addr, N_TEXT, N_TEXT);
#else
				sp = search(addr,
					(N_TEXT | N_EXT), (N_TEXT | N_EXT));
#endif
				if(sp == 0)
					printf("\tno match\n");
				else if(sp->n_name[0] == '_')
					printf("\t%.7s\n",&sp->n_name[1]);
				else
					printf("\t%.8s\n",sp->n_name);
			}
			continue;

		case DS:
			while((arg = token()) != NULL) {
				readinto(addr);
#if vax || ns32000
				sp = search(addr, N_DATA, N_BSS);
#else
				sp = search(addr,
					(N_DATA | N_EXT), (N_BSS | N_EXT));
#endif
				if(sp == 0)
					printf("\tno match\n");
				else if(sp->n_name[0] == '_')
					printf("\t%.7s", &sp->n_name[1]);
				else
					printf("\t%.8s", sp->n_name);
				if(sp)
					printf(" + %u.\n", addr - sp->n_value);
			}
			continue;

		case Q:
			printf("UNIX Dump Analyzer\n");
			printf("usage: crash [dump] [namelist]\n");
			printf("available commands:\n\n");
			for(tp = t; tp->t_sw != 0; tp++)
				if(tp->t_dsc != 0)
					printf("%s\t%s\n",tp->t_nm,tp->t_dsc);
			while(token() != NULL);
			continue;

		case PREGION:
			if((arg = token()) == NULL)
				for(cnt = 0; cnt < (int)v.ve_proc; cnt++)
					prpregion(cnt);
			else do {
				prpregion(atoi(arg));
			} while(arg = token());
			continue;

		case REGION:
			printf("pgoff  pgsz  nval  ref  rlist       iptr   forw         type   flags\n");
			if((arg = token()) == NULL)
				for(cnt = 0; cnt < v.v_region; cnt++)
					prregion(cnt);
			else do {
				prregion(atoi(arg));
			} while(arg = token());
			continue;

		case STAT:
			prstat();
			while(token() != NULL);
			continue;

		case BUFHDR:
			printf("BUF MAJ  MIN   BLOCK FLAGS\n");
			if((arg = token()) == NULL)
				for(cnt = 0; cnt < v.v_buf + v.v_sabuf; cnt++)
					prbufhdr(cnt);
			else do prbufhdr(atoi(arg));
				while((arg = token()) != NULL);
			continue;

		case BUFFER:
			if((arg = token()) == NULL)
				for(cnt = 0; cnt < v.v_buf; cnt++)
					prbuffer(cnt, prdef);
			else if(*arg >= '0' && *arg <= '9')
				do prbuffer(atoi(arg), prdef);
				while((arg = token()) != NULL);
			else {
				for(prptr = prm; prptr->pr_sw != 0; prptr++)
					if(!strcmp(arg, prptr->pr_name))
						break;
				if(prptr->pr_sw == 0) {
					error("invalid mode");
					while(token() != NULL);
					continue;
				}
				prdef = prptr->pr_sw;
				if((arg = token()) == NULL)
					for(cnt = 0; cnt < v.v_buf; cnt++)
						prbuffer(cnt, prptr->pr_sw);
				else do prbuffer(atoi(arg), prptr->pr_sw);
					while((arg = token()) != NULL);
			}
			continue;

		case TOUT:
			printf("FUNCTION ARGUMENT  TIME\n");
			prcallout();
			while(token() != NULL);
			continue;

		case NM:
			while((arg = token()) != NULL)
				prnm(arg);
			continue;

		case OD:
			if((arg = token()) == NULL) {
				error("symbol expected");
				continue;
			}
			if(isdigit(arg[0])) {
#ifdef	DEBUG
printf("arg=%s\n",arg);
#endif
				readinto(addr);
#ifdef	DEBUG
printf("addr=%u ", addr);
printf(FMT,addr);
#endif
			}
			else if((sp = nmsrch(arg)) == NULL) {
				printf("symbol not found\n");
				while(token() != NULL);
				continue;
			} else
				addr = (long)sp->n_value;

			if((arg = token()) == NULL) {
				units = 1;
				arg = NBPW==4 ? "hex" : "octal";
			} else {
				units = atoi(arg);
				if(units == -1) {
					while(token() != NULL);
					continue;
				}
				if((arg = token()) == NULL)
					arg = NBPW==4 ? "hex" : "octal";
				else
					while(token() != NULL);
			}
			prod(addr, units, arg);
			continue;

		case MAP:
			while((arg = token()) != NULL) {
				prmap(arg);
				putc('\n', stdout);
			}
			continue;

		case VAR:
			prvar();
			while(token() != NULL);
			continue;

		/*
		 * Print the active and sleep record lock tables;
		 * also verify correctness of record locking linked lists.
		 * Calls function prlcks to perform location and printing.
		 * prlcks() can be found in file crash/lck.c.
		 */

		case LCK:
			prlcks();
			continue;

		case QUIT:
			exit(0);
			close(nmfd);
		}
	}
}

sigint()
{
	char  *token();

	signal(SIGINT, sigint);
	p = line;
	tok = 1;
	prompt("\neh?\n");
	line[0] = '\0';
	while(token() != NULL);
	longjmp(jmp, 1);
}

char	*
token()
{
	register  char  *cp;

	for(;;)
		switch(*p) {
		case '\0':
		case '\n':
			if(*p == '\n') {
				p++;
				tok = 0;
			}
			if(tok != 0) {
				tok = 0;
				return(NULL);
			}
			prompt("> ");
			p = line;
			if(fgets(line, 256, stdin) == NULL)
				return("quit");
			if(line[0] == '!') {
				system(&line[1]);
				line[0] = '\0';
			}
			continue;

		case ' ':
		case '\t':
			p++;
			continue;

		default:
			tok++;
			cp = p;
			while(*p!=' ' && *p!='\t' && *p!='\n')
				p++;
			*p++ = '\0';
			return(cp);
	}
}

prompt(str)
	char *	str;
{
	while(*str)
		putc(*str++, stderr);
}

prkfp(fp)
{
	printf("kfp:  ");
	printf(FMT, fp);
	printf("\n");
	fflush(stdout);
	return;
}

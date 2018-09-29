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
 	/* @(#) runpcs.c: 1.13 10/14/83 */

/*
 *	UNIX debugger
 */

#include "head.h"

#if u3b

/* breakpoint instructions */
#define BPT1	0xDA000000	/* for 3B */
#define BPT2	0x0000DA00	/*	instruction may begin on half word */

#define MASK1	0xFFFF0000	/*	but ptrace works on word bounds */
#define MASK2	0x0000FFFF

#define exect	execve		/*  no trace bit on 3B ** !! */

#endif

#if vax

/* breakpoint instruction */
#define BPT	03

#endif
#if ns32000

/* breakpoint instruction */
#define BPT	0xf2

#endif

#if u3b5
#define exect	execve

static int opmask[4] = {0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff};
#define OPCODE (0x2e2e2e2e)
#endif

extern MSG		NOFORK;
extern MSG		ENDPCS;
extern MSG		BADWAIT;
extern MSG		NOPCS;

extern ADDR		sigint;
extern ADDR		sigqit;

/* breakpoints */
extern BKPTR		bkpthead;

extern CHAR		lastc;
extern int		errno;
extern INT		pid;
extern L_INT		loopcnt;

extern ADDR	userpc;



getsig(sig)
{	return(sig);
}

/* runpcs - control the user process, running it in single step or
 *	continuous mode, depending on runmode, and with signal execsig.
 *
 *	It has the side effect of deleting all breakpoints (calls delbp()).
 */
runpcs(runmode,execsig)
{
	REG BKPTR	bkpt;
	int cmdval;
#if u3b5
	int idsp;
	char fmt;
	long nextpc;

/* variables needed for single stepping floating point instructions: */

	register int val;	/* holds the word of instruction stream that */
				/* is to receive breakpoint */
	register ADDR adword;	/* address of word of instruction stream to */
				/* receive breakpoint */
	register int bytenum;	/* byte number within word of instruction stream */
				/* where breakpoint is to be placed */
	long savewd;		/* temporary to save original word of instruction */
				/* stream before breakpoint is inserted */
#endif

	/* for 'g' (goto) command, set userpc to address to which to jump */
	if (adrflg) userpc = dot;

	/* execute breakpoints (or single step) loopcnt number of times */
	while (loopcnt-- > 0) {
#if DEBUG > 1
		if(debugflag)
			fprintf(FPRT2, "runpcs(runmode=%d, execsig=%d);\n",
				runmode, execsig);
#endif
#if DEBUG
		if (debug) fprintf(FPRT2, "\tcontinue %#x %d\n",userpc,execsig);
#endif
		errno = 0;
		if (runmode == SINGLE) {
#if vax || ns32000 || u3b5
			/* hardware handles single-stepping */
			delbp();
			ptrace(SINGLE, pid, userpc, execsig);
			bpwait(); chkerr(); execsig=0; readregs();
#else
			/*  no single-step on 3B */
			execbkpt((BKPTR)0,execsig);	/* fake tracing */
			continue;
#endif

		}

		else {		/* continuing from a breakpoint is hard */
#if DEBUG > 1
			if(debugflag) fprintf(FPRT2,
			     " : userpc=%#x, dot=%#x; adrflg=%d; excuting=%d\n",
					userpc, dot, adrflg, executing);
#endif

			/* if truly continuing from bkpt, execute instruction
			 * that should be where breakpoint was set	    */
			if   ((bkpt=scanbkpt(userpc))
			  && !(adrflg && args[0]=='\0') /*  'g' cmd */
/* ?? 0 bkpt */		  && !(userpc==0 && (cmd=='r'||cmd=='R')))  
			{
				execbkpt(bkpt, execsig);
#if u3b5
				if(signo == SIGILL)
				{
	    				if (isfloat(getval(regvals[15],"c")))
					{
						fpstep(&execsig);
					}
					else
						sigprint();
				}
#endif
				execsig = 0;
			}
			setbp();
			ptrace(runmode, pid, userpc, execsig);
			bpwait(); chkerr(); execsig=0;
#if !u3b5
			delbp();
#endif
			readregs();
		}

#if u3b5

	/* We may have returned from bpwait() because we encountered a floating*/
	/* point opcode. Floating point is implemented using illegal opcodes. */
 	/* These opcodes trap to the operating system and the operating system */
	/* sends a SIGILL signal back to sdb.                               */

	/* If there was a SIGILL, check that it was a floating point (and not*/
	/* illegal) opcode by looking at the opcode located at the current pc. */
	/* If it is floating point, then tell ptrace to execute that instruction */
	/* according to the current runmode, by setting execsig equal to SIGILL*/
	/* and calling ptrace again.                                         */
	if(signo == SIGILL)  
	{
	    if(isfloat(getval(regvals[15],"c"))){
#if DEBUG 
	if (debug)
		printf("SIGILL:rvalue %x\n",regvals[15]);
#endif

		/* for non-single step modes - adjust the loop counter to reflect */
		/* that we want to go through the main loop one more time and continue */
		/* to execute                                                  */
		if(runmode != SINGLE)
		{
			loopcnt++;
			execsig = signo;
			continue;
		}

		/* for single step mode - the floating point implementation requires */
		/* sdb to be smart. The way sdb single steps instructions is through */
		/* the use of the trace trap, which occurs at the end of each instruction.*/
		/* The ptrace picks up the trace trap and returns control to sdb */
		/* after each instruction execution. A floating point instruction is */
		/* implemented by executing a subroutine of regular instructions. When */
		/* floating point is done, it returns to the operating system and the */
		/* operating system does a retg to get back to the next instruction */
		/* in the user program. The retg instruction, however, is a special */
		/* instruction that cannot be trace-trapped. So, the entire assembly */
		/* language after the floating point instruction will execute before */
		/* a trap occurs. (except for a floating pt instruction which will */
		/* generate SIGILL before executing.)                              */

		/* There are 3 float single step cases:                                 */
		/*      1. float followed by float instruction   */
		/*      2. non-float followed by float instruction */
		/*      3. float followed by non-float instruction */

		/* for the first 2 cases, single stepping does not have problems, since */
		/* the instructions can be stopped before execution (by trapping at the */
		/* end of the previous instruction, or by SIGILL). The third case, */
		/* however, causes the "retg problem" described above. This will cause */
		/* a problem with single stepping ONLY if the floating point instruction */
		/* is the last instruction of a "C" level statement and the     */
		/* non-floating point instruction is at the beginning of the next */
		/* "C" level statement. In that case, it will appear to the user to */
		/* skip an instruction.                                              */

		/* So, for case 3, we need to set a breakpoint on the non-floating */
		/* point instruction (we know its the  next instruction to execute */
		/* since floating point doesn't branch), run the process with trace */
		/* off to get to the breakpoint, then remove the breakpoint and */
		/* continue in the main loop.                                */

		/* As it turns out, this is also a convenient way to handle case 1. */
		/* Since, if we stayed in single stepping mode, tracing is on */
		/* during execution of the floating point instruction, each instruction */
		/* in the subroutine package will trace trap back to sdb, and sdb */
		/* would have to sit in a wait loop until the end of the floating point */
		/* subroutine (ie, until pc = pc of the next instruction) before it */
		/* continued to single step normally. So, for single stepping any */
		/* floating point instruction, set a breakpoint on the next instruction */
		/* run in continuous mode waiting for the breakpoint to fire, remove */
		/* the breakpoint, then continue single stepping normally.     */
		else
		{
			fpstep(&execsig);
		}
	}
	else sigprint();
	}
	    delbp();
	    if( (signo==0) && (runmode!=SINGLE) ) {
		/*3B5 leaves the user pc pointing after the BPT, when*/
		/*SDB expects it to be backed up to point at the BPT.*/
		/*the above bpwait is the only one which could stop*/
		/*due to BPT. (I hope)*/
		putreg(15,'l',regvals[15]-1); /*in access.c*/
		userpc = dot = regvals[15];
#if DEBUG
	    if (debug)
		fprintf(FPRT2, "\trunpcs: adrflg=%d, userpc=%#x, dot=%#x, pc=%#x\n",
			adrflg, userpc, dot, regvals[15]);
#endif
	    }
#endif
		/* stopped by BPT instruction */

		if ((signo==0) && (bkpt=scanbkpt(userpc))) {
	loop1:

#if DEBUG
			if (debug) fprintf(FPRT2, "\n BPT code; '%s'%#x'%#x'%d",
				bkpt->comm,bkpt->comm[0],'\n',bkpt->flag);
#endif
			dot=bkpt->loc;
			if (bkpt->comm[0] != '\n') {
				cmdval = acommand(bkpt->comm);
				/* if not "really" breakpoint, i.e.
				 * user did not give k command, then
				 * cmdval == 1, and should execute
				 * breakpoint instruction, not count
				 * as a breakpoint in the loop, and
				 * continue to the next breakpoint
				 */
				if (cmdval) {
					execbkpt(bkpt,execsig);
					execsig=0;
					/* if at another breakpoint,
					 * handle as before (check for
					 * user command, by going to
					 * loop1).
					 */
					if ((signo == 0) &&
					    (bkpt=scanbkpt(userpc)))
						goto loop1;
					loopcnt++;  /* Don't count iteration */
					continue;
				}
			}
		}

		/* else not stopped by a BPT instruction */
		else {
			execsig=signo;
			if (execsig) break;
		}
	}	/* end of while loop */
#if DEBUG
 		if (debug) fprintf(FPRT2, "Returning from runpcs\n");
#endif
}

#if u3b5
static fpstep(execsig)
int * execsig;
{
	int idsp;
	char fmt;
	long nextpc;

/* variables needed for single stepping floating point instructions: */

	register int val;	/* holds the word of instruction stream that */
				/* is to receive breakpoint */
	register ADDR adword;	/* address of word of instruction stream to */
				/* receive breakpoint */
	register int bytenum;	/* byte number within word of instruction stream */
				/* where breakpoint is to be placed */
	long savewd;		/* temporary to save original word of instruction */
		/* stream before breakpoint is inserted */
	/* get address of instruction following fp instruction*/

	idsp = DSP;
	nextpc = dis_dot(userpc,idsp,fmt);

	/* set a breakpoint at that next instruction */
	bytenum=nextpc%4;		/*byte num in word*/
	adword=nextpc-bytenum;		/*word address*/
	val=ptrace(RIUSER,pid,adword,0);
	savewd=val;
	val=(val&~opmask[bytenum]) | (OPCODE & opmask[bytenum]);
#if DEBUG
	if (debug)
	printf("FPstepping: nextpc=%lx, adword=%lx, savewd=%lx, val=%lx\n",nextpc,adword,savewd,val);
#endif
	val=ptrace(WIUSER,pid,adword,val);
	if (errno &&val ==-1)
	{
		fprintf(FPRT1,"ERROR: cannot set breakpoint to step float instruction\n");
		sigprint();
		return;
	}

	/* tell ptrace to run through the floating point instruction */
	*execsig=signo;
	ptrace(CONTIN,pid,userpc,*execsig);

	/* wait for bpt to fire */
	bpwait();
	chkerr();
	readregs();

	/* check that it was a breakpoint we hit */
	if (signo ==0)
	{
		/* m32 leaves user pc pointing after bpt, so back it up one byte */
		putreg(15,'l',regvals[15]-1);
		userpc=dot=regvals[15];

		if (userpc==nextpc)
		{
			/* remove the breakpoint */
			val=savewd;
			val=ptrace(WIUSER,pid,adword,val);
			if (errno && val ==-1)
			{
				fprintf(FPRT1,"ERROR: cannot remove breakpoint to step float instruction\n");
				sigprint();
				return;
			}
		}
		else
		{
			printf("ERROR: bad userpc=%lx after stepping float instruction\n",userpc);
			sigprint();
			return;
		}
	}
	else sigprint();
}
#endif

#define BPOUT 0
#define BPIN 1
INT bpstate = BPOUT;

/* endpcs() - kill child (user) process */
endpcs()
{
	REG BKPTR	bkptr;
#if DEBUG
 		if (debug) fprintf(FPRT2, "Entering endpcs with pid=%d\n", pid);
#endif
	if (pid) {
		errno = 0;
   		ptrace(EXIT,pid,0,0); pid=0;
		chkerr();

		/*  cleanup of breakpoint states not necessary.
		*	Only two modes being used are BKPTSET and 0.
		*/
/*		for (bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt) {
			if (bkptr->flag) bkptr->flag = BKPTSET;
		}
*/
	}
	bpstate=BPOUT;
}

/* setup() -initializes ptrace, forks the child (user) process, and
 *		waits for child.
 */
setup()
{
	close(fsym); fsym = -1;
	if ((pid = fork()) == 0)
	{
		ptrace(SETTRC,0,0,0);

		/* restore user interrupt handling mode */
		signal(SIGINT,sigint); signal(SIGQUIT,sigqit);
#if DEBUG
 		if (debug) fprintf(FPRT2, "About to doexec  pid=%d\n",pid);
#endif
		doexec(); exit(0);
	}
	else if (pid == -1) {
		error(NOFORK);
		longjmp(env);
	}
	else {
		bpwait();

#if vax || ns32000 || u3b
		/* read uu->u_ar0 */
		if (-1 == (int) (((struct user *)uu)->u_ar0 =
			(int *) ptrace(RUREGS, pid,
					(char *)&(((struct user *)uu)->u_ar0) -
					  (char *)uu,
					0)))
		{
			perror("Cannot ptrace child process; try again.");
			endpcs();
			longjmp(env);
		}

		/* convert absolute address in user area to sdb internal
		 * address by subtracting the address of the user area,
		 * and adding the address of sdb's copy of that area.
		 */
		(((struct user *)uu)->u_ar0 =
			(int *)((char *)(((struct user *)uu)->u_ar0)
				 - ADDR_U + (int)uu));
#endif

		readregs();
#if DEBUG > 1
		if(debugflag)
			fprintf(FPRT2, "setup(): userpc=%#x;\n", userpc);
#endif
#if DEBUG
		if (debug) fprintf(FPRT2, "About to open symfil = %s\n",symfil);
#endif
		fsym=open(symfil,0);
		if (errflg) {
			fprintf(FPRT1, "%s: cannot execute\n",symfil);
#if DEBUG
			 if (debug) fprintf(FPRT2, "%d %s\n", errflg, errflg);
#endif
			endpcs();
		}
	}
	bpstate=BPOUT;
}

/* execbkpt(bkptr, execsig) - execute the one instruction that was replaced
 *		by a preakpoint instruction.  It has the side effect of
 *		removing all breakpoints, by calling delbp().
 *
 *		In the case of the 3B, a NULL bkptr means that single
 *		stepping is requested, and that a breakpoint instruction
 *		has not just been encountered.
 */

#if vax || ns32000 || u3b5
static
execbkpt(bkptr, execsig)
BKPTR bkptr;
{
#if DEBUG
	if (debug) fprintf(FPRT2,"exbkpt: count=%d, loc=%#x; pid=%d\n",
			bkptr->count, bkptr->loc, pid);
#endif
#if DEBUG > 1
	if (debugflag)
		fprintf(FPRT2,"execbkpt: execsig=%d; userpc=%#x;\n",
			execsig,userpc);
#endif

#if u3b5
	delbp();
#endif

	ptrace(SINGLE, pid, bkptr->loc, execsig);
	bkptr->flag=BKPTSET;
	bpwait(); chkerr(); readregs();
}
#else
#if u3b
/* modified execbkpt() since no tracing on the 3B-20 !
 *	Added option for bkptr = 0 to fake single stepping.
 *	Because do not have single stepping via tracing
 *	set breakpoint at next physical instruction and/or
 *	the next logical instr (loc "jump'ed" to) if not too difficult.
 *	Note: need single stepping to implement just simple breakpoints !
 */
static
execbkpt(bkptr,execsig)
BKPTR	bkptr;
{
	register int ptv;
	long dis_dot();
	BKPT	bkpt1, bkpt2;
	unsigned key1, key2, k3, k4, k5, k6;
	union word word,w2;
#define opcode word.c[0]
	extern int errlev;		/*  in dis/bits.c */
	extern long dis_adr[];		/*  in dis/bits.c */
	long nextloc;
	long upc = userpc;
	long lb,ub;
	long rzeroval;
	long t2;
	int i,foundit,posoffset;

#if DEBUG
	if (debug && bkptr)
		fprintf(FPRT2, "exbkpt: count=%d, loc=%#x; pid=%d;\n",
			bkptr->count, bkptr->loc, pid);
#endif
#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "execbkpt: execsig=%d; upc=%#x;\n",execsig, upc);
#endif
	errno = 0;
	delbp();
	/* key1 and key2 are the first and second nibbles of the op code */
	word.w = chkget(upc, ISP);
#if DEBUG > 1
	if(debugflag)
	   fprintf(FPRT2, "  : word.w=%#x; word.s[0]=%#x;\n", word.w,word.s[0]);
#endif
	if(word.s[0] == (short)0xA100)	/* special 'nop' to align RET instr */
		word.w = chkget(upc+2, ISP);
	key1 = (word.c[0]>>4) & 0xf;
	key2 = word.c[0] & 0xf;
	bkpt1.loc = dis_dot(upc,ISP,'\0');	/* next physical instr */
	bkpt2.flag = 0;
	nextloc = dis_adr[1];
#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2,
			"  : key1=%#x, key2=%#x, nadr=%d, nextloc=%#x(,%#x);\n",
				key1, key2, dis_adr[0], nextloc,dis_adr[2]);
#endif
	if( !bkptr && opcode == 0xba ) /* is it a switch instr? */
	{
		k3 = (word.c[1]>>4) & 0xf;
		k4 = word.c[1] & 0xf;
		if (k4 == 8)
		{
			w2.w = chkget(upc+2,ISP);
			k5 = w2.w & 0x0000000f;
			w2.w = w2.w >> 4;
			rzeroval = SDBREG(k5);
			t2 = w2.w;
			foundit = 0;
			for ( i = 1 ; i <= k3 ; i++)
			{
				if (rzeroval == chkget(t2,DSP))
				{
					foundit = 1;
					break;
				}
				else
				{
					t2 = t2 + 4;
				}
			}
			if (foundit)
			{
				bkpt1.loc = chkget(w2.w+k3*4+4+(i-1)*4,DSP);
			}
			else
			{
				bkpt1.loc = chkget(w2.w+k3*4,DSP);
			}
		}
		else
		{
			fprintf(FPRT1, "Cannot single-step: ");
			printline();
			fprintf(FPRT1, "\n");
			longjmp(env, 0);
		}
	}
	else if( !bkptr && opcode == 0xbb ) /* is it a switcht instr? */
	{
		k3 = (word.c[1]>>4) & 0xf;
		k4 = word.c[1] & 0xf;
		if (k4 == 8)
		{
			w2.w = chkget(upc+2,ISP);
			w2.w = w2.w >> 4;
			lb = chkget(w2.w,DSP);
			ub = chkget(w2.w+4,DSP);
			rzeroval = SDBREG(k3);
			if (rzeroval < lb || rzeroval > ub)
			{
				bkpt1.loc = chkget(w2.w+8,DSP);
			}
			else
			{
				bkpt1.loc = chkget(w2.w+12+(rzeroval-lb)*4,DSP);
			}
		}
		else
		{
			fprintf(FPRT1, "Cannot single-step: ");
			printline();
			fprintf(FPRT1, "\n");
			longjmp(env, 0);
		}
	}
	else if( !bkptr && opcode == 0x7b ) /* is it a return instr? */
	{
		frame = SDBREG(FP);
		bkpt1.loc = get(NEXTCALLPC, DSP);
	}
	else if( !bkptr && opcode == 0x70 ) /* is it a branch instr? */
	{
		k3 = (word.c[1]>>4) & 0xf;
		k4 = word.c[1] & 0xf;
		switch(k4)
		{
			case 0:
				k5 = (word.c[2]>>4) & 0xf;
				rzeroval = SDBREG(k5);
				k6 = (word.w & 0xff0) >> 4;
				bkpt2.loc = rzeroval+k6;
				break;
			case 1:
				k5 = (word.c[2]>>4) & 0xf;
				rzeroval = SDBREG(k5);
				k6 = (word.w & 0xff0) >> 4;
				bkpt2.loc = chkget(rzeroval+k6,DSP);
				break;
			case 2:
				k5 = (word.c[2]>>4) & 0xf;
				rzeroval = SDBREG(k5);
				k6 = (word.w & 0xff0) >> 4;
				bkpt2.loc = rzeroval-k6;
				break;
			case 3:
				k5 = (word.c[2]>>4) & 0xf;
				rzeroval = SDBREG(k5);
				k6 = (word.w & 0xff0) >> 4;
				bkpt2.loc = chkget(rzeroval-k6,DSP);
				break;
			case 4:
				k5 = (word.c[2]>>4) & 0xf;
				rzeroval = SDBREG(k5);
				w2.w = chkget(upc+2,ISP);
				w2.w = w2.w >> 4;
				bkpt2.loc = rzeroval+w2.w;
				break;
			case 5:
				k5 = (word.c[2]>>4) & 0xf;
				rzeroval = SDBREG(k5);
				w2.w = chkget(upc+2,ISP);
				w2.w = w2.w >> 4;
				k6 = rzeroval+w2.w;
				bkpt2.loc = chkget(k6,DSP);
				break;
			case 8:
				w2.w = chkget(upc+2,ISP);
				bkpt2.loc = w2.w >> 4;
				break;
			case 9:
				w2.w = chkget(upc+2,ISP);
				w2.w = w2.w >> 4;
				bkpt2.loc = chkget(w2.w,DSP);
				break;
			default:
				if(bkptr)
					fprintf(FPRT1, "Cannot reset breakpoint: ");
				else
					fprintf(FPRT1, "Cannot single-step: ");
				printline();
				fprintf(FPRT1, "\n");
				longjmp(env, 0);
		}
		bkpt2.flag = 1;
		setbp1(&bkpt2);  /* if the branch is taken */
	}
	else if(errlev || key1 == 0x8 || key1 == 0x9 ||
			(key1 == 0x7 && key2 != 0xa && key2 != 0xe)) {
		if(errlev || nextloc == -1) {
			if(bkptr)
				fprintf(FPRT1, "Cannot reset breakpoint: ");
			else
				fprintf(FPRT1, "Cannot single-step: ");
			printline();
			fprintf(FPRT1, "\n");
			longjmp(env, 0);
		}
		else if (nextloc != bkpt1.loc) {
			bkpt2.loc = nextloc;	/* next logical instr */
			bkpt2.flag = 1;
			setbp1(&bkpt2);		/* in case branch is taken */
		}
	}
	setbp1(&bkpt1);			/* in case no branch, or not taken */
	ptv = ptrace(CONTIN,pid,userpc,execsig);	/* continue */
	if(errno && ptv == -1)
		fprintf(FPRT1,
			"Bad ptrace(CONTIN): ptv=%d; errno=%d; loc=%#x;\n",
					ptv, errno, bkptr->loc);
	if(bkptr)
		bkptr->flag=BKPTSET;
	bpwait(); chkerr(); readregs();
	if(bkpt2.flag)			/* in case "jump" instruction */
		delbp1(&bkpt2);		/* restore */
	delbp1(&bkpt1);			/* restore */
}
#endif
#endif

/* doexec() - performs exec call, after parsing arguments on sdb "run" command
 *		line.
 */
extern STRING environ;

doexec()
{
	char *argl[MAXARG], args[LINSIZ];
	register char c, redchar, *argsp, **arglp, *filnam;

	arglp = argl;
	argsp = args;
	*arglp++ = symfil;
	c = ' ';

	do {
		while (eqany(c, " \t")) {
			c = rdc();	/* get char from extern args[] */
		} 
		if (eqany(c, "<>")) {	/* redirecting I/O argument */
			redchar = c;
			do {
				c = rdc();
			} while (eqany(c, " \t"));
			filnam = argsp;
			do {
				*argsp++ = c;
				c = rdc();
			} while (!eqany(c, " <>\t\n"));
			*argsp++ = '\0';
			if (redchar == '<') {
				close(0);
				if (open(filnam,0) < 0) {
					fprintf(FPRT1,
						"%s: cannot open\n",filnam);
					 exit(0);
				}
			} else {
				close(1);
				if (creat(filnam,0666) < 0) {
					fprintf(FPRT1,
						"%s: cannot create\n",filnam);
					 exit(0);
				}
			}
		} else if (c == '"' || c == '\'') {  /*  quoted argument */
			char sc = c;
			*arglp++ = argsp;
			for (c = rdc(); c != sc; c = rdc()) {
				if (c == '\\')
					c = bsconv(rdc());
				*argsp++ = c;
			}
			c = rdc();
			*argsp++ = '\0';
		} else if (c != '\n') {		/* a regular argument */
			*arglp++ = argsp;
			do {
				*argsp++ = c;
				c = rdc();
			} while(!eqany(c, " <>\t\n"));
			*argsp++ = '\0';
		}
	} while (c != '\n');
	*arglp = (char *) 0;
#if DEBUG
	if (debug) {
		char **dap;
		fprintf(FPRT2,
			"About to exect(%s, %d, %d)\n",symfil,argl,environ);
		for (dap = argl; *dap; dap++) {
			fprintf(FPRT2, "%s, ", *dap);
		}
	}
#endif
	exect(symfil, argl, environ);
	perror("Returned from exect");
}

/* scanpkpt() - returns a pointer to the entry in the breakpoint list
 *		corresponding to the address given, if that is a
 *		breakpoint address.  Otherwise returns NULL pointer.
 */
BKPTR
scanbkpt(adr)
ADDR adr;
{
	REG BKPTR	bkptr;
	for(bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt) {
		if (bkptr->flag && bkptr->loc==adr) break;
	}
	return(bkptr);
}

/* delbp(). Remove breakpoints [delbp1() puts original instruction back] */

delbp()
{
	REG BKPTR	bkptr;

#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "delbp():\n");
#endif
	if (bpstate != BPOUT) {
		for (bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt) {
			if (bkptr->flag) {

				delbp1(bkptr);	/*  separate subr */
			}
		}
		bpstate=BPOUT;
	}
}

/* delbp1(bkptr) - deletes breakpoint given by bkptr */

#if vax || ns32000
static
delbp1(bkptr)
BKPTR	bkptr;
{
	register long adr;
	register int ptv;
	struct proct *procp;

	adr = bkptr->loc;
	ptv = ptrace(RIUSER, pid, adr, 0);
	if (!(errno && ptv == -1)) {
#if DEBUG > 1
		if (debugflag)
			fprintf(FPRT2, "delbp1(): adr=%#x; ptv=%#x;\n",
				adr,ptv);
#endif
		ptv = ptrace(WIUSER, pid, adr,
				(bkptr->ins & 0xff) | (ptv & ~0xff));
		if (!(errno && ptv == -1)) return;
	}
	procp = adrtoprocp(adr);
	if (procp->pname[0] == '_')
		fprintf(FPRT1, "Cannot delete breakpoint:  %s:%d @ %d\n",
			(procp->pname)+1, adrtolineno(adr, procp), dot);
	else
		fprintf(FPRT1, "Cannot delete breakpoint:  %s:%d @ %d\n",
			procp->pname, adrtolineno(adr, procp), dot);
}

#else
#if u3b
/* separated delbp1() from delbp() so execbkpt() can call  (3B) */
static
delbp1(bkptr)
BKPTR	bkptr;
{
	register long adr, wa, val;
	register int  ptv;
	struct proct *procp;

	adr = bkptr->loc;
	wa = adr & ~(WORDSIZE-1);	/*  word boundary */
	errno = 0;
	ptv = ptrace(RIUSER, pid, wa, 0);
	if( !(ptv == -1 && errno) ) {
		val = ptv;
#if DEBUG > 1
		if(debugflag)
			fprintf(FPRT2, "delbp1(): adr=%#x; wa=%#x; val=%#x;\n",
					adr, wa, val);
#endif
		if(adr == wa)
			val = (bkptr->ins & MASK1) | (val&(~MASK1));
		else		/* instr on half word boundary */
			val = (bkptr->ins & MASK2) | (val&(~MASK2));
#if DEBUG > 1
		if(debugflag)
			fprintf(FPRT2, "	new val = %#x;\n", val);
#endif
		ptv = ptrace(WIUSER,pid,wa,val);
	}
	if (errno && ptv == -1)
	{
		procp = adrtoprocp(adr);
		fprintf(FPRT1, "Cannot delete breakpoint:  %s:%d @ %d\n",
		procp->pname, adrtolineno(adr, procp), dot);
	}
}
#else
#if u3b5
static
delbp1(bkptr)
BKPTR	bkptr;
{
	register ADDR aword;
	register int amod4;
	register int ptv;

	amod4 = bkptr->loc % 4;
	aword = bkptr->loc - amod4;	/* word aligned address */
	errno = 0;
	ptv = ptrace(RIUSER,pid,aword,0);
	if (!(errno && ptv == -1))
	{
#if DEBUG > 1
		if (debugflag)
			fprintf(FPRT2,"delbp1(): aword=%#x; ptv=%#x;\n",
				aword,ptv);
#endif
		ptv = (ptv & ~opmask[amod4]) | (bkptr->ins & opmask[amod4]);
		/* writing just the opcode handles the case where */
		/* two bkpts are within one word. */
		ptv = ptrace(WIUSER,pid,aword,ptv);
	}
	if (errno && ptv == -1)
		fprintf(FPRT1,"Cannot delete breakpoint: %s:%d @ %d\n",
			adrtoprocp(bkptr->loc)->pname,
			adrtolineno(bkptr->loc),bkptr->loc);
}
#endif
#endif
#endif


/* setbp() -
 * Insert breakpoints [setbp1() overwrites instruction space with bpt instr] *
 */
setbp()
{
	REG BKPTR	bkptr;

#if DEBUG > 1
	if(debugflag)
		fprintf(FPRT2, "setbp():\n");
#endif
	if (bpstate != BPIN) {
		for (bkptr=bkpthead; bkptr; bkptr=bkptr->nxtbkpt) {
			if (bkptr->flag) {
				setbp1(bkptr);	/*  separate subr */
			}
		}
		bpstate=BPIN;
	}
}

/* setpb1(bkptr) - sets breakpoint given by bkptr */

#if vax || ns32000
static
setbp1(bkptr)
BKPTR	bkptr;
{
	register long adr;
	register int ptv;
	struct proct *procp;
	adr = bkptr->loc;
	errno = 0;
	ptv = ptrace(RIUSER, pid, adr, 0);

	if (!(errno && ptv == -1)) {
		bkptr->ins = ptv;
#if DEBUG > 1
		if (debugflag)
			fprintf(FPRT2, "setbp1(): adr=%#x; ins=%#x\n", 
				adr, ptv);
#endif
		ptv = ptrace(WIUSER, pid, adr, BPT | ptv &~0xff);
		if (!(errno && ptv == -1)) return;
	}

	procp = adrtoprocp(adr);
	if (procp->pname[0] == '_')
		fprintf(FPRT1, "Cannot set breakpoint:  %s:%d @ %d\n",
			(procp->pname)+1, adrtolineno(adr, procp), dot);
	else
		fprintf(FPRT1, "Cannot set breakpoint:  %s:%d @ %d\n",
			procp->pname, adrtolineno(adr, procp), dot);
}

#else
#if u3b
/* separated setbp1() from setbp() so execbkpt() can call */
static
setbp1(bkptr)
BKPTR	bkptr;
{
	register long adr, wa, val;
	register int  ptv;
	struct proct *procp;

	adr = bkptr->loc;
	wa = adr & ~(WORDSIZE-1);	/*  word boundary */
	errno = 0;
	ptv = ptrace(RIUSER, pid, wa, 0);
	if( !(ptv == -1 && errno) ) {
		bkptr->ins = ptv;
#if DEBUG > 1
		if(debugflag)
			fprintf(FPRT2, "setbp1(): adr=%#x; wa=%#x; ins=%#x;\n",
					adr, wa, bkptr->ins);
#endif
		if(adr == wa)
			val = BPT1 | (bkptr->ins&(~MASK1));
		else		/* instr on half word boundary */
			val = BPT2 | (bkptr->ins&(~MASK2));
		ptv = ptrace(WIUSER, pid, wa, val);
#if DEBUG > 1
		if(debugflag)
			fprintf(FPRT2, "	new val=%#x(%#x); wa=%#x;\n",
						ptv, val, wa);
#endif
	}
	if (errno && ptv==-1) {
		procp = adrtoprocp(adr);
		fprintf(FPRT1, "Cannot set breakpoint:  %s:%d @ %d\n",
			procp->pname, adrtolineno(adr, procp), dot);
	}
}

#else
#if u3b5
static
setbp1(bkptr)
BKPTR	bkptr;
{
	register ADDR aword;
	register int amod4;
	register int ptv;

	errno = 0;
	amod4 = bkptr->loc % 4;
	aword = bkptr->loc - amod4;	/* word aligned address */
	ptv = ptrace(RIUSER,pid,aword,0);

	if (!(errno && ptv == -1))
	{
		bkptr->ins = ptv;
#if DEBUG > 1
		if (debugflag)
			fprintf(FPRT2,"setbp1(): aword=%#x; ins=%#x;\n",
				aword,bkptr->ins);
#endif
		ptv = (ptv & ~opmask[amod4]) | (OPCODE & opmask[amod4]);
		ptv = ptrace(WIUSER,pid,aword,ptv);
	}
	if (errno && ptv == -1)
		fprintf(FPRT1,"Cannot set breakpoint: %s:%d @ %d\n",
			adrtoprocp(bkptr->loc)->pname,
			adrtolineno(bkptr->loc),bkptr->loc);
}
#endif
#endif
#endif


bpwait()
{
	REG ADDR w;
	INT stat;

	signal(SIGINT, 1);
#if DEBUG
	if (debug) fprintf(FPRT2, "Waiting for pid %d\n", pid);
#endif
	while ((w = wait(&stat)) != pid  &&  w!=-1) ;
#if DEBUG
	if (debug) fprintf(FPRT2, "Ending wait\n");
	if (debug) fprintf(FPRT2, "w = %d; pid = %d; stat = %#x;\n",w,pid,stat);
#endif
/*	dot = userpc = SDBREG(PC);	*/
	signal(SIGINT,sigint);
	if (w == -1) {
		pid = 0;
		errflg=BADWAIT;
	}
	else if ((stat & 0177) != 0177) {
		if (signo = stat&0177) sigprint();
		if (stat&0200) {
			error(" - core dumped");
			close(fcor);
			corfil = "core";
			setcor();	/* get most recent core dump */
		}
		pid=0;
		errflg=ENDPCS;
	}
	else {
		signo = stat>>8;
#if DEBUG
    		if (debug)
		    fprintf(FPRT2, "stat = %#x, signo = %d\n", stat, signo);
#endif

#if u3b5
		if (signo == SIGTRAP)
			signo = 0;
		else
			if (signo != SIGILL)
				sigprint();
#else
#if vax || ns32000 || u3b
		if (signo != SIGTRAP &&
		    ptrace(RUREGS, pid, SYSREG(PC), 0) != adsubn)
		{
			sigprint();
		}
		else	signo = 0;
#endif
#endif
	}
}

readregs()
{
	/*get REG values from pcs*/
	REG i;
#if u3b5
	int *uar0;	/* local copy of u.u_ar0 in child process */

	errno = 0;
	uar0 = (int *)ptrace(RUREGS,pid,(int)&(((struct user *)0)->u_ar0),0);
	if (errno)
	{
		error("readregs(): could not read u_ar0\n");
		return(ERROR);
	}
	for (i = 0; i < NUMREGS; i++)
		if (reglist[i].roffs != -1)
		{
			regvals[i] =
				ptrace(RUREGS,pid,&(uar0[reglist[i].roffs]),0);
			if (errno)
			{
				fprintf(FPRT1,"readregs(): could not read r%d;\n",i);
				return(ERROR);
			}
		}

#else
#if vax || ns32000 || u3b
	for (i=NUMREGLS - 1; --i>=0;)
	{
        	SDBREG(reglist[i].roffs) =
                    ptrace(RUREGS, pid, SYSREG(reglist[i].roffs), 0);
	}
#endif
#endif

	if( pid == 0 && PC < 0 )
	{
		error( NOPCS );
		return(ERROR);
	}
	else
	{
#if u3b5
		dot = userpc = regvals[15];
#else
		dot = userpc = SDBREG(PC);
#endif
	}
}

char 
readchar() {
	lastc = *argsp++;
	if (lastc == '\0') lastc = '\n';
	return(lastc);
}

char
rdc()
{
	register char c;

	c = *argsp++;
	return(c == '\0' ? '\n' : c);
}

#if u3b5

/* This routine checks to find out what caused the 
 *  signal SIGILL to be generated in the user program.
 *  This routine actually determines if the opcode is
 *  a floating point opcode, since 3b5 floating point
 *  instructions cause the SIGILL signal to be generated.
 */

/* floating point opcodes */


#define fadds2 0x31
#define fsubs2 0x41
#define fmuls2 0x51
#define fdivs2 0x61
#define movhs  0x71
#define movws  0x81
#define movss  0x91
#define movsd  0xa1
#define movsh  0xb1
#define movtsh 0xc1
#define movsw  0xd1
#define movtsw 0xe1
#define fcmps  0xf1

#define faddd2 0x35
#define fsubd2 0x45
#define fmuld2 0x55
#define fdivd2 0x65
#define movhd 0x75
#define movwd 0x85
#define movds 0x95
#define movdd 0xa5
#define movdh 0xb5
#define movtdh 0xc5
#define movdw 0xd5
#define movtdw 0xe5
#define fcmpd 0xf5

#define fadds3 0x39
#define fsubs3 0x49
#define fmuls3 0x59
#define fdivs3 0x69
#define faddd3 0x79
#define fsubd3 0x89
#define fmuld3 0x99
#define fdivd3 0xa9

int
isfloat(instr)
char instr;
{

	int flotfnd;
	flotfnd = FALSE;

#if DEBUG 
	if (debug)
	printf("opcode %x\n",instr);
#endif
	switch(instr){
		case fadds2:
		case fsubs2:
		case fmuls2:
		case fdivs2:
		case movhs:
		case movws:
		case movss:
		case movsd:
		case movsh:
		case movtsh:
		case movsw:
		case movtsw:
		case fcmps:
		case faddd2:
		case fsubd2:
		case fmuld2:
		case fdivd2:
		case movhd:
		case movwd:
		case movds:
		case movdd:
		case movdh:
		case movtdh:
		case movdw:
		case movtdw:
		case fcmpd:
		case fadds3:
		case fsubs3:
		case fmuls3:
		case fdivs3:
		case faddd3:
		case fsubd3:
		case fmuld3:
		case fdivd3:
			flotfnd = TRUE;
			break;

		default:
			flotfnd = FALSE;
			break;
	}
	return(flotfnd);
}
#endif

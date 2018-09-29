/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:com/machdep.c	1.23"

/*
 ****		MACHINE and OPERATING SYSTEM DEPENDENT
 ****		Routines which deal with the run-time stack
 */

#include "head.h"
#include "coff.h"




extern BKPTR	bkpthead;
extern MSG		NOPCS;

long rdsym();			/* in symt.c */
extern SYMENT syment;		/* rdsym() stores symbol table entry */
extern AUXENT auxent[];		/* rdsym() stores auxiliary entry */
extern int gflag;		/* initfp() in symt.c sets */
static int frameflg;		/* set if now past last frame */
static int broken;		/* broken frame stack */
static int nosave;
static int savrflag;
static ADDR topoffrm;		/* top of frame if ap < fp */
static ADDR epargp;		/* Fortran entry point prologue builds an */
				/* extra frame and saves the old ap */



/* initialize frame pointers to top of call stack */
/*  MACHINE DEPENDENT */
struct proct *
initframe() {
	register struct proct *procp;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("initframe");
	}
	else if (debugflag == 2)
	{
		enter2("initframe");
		closeparen();
	}
#endif
	if ((pid == 0) && (fcor < 0 || fakecor)) {
		frameflg = 1;
		return(badproc);
	}

#if vax || u3b
	argp = SDBREG(AP);
	frame = SDBREG(FP);
	callpc = SDBREG(PC);
#else
#if u3b5 || u3b15 || u3b2
	argp = regvals[10];
	frame = regvals[9];
	callpc = regvals[15];
#endif
#endif

	procp = adrtoprocp(callpc);
	frameflg = 0;
	topoffrm = 0;
	broken = 0;

#if u3b || u3b2 || u3b5 || u3b15
	/*
	**	if (frame <= argp)  the stack frame is broken.
	**	try to fix it.
	*/	

	if (frame <= argp) 
		broken = fixframe(procp);
#endif

#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("initframe");
		printf("0x%x",procp);
		endofline();
	}
#endif
	return(procp);
}
/*--------------------------------------*/

fixframe(procp)
register struct proct *procp;
{
	register struct proct *tmprocp;
	extern ADDR dpstart, dpsize;
	union word word;
	
#if DEBUG
	if (debugflag == 2)
	{
		enter2("fixframe");
		closeparen();
		endofline();
	}
#endif
/*
 * if pc is in the range of doprnt.s text, reproduce a frame
 */
	if ((callpc > dpstart) && (callpc < dpstart + dpsize))
	{
#if u3b2 || u3b5 || u3b15
		fixdprnt(regvals[12]);
#else
		fixdprnt(SDBREG(SP));
#endif
		return 0;
	}

/*
 *  FORTRAN entry points. the prolog pushes the original ap on the stack,
 *  followed by the arguments. ap is set to the first argument.
 */
	if ((argp - frame) == 2*WORDSIZE)
	{
		tmprocp = procp;
		while ((tmprocp->notstab) && (tmprocp != badproc))
			tmprocp = adrtoprocp(tmprocp->paddress -1);
		if ((tmprocp->sfptr)->f_type == F77) {
			epargp = get(argp - WORDSIZE, DSP); /*save the real ap*/
			return 0;
		}
	}

/*
 *  Next instruction is 'RET', fp is restored,  (for 3b2, 3b5 )	
 *  or just before the first instruction of the procedure.
 *  In both cases, sp  points to top of frame.
 */

#if u3b2 || u3b5 || u3b15
	word.w = get(callpc,ISP);
	if (nosave  || (frame == argp) || (word.c[0]  == 0x08) || 
				(callpc == procp->paddress)) 
	{
		topoffrm = regvals[12];
		return 0;
	}
#else
	if (nosave || (callpc == procp->paddress))
	{
		topoffrm = SDBREG(SP);	/* SP points to top of frame */
		return 0;
	}
#endif

	return 1;
}
/*--------------------------------------*/
/*
 * _doprnt uses fp as a general purpose register. 
 * Before any of the printf routines jmp to doprnt, LOCALS bytes are pushed
 * on the stack for local variables, and ap is saved at (SP - APSAVE).
 * In doprnt itself a max of 3 words are pushed on the stack - so fixprnt
 * checks up to 3 locations to find the last frame.
 */
#if u3b2 || u3b5 || u3b15
#define LOCALS	468
#else
#define LOCALS	460
#endif
#define APSAVE	16

fixdprnt(addr)
ADDR addr;
{
	int i;

#if DEBUG
	if (debugflag == 2)
	{
		enter2("fixdprnt");
		closeparen();
		endofline();
	}
#endif
	addr -= (LOCALS + SAVEDREGS*WORDSIZE) ;
	
	for (i = 0; i < 3; i++)
	{
	     if ( (isisp(get(addr, DSP))) &&
		      (get (addr + WORDSIZE,DSP) > USRSTACK ) &&
		      (get (addr + 2*WORDSIZE,DSP) > USRSTACK ) )
				break;
	      else
				addr -= WORDSIZE;
	}
	frame = addr + (SAVEDREGS*WORDSIZE);
	argp =  get(frame + LOCALS - APSAVE, DSP);
	
}
/*--------------------------------------*/
/*
 *  savr frame
 */
fixsavr()
{

#if DEBUG
	if (debugflag == 2)
	{
		enter2("fixsavr");
		closeparen();
		endofline();
	}
#endif
#if u3b2 || u3b5 || u3b15
	if (!isisp(callpc) && (callpc < USRSTACK)) {
#else
	if (!isisp(callpc)) {
#endif
		topoffrm = NEXTCALLPC - WORDSIZE;
		callpc = get(topoffrm, DSP);
		argp = get(frame +  3 * WORDSIZE,DSP);
		frame = get(NEXTFRAME, DSP);
	} 
}
/*--------------------------------------*/

struct proct *
nextframe() {
	register struct proct *procp;
	extern ADDR dpstart, dpsize;
	ADDR oldap;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("nextframe");
	}
	else if (debugflag == 2)
	{
		enter2("nextframe");
		closeparen();
	}
#endif
#if u3b5 || u3b15 || u3b2 || u3b
	/*
 	**	If we have stopped in an assembly language
 	**	routine which does not do a save, then
 	**	the frame pointer will not have been saved
	**	and reset for the current frame. 
 	*/
	if (broken) 
		return(badproc);	/* broken stack frame */
	if (epargp) {
		argp = epargp;
		epargp = 0;
	}
	oldap = argp;			/* save old AP */
	if (frame <= argp) {
		if (topoffrm == 0)  {		 /*topoffrm is not set */
			frameflg = 1;
			return(badproc);
		}
		callpc = get(topoffrm - 2*WORDSIZE, DSP);
		if (savrflag)
			fixsavr();
		else 
			argp = get(topoffrm - 1*WORDSIZE, DSP);
	}
	else {
		callpc = get(NEXTCALLPC, DSP);
		if (savrflag)
			fixsavr();
		else {
			argp = get(NEXTARGP, DSP);
			frame = get(NEXTFRAME, DSP);
		}
	}
 
#endif
#if vax
	callpc = get(NEXTCALLPC, DSP);
	argp = get(NEXTARGP, DSP);
	frame = get(NEXTFRAME, DSP)
#endif

#if u3b || u3b5 || u3b15 || u3b2
	if (!savrflag)
		topoffrm = oldap;	/* save old AP in case next frame has no fp */
#endif

/*
 * if in _doprnt fix frame
 */
	if ((callpc > dpstart) && (callpc < dpstart + dpsize))
		fixdprnt(oldap);
	procp = adrtoprocp(callpc-1);
	frameflg = ((procp == badproc) || (eqstr("_start", procp->pname)));
	if (!frameflg) {
		if (savrflag) 
			savrflag = 0;
		else if (argp >= frame)
			broken = fixframe(procp);
		if (eqstr("savr", procp->pname)) 
			savrflag = 1;
	}

#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("nextframe");
		printf("0x%x",procp);
		endofline();
	}
#endif
	return(procp);
}

/* returns core image address for variable */
/* formaddr() should be in symt.c ?? */
ADDR
formaddr(class, addr)
register char class;
ADDR addr; {
	ADDR x;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("formaddr");
	}
	else if (debugflag == 2)
	{
		enter2("formaddr");
		arg("class");
		printf("0x%x",class);
		comma();
		arg("addr");
		printf("0x%x",addr);
		closeparen();
	}
#endif
	switch(class) {
	case C_REG:
	case C_REGPARM:
		x = stackreg(addr);
		break;
	case C_EXT:
	case C_STAT:
		x = addr;
		break;
	case C_MOS:
	case C_MOU:
	case C_MOE:
	case C_FIELD:			/* ?? */
		x = addr;
		break;
		
	case C_ARG:
		x = argp+addr;
		break;
		
	case C_AUTO:
		x = frame+addr;	/* addr was negated in outvar() */
		break;

	default:
		fprintf(FPRT1,"Bad class in formaddr: %d(%#x)\n", class, class);
		x = 0;
		break;
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("formaddr");
		printf("0x%x",x);
		endofline();
	}
#endif
	return x;
}

/*
 *  stackreg(reg):
 * This routine is intended to be used to find the address of a register
 * variable.  The stack is searched backwards from the current frame
 * until the procedure with the register variable is found.  If the
 * register assigned to the variable appears on the stack, then its
 * address is returned.  Otherwise, the register variable is still in
 * the named register, so the register number is returned.  We distinguish
 * addresses from register numbers by noting that register numbers are less
 * than NUMREGS (16) and that stack addresses are greater.
 *
 *  MACHINE DEPENDENT
 */
/* new stackreg() for 3B Unix
 *	C stores 'n' registers starting with %R8 and going backwards
 *	can only get 'n' by looking at text -- bits 8->11 from beginning of proc
 */

#if u3b || u3b5 || u3b15 || u3b2
#define REGSAV1		8
#define ISREGSAV(xr,xn)	(((REGSAV1-xn) < xr) && (xr <= REGSAV1))
#if u3b
#define NUMREGSOFF	1
#define REGADDR(xr)	(frame - (REGSAV1-xr+1)*REGSIZE)
#else
#define NUMREGSOFF	(9 - (getval(procp->st_offset+1,"bu",DSP)&0x0f))
#define REGADDR(xr,xn)	(frame - (ADDR)(15-xr-xn)*REGSIZE)
#endif
ADDR
stackreg(reg)
ADDR reg;
{
	register struct proct *procp;
	ADDR regaddr;
	unsigned int nrs;
	/* unsigned int nn;	don't adjust addresses - see below */

#if DEBUG
	if (debugflag ==1)
	{
		enter1("stackreg");
	}
	else if (debugflag == 2)
	{
		enter2("stackreg");
		arg("reg");
		printf("0x%x",reg);
		closeparen();
	}
#endif
	/*  if no procedure to search down to, return reg */
	if (proc[0] == '\0')
	{
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("stackreg");
			printf("0x%x",reg);
			endofline();
		}
#endif
		return(reg);
	}


	regaddr = reg;
	for (procp=initframe(); procp != badproc; procp=nextframe()) {
		if (sl_procp == procp) break;
#if u3b
		nrs = getval(procp->paddress +NUMREGSOFF, "bu", DSP);
		nrs >>= 4;	/* get high four bits */
		if (ISREGSAV(reg,nrs)) {
			regaddr = REGADDR(reg);
#else
		nrs = NUMREGSOFF;
		if (ISREGSAV(reg,nrs)) {
			regaddr = REGADDR(reg,nrs);
#endif
			/*  3B stores chars and shorts in registers
			 * right justified, even though they are stored
			 * left justified in core.  Register variables
			 * are stored in register image on the stack,
			 * and that is the way they should be treated,
			 * so that the calling function doesn't have
			 * to check again whether they are actually
			 * in the registers or not.
			 * nn = WORDSIZE - typetosize(sl_type, sl_size);
			 * if (nn > 0)
			 * 	regaddr += nn;
			 */
		}
	}
	if (procp == badproc) {
		fprintf(FPRT1, "Stackreg() error: frame=%#x\n", frame);
		regaddr = ERROR;
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("stackreg");
		printf("0x%x",regaddr);
		endofline();
	}
#endif
	return(regaddr);
}

#else
#if vax
/*  VAX version - C stores 'n' registers arbitrarily.
 *	A mask (16 bits) is stored as the high order short in the word which is
 *	offset one word from the beginning of the frame (frame + WORDSIZE).
 *	The 0'th bit of this mask tells whether register 0 has been saved,
 *	the 1'st bit tells wheter register 1 has been saved, etc.
 *	Each saved register in the frame occupies another word of space,
 *	and they are stored in order, from lowest to highest numbered.
 *	The first saved register is at frame + 5 * WORDSIZE.
 */

#define REGOFF 5 * WORDSIZE
#define MASKOFF WORDSIZE
ADDR
stackreg(reg) {
	register int regfl, mask, i;
	register struct proct *procp;
	ADDR regaddr;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("stackreg");
	}
	else if (debugflag == 2)
	{
		enter2("stackreg");
		arg("reg");
		printf("0x%x",reg);
		closeparen();
	}
#endif
	if (proc[0] == '\0')
	{
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("stackreg");
			printf("0x%x",reg);
			endofline();
		}
#endif
		return(reg);
	}
	regaddr = reg;
	regfl = 0x10000 << reg;
	for (procp=initframe(); procp!=badproc; procp=nextframe()) {
		if (sl_procp == procp)
			break;
		if ((mask = get(frame + MASKOFF)) & regfl) {
			regaddr = frame + REGOFF;
			for (i=0; i < reg; i++) {
				if (mask & 0x10000) regaddr += WORDSIZE;
				mask >>= 1;
			}
		}
	}
	if (procp == badproc) {
		fprintf(FPRT1, "Stackreg() error: frame=%#x\n", frame);
		regaddr = ERROR;
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("stackreg");
		printf("0x%x",regaddr);
		endofline();
	}
#endif
	return(regaddr);
}
#endif
#endif

/* Print the stack trace: 't' and 'T' commands
 * set top (i.e. top != 0) to print just the top procedure
 * modified to print something even if a.out stripped
 */

#if u3b

/*  The 3B saves 13 words worth of registers before saving arguments */
#define NARGSTACK(X)	( ((frame - X) / WORDSIZE) -13)
#else
#if u3b5 || u3b15 || u3b2
#define NARGSTACK(X)	(((frame - X) / WORDSIZE) - 9)
#else
#if vax
/*  The number of words stored as arguments is in the first byte
 *	of the zero'th argument.  The remaining bytes of the word should
 *	be zero.  Argp is set to point to the first argument.
 */
#define NARGSTACK(X)    (X += WORDSIZE, \
			(narg = get(X-WORDSIZE, DSP)) & ~0xff ? 0 : narg\
		     )
#endif
#endif
#endif

prfrx(mode) 			/* mode = 2 assume no save instructin */
int mode;			/* mode = 1  only top frame */
{
	int narg;		/* number of words that arguments occupy */
	long offs;		/* offset into symbol table */
	register char class;	/*storage class of symbol */
	register int endflg;	/* 1 iff cannot find symbolic names of
					more arguments */
	int subsproc = 0;	/* 1 iff not top function on stack */
	int top;
	register char *p;	/* pointer to procedure (function) name */
	int line_num;		/* line number in calling function */
	register struct proct *procp;
	SYMENT *syp = &syment;
	AUXENT *axp = auxent;
	ADDR argpl;	/* local copy of argp, nextframe() needs the original */
	int val;
	
#if DEBUG
	if (debugflag ==1)
	{
		enter1("prfrx");
	}
	else if (debugflag == 2)
	{
		enter2("prfrx");
		arg("top");
		printf("0x%x",top);
		closeparen();
	}
#endif
	switch(mode) {
	case 1:
			top = 1;
			nosave = 0;
			break;
	case 2:
			top = 0;
			nosave = 1;
			break;
	default:
			top = nosave = 0;
	}
	procp = initframe();
	if (frameflg) {		/*  no initial frame */
		if (pid == 0 && (fcor < 0 || fakecor))	{  /* usual error */
			errflg = "No process and no core file.";
			chkerr();
		}
		else {				/* unusual initial frame */
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("prfrx");
				endofline();
			}
#endif
			nosave = 0;
			return;
		}
	}
	do {
#if u3b || u3b5 || u3b15 || u3b2

		argpl = argp;	/*  argp for the current frame */

		/*  3B crt0 (start) has an old fp of zero */
		if (frame < USRSTACK)
			broken = 1;
		else if (get(NEXTFRAME, DSP) == 0)
		{
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("prfrx");
				endofline();
			}
#endif
			nosave = 0;
			return;
		}
#else
#if vax
		/*  VAX crt0 (start) gets a current fp of zero */
		if (frame == 0)
		{
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("prfrx");
				endofline();
			}
#endif
			nosave = 0;
			return;
		}
#endif
#endif
		p = procp->pname;
		if (eqstr("__dbsubc", p))	/*  3B ?? */
		{
#if DEBUG
			if (debugflag == 1)
			{
				exit1();
			}
			else if (debugflag == 2)
			{
				exit2("prfrx");
				endofline();
			}
#endif
			nosave = 0;
			return;
		}
		if (procp == badproc) {		/*  if stripped a.out */
			printf("pc: 0x%lx;	args: (", callpc);
			endflg = 1;
		}
#if vax
		else if (p[0] == '_') {
			printf("%s(", p+1);
			endflg = 0;
		}
#endif
		else {
			if (nshlib) 
				prpname(procp,"(");
			else
				printf("%s(", p);
			endflg = 0;
		}
		if (endflg == 0) {
			offs = procp->st_offset;
			libn = procp->lib;
			do {		/*  in COFF, always just 1 ? */
				if( (offs = rdsym(offs)) == ERROR) {
					endflg++;
					break;
				}
			} while (ISFCN(syp->n_type));
			class = syp->n_sclass;
			while (! ISARGV(class)) {
				if ((offs = rdsym(offs)) == ERROR) {
					endflg++;
					break;
				}
				class = syp->n_sclass;
				if (ISFCN(syp->n_type) || 
						eqstr(syp ->n_nptr,".ef")) {
					endflg++;
					break;
				}
			}

		}

#if u3b | u3b2 | u3b5 || u3b15
		if (frame <= argp)     /* if fp wasn't saved in current frame */
		{
			if (broken)
			{
				narg = 0;
			}
			else if (epargp) {	/* Fortran entry point */
				argpl = epargp;
				narg = NARGSTACK(epargp);
			}
			else
				narg = ((topoffrm - argp) / WORDSIZE) - 2;
		}
		else {
			if (savrflag) 
				narg = 0;
			else
#endif
				narg = NARGSTACK(argp);
#if u3b | u3b2 | u3b5 || u3b15
		}
#endif
		while (narg) {
			if (endflg) {
				val = get(argpl,DSP);
				if (val > 9 || val < 0)
					printf("0x");
				printf("%x", val);
				argpl += WORDSIZE;
			} else {
				int length;
				if ((syp->n_type == T_STRUCT) ||
				    (syp->n_type == T_UNION))   {
				    /* print address of structure
				     * (so that structures of, e.g.
				     * 100 element arrays, are not dumped)
				     */
#if u3b || u3b5 || u3b15 || u3b2
				    printf( "&%s=", syp->n_nptr );
#else
#if vax
				    /* VAX: skip leading underscore */
				    if (syp->n_nptr[0] == '_')
					printf("&%s=", syp->n_nptr+1);
				    else
					printf("&%s=", syp->n_nptr);
#endif
#endif
				    dispx(argpl, "x", C_EXT, 
						(short) (-1), DSP);
				    length = axp->x_sym.x_misc.x_lnsz.x_size;
				}
				else {
				    if (syp->n_type == T_ENUM)
					length =
					    axp->x_sym.x_misc.x_lnsz.x_size;
				    else
					length = typetosize(syp->n_type, 0);
#if u3b || u3b5 || u3b15 || u3b2
				    printf("%s=", syp->n_nptr);
				    /* The address of a short or char is
				     * expected to be the left byte of the
				     * variable.  However, argp points to
				     * the left end of the WORD containing
				     * the parameter, which is right justified.
				     * The address is thus adjusted accordingly.
				     * length<WORDSIZE test needed, since length
				     * could be sizeof(double) > WORDSIZE.
				     *
				     * Alternate code:
				     *  dispx(argp, "", C_REGPARAM,
				     *	    (short)syp->n_type, 0, DSP);
				     * This has the same effect, since dispx
				     * sees a "register" variable (hence
				     * right justified), on the stack, and
				     * extracts the right part.  This is
				     * more efficient, but less clean, since
				     * parameters are not necessarily
				     * register variables, they only look that
				     * way.
				     */
				    if (length < WORDSIZE) {
					dispx(argpl+WORDSIZE-length,"",
					    C_EXT,(short)syp->n_type,DSP);
				    } else {
					dispx(argpl, "", C_EXT,
					    (short)syp->n_type, DSP);
				    }
#else
#if vax
				    /* VAX: skip leading underscore */
				    if (syp->n_nptr[0] == '_')
				    {
					printf( "%s=", syp->n_nptr + 1 );
				    }
				    else
				    {
					printf( "%s=", syp->n_nptr );
				    }
				    dispx(argpl, "", C_EXT, 
					    (short) syp->n_type, DSP);
#endif
#endif
				}
				if (length > WORDSIZE) {
					argpl += length;
					/*  adjust for more than 1 word */
					narg -= length/WORDSIZE -1;
				}
				/* bytes, shorts, longs pushed as ints */
				else {
					argpl += WORDSIZE;
				}
			}
			do {
				if (endflg) break;
				if ((offs = rdsym(offs)) == ERROR) {
					endflg++;
					break;
				}
				class = syp->n_sclass;
				if (ISFCN(syp->n_type)|| 
					eqstr(syp ->n_nptr,".ef")) {
					endflg++;
					break;
				}
			} while (! ISARGV(class));
			if (--narg != 0) printf(",");
		}
		if (broken)
			printf("...");
		printf(")");
		if (procp->sfptr != badfile)
			printf("   [%s",
				(procp->sfptr)->sfilename);
		if(gflag) {
			if ((line_num = adrtolineno(callpc-subsproc,procp)) > 0)
				printf(":%d", line_num);
		}
		if(procp->sfptr != badfile)
			printf("]");
		printf("\n");
		if (broken)	/* top frame has no fp, SAVE was not executed */
		{
		  printf("Broken stack frame \n");
		  break;
		}
		subsproc = 1;
		procp = nextframe();
		libn = procp->lib;
	} while (!top && !frameflg );	/*  only one frame desired, or
						no frames left in backtrace */
/* Vax:	} while (((procp = nextframe()) != badproc) && !top);*/
	nosave = 0;
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("prfrx");
		endofline();
	}
#endif
}


/* machine dependent initialization */
sdbenter() {
#if vax
	mkioptab();
#endif
}

/* machine dependent exit code */
sdbexit() {
	exit(0);
}

#if u3b || u3b5 || u3b15 || u3b2
/*  isubcall() to replace SUBCALL #define (more complicated on 3B) */
isubcall(loc, space)
long loc;
int space;
{
	register int opcode;
	union word word;

	word.w = get(loc,space);
	opcode = word.c[0];
#if u3b
	return(opcode == 0x79 || opcode == 0x78 || opcode == 0x77);
#else
	return(opcode == 0x2c);		/* opcode for call instruction */
#endif
}
#endif

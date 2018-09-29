/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:com/access.c	1.18"

/*
 *	UNIX debugger
 */

#include "head.h"
#define STAR	4
#define ISDSP(X)	((datmap.b1 <= X && X < datmap.e1) || \
			 (datmap.b2 <= X && X < datmap.e2) || \
			 (txtmap.b2 <= X && X < txtmap.e2))
#define ISISP(X)	(txtmap.b1 <= X && X < txtmap.e1)
/*
** #define	ISDSP(X)	( ( datmap.b1 <= X && X < datmap.e1 ) || \
**	 			  ( datmap.b2 <= X && X < datmap.e2 ) )
** #define	ISISP(X)	( ( txtmap.b1 <= X && X < txtmap.e1 ) || \
**				  ( txtmap.b2 <= X && X < txtmap.e2 ) )
*/
#define WITHIN(adr, lbd, ubd) ((adr) >= (lbd) && (adr) < (ubd))

extern int wtflag;	

extern MSG		BADDAT;
extern MSG		BADTXT;
extern MAP		txtmap;
extern MAP		datmap;
extern STRING		errflg;
extern int		errno;

extern INT		pid;


/* file handling and access routines */

/*
 * getval and putval can be speeded up
 * by checking to see if only one word has to be access'd
 * (which is always the case in the VAX, and in the 3B for d = 'b' or 'c')
 * and by calling access() themselves
 */

/* get data at loc using descriptor format d */
/* value is returned in a long int, right justified.
 *
 * getval should be called only when !ISREGV(sl_class), because on the
 * 3B, register variables are right justified, while all other data is
 * left justified.  A register image on the stack is still right justified,
 * and getreg() will retrieve it.
 */
long
getval(loc, d, space)
ADDR loc;
char *d;
{
	register int ln,xxx;
	register union word word;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("getval");
	}
	else if (debugflag == 2)
	{
		enter2("getval");
		arg("loc");
		printf("0x%x",loc);
		comma();
		arg("d");
		printf("0x%x",d);
		comma();
		arg("space");
		printf("0x%x",space);
		closeparen();
	}
#endif
	if((ln = dtol(*d)) > WORDSIZE) {
		fprintf(FPRT1, "Error: getval(): ln=%d\n", ln);
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("getval");
			printf("0x%x",-1);
			endofline();
		}
#endif
		return(-1);
	}

	/*  This seemingly machine independent code is highly
	 *	machine dependent, and does the following things:
	 *
	 *	3B -
	 *		chars are unsigned, and stored in the first
	 *		byte of a word (left end).  The union "word"
	 *		gets this byte, and moves it to a long.
	 *
	 *		shorts may be signed or unsigned.  They are
	 *		stored in the first two bytes of a word
	 *		(left end).  The union "word" gets the
	 *		value, and by using us (unsigned short) or
	 *		s (signed short) does sign extension as necessary.
	 *
	 *		ints are full words, so sign extension is
	 *		meaningless when moving bits (no bits to pad).
	 *
	 *	VAX -
	 *		chars may be signed or unsigned, and are stored
	 *		in the first byte of a word (right end).  The
	 *		union "word" gets this byte, and sign extends
	 *		in the case of a signed char, by using the member "c".
	 *		Sign extension is not done when cu is referenced.
	 *
	 *		shorts may be signed or unsigned.  They are stored
	 *		in the first two bytes of a word (right end).
	 *		They are handled as in the 3B case.
	 *
	 *		ints - see 3B.
	 */
	word.w = get(loc, space);
	/*  d[1] is either '\0' in the case of a one character
	 *	descriptor string, or 'u' in the case of "su"
	 *	(short unsigned), "lu" (long unsigned), or
	 *	"cu" (character unsigned).
	 */
	switch(ln) {
	case 1:
		if (d[1])			/* character unsigned */
			xxx =word.cu[0];
		else		/* character (VAX - signed; 3B - unsigned) */
			xxx =word.c[0];
		break;

	case 2:
		if (d[1])			/* short unsigned */
			xxx =word.su[0];
		else				/* short, signed */
			xxx =word.s[0];
		break;

	case 4:
		/* bits are bits.  Return the full word. */
		xxx =word.w;
		break;

	default:	/*  0??, 3??? */
		xxx =0;
		break;
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("getval");
		printf("0x%x",xxx);
		endofline();
	}
#endif
	return xxx;

}

/* put value at loc using descriptor format d */
/* the value is a right justified, signed int, so only the byte size
 * of the target is needed.  Signed vs. unsigned is irrelevant - see K&R
 * page 41.
 */
putval(loc, d, value)
ADDR loc;
char *d;
long value;
{
	union word word;
	int idsp;			
	register int ln;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("putval");
	}
	else if (debugflag == 2)
	{
		enter2("putval");
		arg("loc");
		printf("0x%x",loc);
		comma();
		arg("d");
		printf("0x%x",d);
		comma();
		arg("value");
		printf("0x%x",value);
		closeparen();
	}
#endif
	if((ln = dtol(*d)) > WORDSIZE) {
		fprintf(FPRT1, "Error: putval(): ln=%d\n", ln);
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("putval");
			printf("0x%x",-1);
			endofline();
		}
#endif
		return(-1);
	}
	idsp = DSP;
	if(cmd == '!' && ISISP(loc) && !ISDSP(loc)) {	
		idsp = ISP;	/* on 3B and VAX, I and D addresses distinct */
		if (!wtflag && pid) {
			errflg = "Cannot write I space.";
			chkerr();
		}
	}
	/* See comments in getval (above) about byte ordering. */
	word.l = get(loc,idsp);
	switch(ln) {
		case 1:
			word.c[0] = (unsigned char)value;
			break;
		case 2:
			word.s[0] = (unsigned short)value;
			break;
		default:
			word.l = value;
			break;
	}
	put(loc, idsp, word.l);
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("putval");
		printf("0x%x",0);
		endofline();
	}
#endif
	return(0);
}

/*	get value from named register (r0 is 0) using descriptor format d */
/*	getreg may be called to get a value that is in a register or in
 *	the image of a register on the stack.  ISREGN(reg) is used to
 *	determine whether the address given is a register.  This routine
 *	is important for the 3B, where register data is right justified,
 *	while all other data is left justified.
 *
 *	call getreg if ISREGV(sl_class).
 */
long
getreg(reg, d)
ADDR reg;
char *d;
{
	register long part;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("getreg");
	}
	else if (debugflag == 2)
	{
		enter2("getreg");
		arg("reg");
		printf("0x%x",reg);
		comma();
		arg("d");
		printf("0x%x",d);
		closeparen();
	}
#endif
	if (ISREGN(reg)) {
		/*  if less than word, return righmost part */
#if vax || u3b
		part = getpart(SDBREG(reglist[reg].roffs),d);
#else
#if u3b5 || u3b15 || u3b2
		part = getpart(regvals[reg],d);
#endif
#endif
	}
	else
	{
		part = getpart(get(reg, DSP),d);
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("getreg");
		printf("0x%x",part);
		endofline();
	}
#endif
	return part;
}

/* put value in named register (r0% is 0) using descriptor format d */
/* if !ISREGN(reg), then the "register" is really a register image
 * on the stack, and this is written.  putreg is still called in
 * this case, because this routine is designed to handle the writing
 * of all register images, whether they be in registers or on the stack
 * (important in the 3B, where registers are right justified, all else
 * left justified).
 *
 * Call putreg if ISREGV(sl_class).
 */
putreg(reg, d, value)
ADDR reg;
char *d;
long value;
{
	register long val;
	register int ptv;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("putreg");
	}
	else if (debugflag == 2)
	{
		enter2("putreg");
		arg("reg");
		printf("0x%x",reg);
		comma();
		arg("d");
		printf("0x%x",d);
		comma();
		arg("value");
		printf("0x%x",value);
		closeparen();
	}
#endif
	val = getpart(value,d);		/* if less than word, right part */
#if vax || u3b
	if (ISREGN(reg)) SDBREG(reglist[reg].roffs) = val;
#endif

	if( pid != 0 ) {	/* added */
		errno = 0;
#if vax || u3b
		if (ISREGN(reg)) {
		    ptv = ptrace(WUREGS, pid, SYSREG(reglist[reg].roffs), val);
		    if(ptv == -1 && errno)
			fprintf(FPRT1,
			    "Warn: putreg(reg=%d) could not ptrace;\n", reg);
		}
		else {		/* added, for registers on stack */
		    put(reg, DSP, value);
		}
#else
#if u3b5 || u3b15 || u3b2
		{
		int *uar0;

		uar0 = (int *)ptrace(RUREGS,pid,(int)&(((struct user *)0)->u_ar0),0);
		if ((int)uar0 == -1 || errno)
			fprintf(FPRT1,"putreg(): could not read u_ar0\n");
		uar0 += reglist[reg].roffs;
		ptv = ptrace(WUREGS,pid,(int)uar0,value);
		if (ptv == -1 || errno)
			fprintf(FPRT1,"putreg(): could not write %s\n",reglist[reg].rname);
		regvals[reg] = value;
		}
#endif
#endif
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("putreg");
			endofline();
		}
#endif
	}

	/*  registers in uarea on core file do not get updated ?! */
}

/* for 3B:	new get() and put()
 *	when pid != 0 access() calls ptrace()
 *	and ptrace() can only read/write on word boundaries;
 *	3B instructions can begin on half-words.
 *	Now can access WORDSIZE bytes beginning at any address !
 *
 * for VAX: get() and put() are simply filters which can handle
 *	machine dependent manipulations before calling access().
 *	None are needed on the VAX, so these functions are trivial on
 *	that machine.
 */

/* get (adr, space) - simply returns the value of a word starting
 *		at adr.  If there is a left justified value, it
 *		is left for the calling routine (getval) to extract it.
 */
POS
get(adr, space)
L_INT	adr;
{
	int xxx;
#if vax
#if DEBUG
	if (debugflag ==1)
	{
		enter1("get");
	}
	else if (debugflag == 2)
	{
		enter2("get");
		arg("adr");
		printf("0x%x",adr);
		comma();
		arg("space");
		printf("0x%x",space);
		closeparen();
	}
#endif
	xxx = access(RD, adr, space, 0);
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("get");
			printf("0x%x",xxx);
			endofline();
		}
#endif
	return xxx;
#else
	register int bn, i;
	register long wa;
	union word rval;
	union dbl dblw;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("get");
	}
	else if (debugflag == 2)
	{
		enter2("get");
		arg("adr");
		printf("0x%x",adr);
		comma();
		arg("space");
		printf("0x%x",space);
		closeparen();
	}
#endif
	bn = adr & (WORDSIZE-1);		/* byte number */
	if(pid == 0 || bn == 0)
	{
		xxx = access(RD,adr,space,0);		/* old way */
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("get");
			printf("0x%x",xxx);
			endofline();
		}
#endif
		return xxx;
	}
	wa = adr & ~(WORDSIZE-1);		/* word aligned loc */
	dblw.ww.w1 = access(RD,wa,space,0);
	dblw.ww.w2 = access(RD,wa+WORDSIZE,space,0);
	for(i=0; i<WORDSIZE; rval.c[i++] = dblw.cc[bn++])
		;
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("get");
		printf("0x%x",rval.w);
		endofline();
	}
#endif
	return(rval.w);
#endif
}

/* put(adr, space, value) - simply puts the value into the word starting
 *			at address adr.  All shifting, masking, or'ing,
 *			etc. must have already been done.
 */
put(adr,space,value)
L_INT	adr;
{
	register int bn, i;
	register long wa;
	union word val;
	union dbl dblw;
#if DEBUG
	if (debugflag ==1)
	{
		enter1("put");
	}
	else if (debugflag == 2)
	{
		enter2("put");
		arg("adr");
		printf("0x%x",adr);
		comma();
		arg("space");
		printf("0x%x",space);
		comma();
		arg("value");
		printf("0x%x",value);
		closeparen();
	}
#endif
#if vax
	access(WT, adr, space, value);
#else

	bn = adr & (WORDSIZE-1);		/* byte number */
	if(pid == 0 || bn == 0)
		access(WT,adr,space,value);	/* old way */
	else {
		wa = adr & ~(WORDSIZE-1);	/* word aligned loc */
		dblw.ww.w1 = access(RD,wa,space,0);
		dblw.ww.w2 = access(RD,wa+WORDSIZE,space,0);
		val.w = value;
		for(i=0; i<WORDSIZE; dblw.cc[bn++] = val.c[i++])
			;
		access(WT,wa,space,dblw.ww.w1);
		access(WT,wa+WORDSIZE,space,dblw.ww.w2);
	}
#endif
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("put");
		endofline();
	}
#endif
}

static
access(mode,adr,space,value)
L_INT	adr;
{
	REG INT	pmode, rd, file;
	INT	w;	/*  ADDR --> INT; sizeof same on 3B */
	register long sadr = adr;


	if (space == NSP) return(0);
#if DEBUG
	if (debugflag ==1)
	{
		enter1("access");
	}
	else if (debugflag == 2)
	{
		enter2("access");
		arg("mode");
		printf("0x%x",mode);
		comma();
		arg("adr");
		printf("0x%x",adr);
		comma();
		arg("space");
		printf("0x%x",space);
		comma();
		arg("value");
		printf("0x%x",value);
		closeparen();
	}
#endif

	rd = mode==RD;
	errno = 0;
	errflg = (STRING)0;

	if (pid) {		/* tracing on ? */
	     pmode = (space&DSP ? (rd?RDUSER:WDUSER) : (rd?RIUSER:WIUSER));
	     w = ptrace(pmode, pid, adr, value);
	     if (errno && (w == -1)) {
	     	  errflg = (space&DSP ? BADDAT : BADTXT);
		  chkerr();	/*  omit error checking elsewhere now ?? */
	     }
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("access");
			printf("0x%x",w);
			endofline();
		}
#endif
	     return(w);
	}

	/* if address in a.out, write a.out, unless 'core' file is */ 
	/*	really system file					*/
	if (!rd && cmd == '!' && space == DSP && chkmap(&adr,ISP)) {
		if (datmap.ufd < 0) space = ISP;	/* no core */
#if !(u3b5 || u3b15 || u3b2)
		else {
			extern int magic;
			if (magic == ((struct user *)uu)->u_exdata.ux_mag)
				space = ISP;	/*  really core file */
		}
#endif
	}
	file = (space&DSP ? datmap.ufd : txtmap.ufd);
	adr = sadr;		/* chkmap() overwrites adr */
	if   (!chkmap(&adr,space)) {
		errflg = file > 0 ?		
			(space&DSP ? BADDAT : BADTXT) :
			("No process and/or no file");
		chkerr();	
	}
	w = 0;

	/* need -w flag to write a.out(core ?);  added error message(s) */
	/*  rearranged code to perform checks before writing file	     */
	if(file < 0) {
		errflg = "No process and/or no file.";
	}
	else if (!rd && !wtflag) {
		errflg = "Need -w flag to write a file.";
	}
	else {
		int badflag = 0;
		if (space & DSP) {
			if ((lseek(file, (long)adr, 0) == -1) ||
			    ((rd ?
				read(file,&w,sizeof(w)) :
				write(file,&value,sizeof(w))) < sizeof(w))
			)
				badflag = 1;
		}
		else {	/* ISP */
			extern long curoffs;	/* defined in symt.c */
			if (adr != curoffs &&
				(blseek(&sbuf, (long)adr, 0) == -1))
				badflag = 1;
			else if ((rd ?
				bread(&sbuf,&w,sizeof(w)) :
				write(file,&value,sizeof(w))) < sizeof(w))
					badflag = 1;
			if (badflag || !rd) curoffs = ERROR-1;
			else curoffs = (long)adr + sizeof(w);
		}
		if (badflag) {
			fprintf(FPRT1, "Cannot %s file '%s` [%s].\n",
				rd ? "read" : "write",
				space & DSP ? corfil : symfil,
				(space&DSP ? BADDAT : BADTXT));
			if(!rd && (space & DSP))
				fprintf(FPRT1,
				"Must be stopped at a breakpoint.\n");
		}
	}
	chkerr();	
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("access");
		printf("0x%x",w);
		endofline();
	}
#endif
	return(w);
}

static
chkmap(adr,space)
	REG L_INT	*adr;
	REG INT		space;
{
	REG MAPPTR amap;
#if DEBUG
	if (debugflag ==1)
	{
		enter1("chkmap");
	}
	else if (debugflag == 2)
	{
		enter2("chkmap");
		arg("*adr");
		printf("0x%x",*adr);
		comma();
		arg("space");
		printf("0x%x",space);
		closeparen();
	}
#endif
	amap=((space&DSP?&datmap:&txtmap));
#if vax || u3b
	if ((space & STAR) || (!WITHIN(*adr, amap->b1, amap->e1))) {
#else
#if u3b5 || u3b15 || u3b2
	if (!WITHIN(*adr,amap->b1,amap->e1)) {
#endif
#endif
		if (WITHIN(*adr, amap->b2, amap->e2)) {
			*adr += (amap->f2) - (amap->b2);
		}
		else {
			return(0);	/* errflg set in access() */
		}
	}
	else {
		*adr += (amap->f1) - (amap->b1);
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("chkmap");
		printf("0x%x (*adr == %#x)",1,*adr);
		endofline();
	}
#endif
	return(1);
}

POS
chkget(n, space)
L_INT		n;
{
	REG L_INT	w;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("chkget");
	}
	else if (debugflag == 2)
	{
		enter2("chkget");
		arg("n");
		printf("0x%x",n);
		comma();
		arg("space");
		printf("0x%x",space);
		closeparen();
	}
#endif
	w = get(n, space);
	chkerr();
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("chkget");
		printf("0x%x",w);
		endofline();
	}
#endif
	return(w);
}

/*  getpart() -- get righmost char or short */
long
getpart(value, d)
long value;
char *d;
{
	register long val;
#if DEBUG
	if (debugflag ==1)
	{
		enter1("getpart");
	}
	else if (debugflag == 2)
	{
		enter2("getpart");
		arg("value");
		printf("0x%x",value);
		comma();
		arg("d");
		printf("0x%x",d);
		closeparen();
	}
#endif
	switch(dtol(*d)) {	/*  when > char or short, do as C */
		case 1:
			if (d[1])			/* unsigned char */
			    val = (unsigned char) value;
			else				/* signed char */
			    val = (char) value;
			break;
		case 2:
			if (d[1])			/* unsigned short */
			    val = (unsigned short) value;
			else				/* signed short */
			    val = (short) value;
			break;
		default:
			val = value;		/* full word - bits are bits */
			break;
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("getpart");
		printf("0x%x",val);
		endofline();
	}
#endif
	return(val);
}

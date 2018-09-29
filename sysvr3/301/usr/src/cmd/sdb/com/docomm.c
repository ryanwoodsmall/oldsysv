/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:com/docomm.c	1.16"

#include "head.h"
#include "coff.h"

docommand()
{
	register	char	*p;
	char			*s;
	register	int	i;
	register	ADDR	addr, bkaddr;
	struct		proct 	*procp;
	int			cnt;
	char			*dsc;
	int	asgnflag;	/* 1 if '!' (assignment) operation to address */
	
#if DEBUG
	if (debugflag ==1)
	{
		enter1("docommand");
	}
	else if (debugflag == 2)
	{
		enter2("docommand");
		closeparen();
	}
#endif
	cntval = 1;
	adrflg = 0;
	errflg = 0;

	if (scallf)
	{
		doscall();
		setcur(1);
		lastcom = NOCOM;
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("docommand");
			endofline();
		}
#endif
		return;
	}
	
	if (reflag)		/* search for regular expression */
	{
		dore();
		lastcom = PRCOM;
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("docommand");
			endofline();
		}
#endif
		return;
	}
	
	if (cmd == '\0')
	{
		if (integ != 0 && var[0] != '\0')
		{
			error("Invalid command (1)");
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("docommand");
			endofline();
		}
#endif
			return;
		}
		if (integ != 0)		/* print line number */
		{
			ffind((int) integ);
			fprint();
			lastcom = PRCOM;
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("docommand");
			endofline();
		}
#endif
			return;
		}
		if (var[0] != 0)
		{
			fprintf(FPRT1, "Unexpected null command\n");
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("docommand");
			endofline();
		}
#endif
			return;
		}
	}
		
	switch (cmd)
	{
	case '<': /* input commands from file */
		{
		    FILE *savefile = inputfile;
		    if ((inputfile = fopen(args, "r")) == NULL)
		    {
			fprintf(FPRT1, "Cannot open file %s\n", args);
			inputfile = savefile;
		    }
		    else
		    {
			if (savefile != stdin) fclose(savefile);
			metabase = SCRIPTEXP;
		    }
		    lastcom = NOCOM;
		}
		break;
	case 'v':		/* #v: toggle verbose mode */
		if(vmode && !integ)
		{
			vmode = 0;
		}
		else
		{
			vmode = integ +1;
		}
		break;
	
	case 'Y':
		debug = !debug;
		debugflag = integ;	/* #Y turns on debugging level # */
		break;

	case 'V':
		version();
		break;

	case 'M':
		if (args[0])
		{
			setmap(args);
		}
		else
		{
			printmap("? map", &txtmap);
			printmap("/ map", &datmap);
		}
		break;

	case 'x':
		printregs();
		break;

	case 'X':
		printpc();
		break;

	case 'a':
		if (integ)
		{
			cpstr(args, "l\n");
		}
		else
		{
			if (proc[0])
			{
				cpall(args, "T\n");
			}
			else
			{
				error("Bad arguments");
				break;
			}
		}
		goto setbrk;
		break;	

	case 'l':
		setcur(1);
		lastcom = NOCOM;
		break;
		
	case 'T':
		prfrx(1);
		lastcom = NOCOM;
		break;
		
	case 't':
		lastcom = NOCOM;
		prframe();
		break;
		
	case 'e':
		p = args;
		if (*p == '\0')
		{
			char *name;
			name = curproc()->pname;
#if u3b || u3b5 || u3b15 || u3b2
			printf("%s() in \"%s\"\n", name, curfile);
#else
#if vax
			if (*name == '_')
			{
			     printf("%s() in \"%s\"\n", name+1, curfile);
			}
			else
			{
				printf("%s() in \"%s\"\n", name, curfile);
			}
#endif
#endif
			break;
		}
			/* s --> 2nd arg, else NULL */
		for (s = (char *)strchr(p,' '); *s == ' '; s++)
			;
		if (s != NULL || p[strlen(p) - 1] == '/') /* new directory */
		{
			char save[80], *q;
			if (s == NULL)		/* change base directory */
			{
				strcpy(save,fwp);
				s = &save[0];
			}
			q = filework;
			strcat(sdbpath,":");
			strcat(sdbpath,p);
			sdbpath[strlen(sdbpath)-1] = (char) 0;
			while (*p != ' ' && *p != '\0')	/* copy new directory */
			{
				*q++ = *p++;
			}
			*q++ = '/';		/* extra '/' does not hurt */
			*q = '\0';
			fwp = q;
			finit(s);
			printf("\"\[%.*s\]%s\"\n", fwp-filework, filework, fwp);
			lastcom = PRCOM;
			break;
		}
		
		else if (eqany('.', p))		/* simple filename */
		{
			finit(args);
			printf("\"%s\"\n", curfile);
			lastcom = PRCOM;
			break;
		}
		/* argument is procedure name */
		procp = findproc(args);
		if ((procp->pname[0] != '\0') && (procp->sfptr != badfile))
		{
			finit(procp->sfptr->sfilename);
			ffind((int) procp->lineno);
		}
		else
		{
			fprintf(FPRT1, "Cannot find %s\n", args);
		}
		{
			char *name;
			name = curproc()->pname;
#if u3b || u3b5 || u3b15 || u3b2
			printf("%s() in \"%s\"\n", name, curfile);
#else
#if vax
			if (*name == '_')
			{
			     printf("%s() in \"%s\"\n", name+1, curfile);
			}
			else
			{
				printf("%s() in \"%s\"\n", name, curfile);
			}
#endif
#endif
		}
		lastcom = PRCOM;
		break;
		
	case 'p':
		if (integ) ffind((int) integ)
			;
		fprint();
		lastcom = PRCOM;
		break;
		
	case 'q':
		endpcs();
		sdbexit();
		
	case 'w':
		if (integ)
		{
			ffind( ( int ) integ );
		}
		i = curstmt.lnno;
		fback( WINDOW / 2 );
		fprintn( WINDOW );
		ffind( ( int ) i );
		lastcom = PRCOM;
		break;
		
	case 'Q':
		fprintf(stderr,"curfile='%s' filework='%s'\n",curfile,filework);
		prdebug();
		break;

	case 'z':
		if (integ)
		{
			ffind( ( int ) integ );
		}
		fprintn( WINDOW );
		lastcom = PRCOM;
		break;

	case '-':
		fback(integ ? (int) integ : 1);
		fpargs();
		lastcom = PRCOM;
		break;

	case '+':
		fforward( integ ? (int) integ : 1 );
		fpargs();
		lastcom = PRCOM;
		break;

	case '\n':
		switch (lastcom)
		{
		case PRCOM:
			fforward(1);
			fprint();
			break;
		case DSCOM:
			oaddr += oincr ? oincr : typetosize(otype, WORDSIZE);
			printf("0x%lx/ ", oaddr);
			dispf((ADDR) oaddr, odesc,
			    ISREGV(oclass) ? oclass : C_EXT, otype, 0);
			break;
		case DSICOM:
			dot += oincr;
			prisploc(2);
			dispi(dot, odesc, C_EXT, (short)0, 0);
			break;
		}
		break;

	case '\004':
		if (!isatty(0))
			sdbexit();
		switch (lastcom)
		{
		case PRCOM:
			fforward(1);
			printf("\r");
			fprintn(WINDOW);
			lastcom = PRCOM;
			break;
		case DSICOM:
			printf("\r");
			for (i=0; i<WINDOW; i++)
			{
				dot += oincr;
				prisploc(2);
				if ( dispi( dot, odesc, C_EXT, ( short ) 0, 0 )
				    == -1)
				{
					break;
				}
			}
			break;
		case DSCOM:
			printf("\r");
			for (i=0; i<WINDOW; i++)
			{
				oaddr += oincr ?
					oincr : typetosize(otype, WORDSIZE);
				printf("%#lx/ ", oaddr);
				if ( dispf( ( ADDR ) oaddr, odesc,
					ISREGV( oclass ) ? oclass :
					C_EXT, otype, 0) == -1)
				{
					break;
				}
			}
			break;
		default:
			printf("\n");
		}
		break;

	case 'r':
		if (args[0] == '\0')
		{
			getargs();
		}
	case 'R':
		signo = 0;
		cpstr(oldargs, args);
		if (integ) cntval = integ;
		if (executing == FALSE)
		{
			executing = TRUE;
			if (integ)
			{
				cntval = integ;
			}
			dopcs('r');
			executing = FALSE;
		}
		bkaddr = -1;
		goto f1;

	case 'c':
		signo = 0;
	case 'C':
		if (proc[0] != '\0' || integ != 0)
		{
			dot = setdot(BKOFFSET, ALTBKOFFSET);
			if (dot == -1)
			{
				error("Cannot set temporary breakpoint");
				break;
			}
			dopcs('b');
			bkaddr = dot;
		}
		else
		{
			bkaddr = -1;
		}
		integ = atol(args);

f1:

		if (integ)
		{
			cntval = integ;
		}
		if (dopcs('c') == -1)
		{
			break;
		}
		if (bkaddr != -1)
		{
			ADDR dotsave;
			dotsave = dot;
			dot = bkaddr;
			dopcs('d');
			dot = dotsave;
		}
		if (!signo)
		{
			printf("Breakpoint");
		}
		printf(" at\n");
		setcur(1);
		lastcom = NOCOM;
		break;
		
	case 'i':
		signo = 0;
	case 'I':
/*		integ = atol(args);	removed */
		if (integ)
		{
			cntval = integ;
		}
		dopcs('s');
		if (signo)
		{
			printf("\n");
		}
		setcur(0);
		printpc();
		lastcom = NOCOM;
		break;

	case 'S':
	case 's':
		signo = 0;
/*		integ = atol(args);	removed */
		singstep(integ ? (int) integ : 1, cmd);
		if (signo)
		{
			printf("\n");
		}
		if( pid > 0 )
		{
			setcur(1);
		}
		lastcom = NOCOM;
		break;
		
	case 'g':
		if (pid == 0  ||  signo)
		{
			error("Not stopped at breakpoint");
			break;
		}
		dot = setdot(BKOFFSET, ALTBKOFFSET);
		if (dot == -1)
		{
			error("Bad address");
			break;
		}
		adrflg = 1;
		integ = atol(args);
		if (integ)
		{
			cntval = integ;
		}
		dopcs('c');
		if (!signo)
		{
			printf("Breakpoint");
		}
		printf(" at\n");
		setcur(1);
		adrflg = 0;
		lastcom = NOCOM;
		break;

/*  TEMPORARY ****	put back !! ***** */
	case 'k':
		if (scallx)
		{
	 		userpc = dot = USERPC = pcs;
#if u3b || vax
			SDBREG(FP) = fps;
			SDBREG(AP) = aps;
#else
#if u3b5 || u3b15 || u3b2
			regvals[9] = fps;
			regvals[10] = aps;
#endif
#endif
			if (bkpts)
			{
				bkpts->flag = flagss;
			}
			scallx = 0;
			error("Procedure killed");
			longjmp(env, 0);
		}
		else
		{
			dopcs('k');
			printf("\n");
			lastcom = NOCOM;
			break;
		}

	case 'B':
		prbkpt();
		break;

	case 'b':
	setbrk:
		if (proc[0]=='\0' && integ==0 && ncolonflag==0)
		{    
			integ = curstmt.lnno;
		}
		dot = setdot(BKOFFSET, ALTBKOFFSET);
		/* also fixed setcur() and runpcs() to allow breaks at 0 */
		if (dot == -1 )
		{
			fprintf(FPRT1, "Cannot Set Breakpoint");
		/*	if(dot == 0)
		**		fprintf(FPRT1, "; Try ''%d:b``",
		**				dis_dot((long)0,ISP,'i'));
		*/
			fprintf(FPRT1, "\n");
			break;
		}
		else if (dot == -2)
		{
			fprintf(FPRT1, "Cannot Set Breakpoint on any part of ");
			fprintf(FPRT1, "an inline expanded function.\n");
			break;
		}
		dopcs('b');
		s = " b\n";
		s[1] = cmd;
		printbkpt(s, adrtoprocp(dot), dot);
		break;
		
	case 'd':
		if (proc[0] == '\0' && integ == 0)
		{
			idbkpt();
			break;
		}
		dot = setdot(BKOFFSET, ALTBKOFFSET);
		if (dot == -1)
		{
			error("Non existent breakpoint");
			break;
		}
		dopcs('d');
		break;
		
	case 'D':
		dabkpt();
		error("All breakpoints deleted");
		break;

	case 'm':		/*  made it work -- with many changes */
		signo = 0;
		addr = integ;
		cnt = atol(args);
		if(addr == 0 && var[0] != '\0')
		{
		      addr = varaddr("", var);	/* was proc | curproc() */
		      dsc = typetodesc(sl_stype);
		}
		else
		{
			sl_class = C_EXT;	/* needed by getlocv */
			dsc = "d";
		}
		if(addr != -1)
		{
		    int j;
		    j = getlocv(addr, dsc, DSP);
		    i = monex(addr, dsc, cnt);
		    if (j != i)		/* got a change of value */
		    {
			printf("Prev stmt changed %s %#x ",
				ISREGN(addr) ? "reg" : "loc", addr);
			if(var[0] != '\0')
			{
#if u3b || u3b5 || u3b15 || u3b2
				if(addr >= USRSTACK)
				{
					printf("[%s:%s]", curproc()->pname,
						var);
				}
#else
#if vax
				if ( addr >= 0x70000000 )
				{
					char *name;
					name = curproc()->pname;
					if( *name == '_' )
					{
					   printf("[%s:%s]", name+1, var);
					}
					else
					{
					   printf("[%s:%s]", name, var);
					}
				}
#endif
#endif
				else
				{
					printf("[%s]", var);
				}
			}
			printf(" from %d(%#x) to %d(%#x).\n", j, j, i, i);
		    }
		    /* else stopped because count exceeded or got a signal */
		    else
		    {
			if (cnt > 1 || signo != 0)
			{		/* monex() set cntval before return */
			    printf(
				"Stopped after %d steps; loc %#x unchanged.\n",
				cntval, addr );
			}
		    }
		}
		setcur(1);
		lastcom = NOCOM;
		break;
		
	case '?':
		p = args[0] ? args : "i";
		if(var[0] == '\0')
		{
			dot = setdot(0, 0);
		}
		/* else if (var[0] == '.' && var[1] == '\0') dot = dot */
		else
		{
			dot = findvar( proc, var, "d", 0 );
		}
		if (errflg)
		{
			error(errflg);
			break;
		}
		prisploc(2);
		dispi(dot, p, C_EXT, (short) 0, 0);
		lastcom = DSICOM;
		break;

	case '/':
		if (var[0] == '.' && var[1] == '\0')
		{
			if (integ == 0) integ = oaddr;
			dispf((ADDR) integ, args[0] ? args : odesc,
			    oclass == C_REG ? oclass : C_EXT, otype, 0);
			oaddr = integ;
		}
		else
		{
			if ((integ || ncolonflag) && (var[0] == '\0'))
			{
				dispf((ADDR) integ, args, C_EXT, 0, 0);
				oaddr = integ;
				cpstr(odesc, args);
				oclass = C_EXT;
				otype = 0;
			}
			else
			{
				dispvar(proc, var, args);
			}
		}
		lastcom = DSCOM;
		break;
		
	case '=':
		p = args[0] ? args : "x";
		if (var[0] == '\0')
		{
			if (proc[0])
			{
				addr = setdot(0, 0);
				if (addr == -1)
				{
					error("Unknown address");
					break;
				}
			}
			else
			{
				addr = integ;
			}
			dispf(addr, p, 0, -1, 0);
		}
		else
		{
			if (var[0] == '.' && var[1] == '\0')
			{
				dispi(dot, p, 0, -1, 0);
			}
			else 
			{
				findvar(proc, var, p, 2);
			}
		}
		break;

	case '!':
		if (var[0] == '.' && var[1] == '\0')
		{
			if (integ == 0)
			{
				integ = oaddr;
			}
			oaddr = addr = integ;
			asgnflag = 1;
		}
		else
		{
			if ((integ || ncolonflag) && (var[0] == '\0'))
			{
				oaddr = addr = integ;
				asgnflag = 1;
			}
			else
			{
				asgnflag = 0;
			}
		}

		/* view numeric address as address of int, make assignment */
		if (asgnflag)
		{
			switch(argvalue(args))
			{
			case INTEG:
				putval(addr, "d", rhs_value.val.i);
				break;
			case DBL:
				putval(addr, "d", (int)(rhs_value.val.dd));
				break;
			case FLOAT:
				putval(addr, "d", (int)(rhs_value.val.f));
				break;
			default:	/* error in argvalue */
				break;
			}
		}

		else	/* assign to LHS: call varasgn in display.c */
		{	/* set up global variable rhs_value, with value.
			 * argvalue() < 0 means RHS could not be computed
			 */
			if ((int)argvalue(args) >= 0)
			{
				varasgn(proc, var);
			}
		}

		lastcom = NOCOM;
		break;

	case '"':
		if (!(eqany('\\', args)))
		{
			printf(args);
		}
		else	/* convert escape sequences, copying in place */
		{ 
			char *from, *to;
			to = args;
			for (from = args; *from; from++)
			{
				if (*from == '\\')
				{
					*to++ = bsconv(*(++from));
				}
				else
				{
					*to++ = *from;
				}
			}
			*to = '\0';
			printf(args);
		}
		break;

	case ' ':		/* to catch M? */
		printf("Bad command for %s\n", var);
		break;

	default:	/* added to indicate bad command */
		printf(FPRT2, "Bad sdb command: '%c'=\\%#x\n", cmd,(int)cmd);
		break;
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("docommand");
		endofline();
	}
#endif
}

fpargs() {
	register int i;
	
#if DEBUG
	if (debugflag ==1)
	{
		enter1("fpargs");
	}
	else if (debugflag == 2)
	{
		enter2("fpargs");
		closeparen();
	}
#endif
	switch(args[0]) {
	case 'p':
	case '\0':
		fprint();
		break;
	case 'w':
		i = curstmt.lnno;
		fback(WINDOW/2);
		fprintn(WINDOW);
		ffind((int) i);
		break;
	case 'z':
		fprintn(WINDOW);
		break;
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("fpargs");
		endofline();
	}
#endif
}

extern MSG	BADTXT;

/* Used by =, ?, a, b, c, C, d and g commands to find linenumber */
ADDR
setdot(bkoffset, altbkoffset) {
	REG ADDR loc;		/* dot returned */
	struct proct *procp = badproc;
#if DEBUG
	if (debugflag ==1)
	{
		enter1("setdot");
	}
	else if (debugflag == 2)
	{
		enter2("setdot");
		arg("bkoffset");
		printf("0x%x",bkoffset);
		comma();
		arg("altbkoffset");
		printf("0x%x",altbkoffset);
		closeparen();
	}
#endif
	if (ncolonflag) {
		loc = integ;
		get(loc, ISP);
		if (errflg)
			loc = -1;
	} else {
		struct stmt stmt;
		ADDR addr;
		extern int errlev;		/*  in dis/bits.c */

		stmt.lnno = integ;
		stmt.stno = integ2;

		loc = getaddr(proc, stmt);
		if (loc == -1) {
			errflg = "Bad line number";
			loc = -1;
		}
		else if (loc == -2)
		{
			errflg = "inline expanded function";
			loc = -2;
		}
		else
		{
			procp = adrtoprocp(loc);
			addr = procp->paddress;
			if (procp->notstab) {
				if (loc < addr + altbkoffset)
					loc = addr + altbkoffset;
			} else {
				if (loc < addr + bkoffset)
					loc = addr + bkoffset;
			}
			dis_dot(loc,ISP,'\0');	/* sets errlev if bad loc */
			if(errlev > 0)	/*  not an instruction boundary */
				loc = addr;	/* put back */
		}
	}
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("setdot");
			printf("0x%x",loc);
			endofline();
		}
#endif
	return(loc);
}

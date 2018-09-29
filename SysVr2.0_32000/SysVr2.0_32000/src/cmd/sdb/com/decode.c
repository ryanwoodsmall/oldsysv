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
	/* @(#) decode.c: 1.7 5/17/83 */

#include "head.h"

/* decode() - decode a line read from standard input */

/* It may help in reading this code to view sdb commands as
 *		address,count command options
 *
 *	An address may be a line number, a physical address (number:),
 *	or a variable (proc:var, :var, or var).  If there is no ambiguity
 *	between line number and physical address, the colon following the
 *	physical address is not required.  A line number is actually
 *	a statement number in the form line [, statement].  Since COFF
 *	does not have statement numbers within a line, any statement other
 *	than 1 is an error.  Note integ2 (statement number) is initially 1.
 *
 *	Count is stored in integ, and may be, e.g. the number of breakpoints
 *	to be skipped on a run (r) command.  The only case (so far) in
 *	which both address and count are used is in accessing a variable
 *	on the stack.  In this case, the command is [proc:] var, count command.
 *	Note that this use of the comma does not conflict with the use of a
 *	comma as a delimeter between line and statement numbers, as a
 *	comma following a variable indicates count, while a comma following
 *	a number indicates statement number.  The two variables cntcomma and
 *	commaflag, respectively, keep track of this.
 *
 *	Commands are identified by context, either knowing that what came
 *	before requires a command to follow, or that the character being
 *	considered is a command in the right place, or it is a command
 *	character that cannot be confused with a symbol.
 *
 *	Usually, the options are copied intact, and the particular
 *	commands parse them.  If this parser must scan them, they are
 *	generally expected to be numbers, violating the general format
 *	count command.  For instance, S4 is an sdb command which ought
 *	to be 4S.
 */
decode(p)
char *p; {
	register char c, *q;
	int commaflag = 0;
	int cntcomma = 0;    /* flag for comma delimeter before count field */
	int syst;

	if ( *p == '#' )
	{
		printf("cmd=%c proc=%s var=%s args=%s ", cmd, proc, var, args);
		printf("integ=%d integ2=%d dot=%d\n", integ, integ2, dot);
	}
	integ = scallf = reflag = colonflag = ncolonflag = percentflag = 0;
	integ2 = 1;
	proc[0] = cmd = args[0] = var[0] = '\0';
	if(debug)
	{
		fprintf( stderr, "cmd=%c proc=%s var=%s ", cmd, proc, var );
		fprintf( stderr, "integ=%d integ2=%d\n", integ, integ2 );
	}
	argsp = args;
	
	if (eqany(*p, "/?")) {	/* regular expression */
		c = *p;
		redir = (c == '/');
		reflag = 1;
		p++;
		if (*p == '\n' || *p == c) return(0);
		q = re;
		while(*p != c && *p != '\n') *q++ = *p++;
		*q = '\0';
		return(0);
	}
	
	if (*p == '!')		{ /* shell escape */
		for (q = p; *q != '\n'; q++) ;
		*q = '\0';
		syst = system(p+1);
#if DEBUG > 1
		if(syst && debugflag)
			fprintf(FPRT2,
				"Bad shell command: exit status=%#x;\n", syst);
#endif
		lastcom = NOCOM;
		cmd = '\n';
		return(0);
	}
	
	if (*p == '\n') {
		cmd = '\n';
		return(0);
	}
	
	if (*p == ':') {
		colonflag++;
	}
	
	while (*p != '\n') {	/* decode item by item */
	
		if (commaflag && number(*p)) {	/* decimal number */
			if (integ2 != 1) {
				fprintf(FPRT1,"Too many numbers (%d)\n",integ2);
				return(1);
			}
			integ2 = readint(&p);
			if (*p == ':') {
				ncolonflag++;
				p++;
			}
			continue;
		}

		if (number(*p)) {	/* decimal number */
			if (integ) {
				fprintf(FPRT1, "Too many numbers (%d)\n",integ);
				return(1);
			}
			integ = readint(&p);
			if (*p == ':') {
				ncolonflag++;
				p++;
			}
			continue;
		}
		
		if (*p == ',') {
			commaflag++;
		}
		if (varchar(*p) || eqany(*p, COMMANDS)) { 
					/* proc, variable or command */
			if (cmd != '\0') {
				p = cpall(args, p);
				continue;
			}

			q = p;
			/* find end of variable, number, or command */
			for(;;) {
				while (varchar(*q) || number(*q) ||
				    eqany(*q,SUBCMDS)) q++;
				/* treat ',' inside [] differently, so
				 * use indxchar test */
				if (*q == '[') {
				    q++;
				    while (indxchar(*q)) q++;
				    continue;  /* ']' satisfies varchar */
				}
				break;
			}
			/* '!' command must be treated separately,
			 * because it can be followed by a colon, which
			 * confuses the parser (see statement
			 * "if (*q == ':')" below).
			 * Also, s!var will not parse correctly, since
			 * q-p==1, s is in COMMANDS, and proc[0] == '\0';
			 * so must copy variable name here.
			 */
			if (*q == '!') {	/* variable assignment */
				p = cpname(var, p);
				cmd = '!';
				p = cpall(args, p+1);
				continue;
			}
			if (*q == '(') {	/* procedure call */
				if (proc[0] != '\0') {
				    fprintf(FPRT1,
					"Too many procedure calls (%s)\n",proc);
				    return(1);
				}
				scallf = 1;
				p = cpname(proc, p);
				p = cpall(args, p);
				continue;
			}
			if (*q == ':') { /* procedure name */
				colonflag++;
				p = cpname(proc, p);
				continue;
			}
			if (*q == '$') {	/* variable name */
#if DEBUG > 1
				if(debugflag)
					fprintf(FPRT2, "var=%s; p=%s; q=%s;\n",
							var,p,q);
#endif
				p = cpname(var, p);
				continue;
			}
			if (*q == ',') {	/* count (for stack level) */
						/* count preceeds any cmnd */
				p = cpname(var, p) +1;  /* skip comma */
				cntcomma = 1;
				continue;
			}
			if (((q-p == 1 && eqany(*p,COMMANDS) && 
				(proc[0]=='\0' || eqany(*p, "abcd"))) ||
				((integ || cntcomma) &&  eqany(*p,COMMANDS)) || 
				 eqany(*p, "+-?!<")) 	/* add '!' ?? */
				&& !(*p=='-' && *(p+1) == '>'))
							{  /* command */
				cmd = *p++;
#if DEBUG > 1
				if(debugflag)
				    fprintf(FPRT2, "cmd=%c(%#x); p=%s; q=%s;\n",
							cmd,cmd,p,q);
#endif
                        /* otherwise, it's a variable */

                                /*All SDB commands can be divided into*/
                                /*four sets:                          */
                                /*Those which never come through here:*/
                                /*  Reg.Exp., \n, system, & sing-sub  */
                                /*Those which expect decode() to con- */
                                /*tinue to parse their right args:    */
                                /*  +-ISYipsvwz                       */
                                /*Those which take no right args:     */
                                /*  \004BDQTVXadklqtx                 */
                                /*Those which expect to parse their   */
                                /*right args themselves with decode() */
                                /*setting args equal to the rest of   */
                                /*the command line:                   */
                                /*  CMRbcegmr"?/=!                    */
                                /*Any command in the last two sets can*/
                                /*set args and exit immediately.      */
                                if(eqany( cmd,
                                   "\004\"?!/=BCDMQRTVXabcdegklmqrtx")) {
                                        while(*p == ' ')
                                                p++;
                                        p = cpall(args, p);
                                }
                                continue;
                        }
			if (var[0] != '\0') {
			    fprintf(FPRT1, "Too many variable names(%s)\n",var);
			    return(1);
			}
			p = cpname(var, p);
			if (*p == '%') {
				percentflag++;
				p++;
			}
#if DEBUG > 1
			if(debugflag)
				fprintf(FPRT2, "var=%s; p=%s; q=%s;\n",var,p,q);
#endif
			if (eqstr(var, ".?")) {
				var[1] = '\0';
				cmd = '?';
				continue;
			}
			if (*p == '\n') {
				cmd = '/';
				continue;
			}
			if (cmd == '\0') cmd = *p ? *p : '/';
			p++;
			p = cpall(args,p);
			continue;
		}
		p++;	/* otherwise ignore p */
	}
	return(0);
}

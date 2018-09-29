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
/*	@(#)command.c	1.2	*/
/*
 *  Command decoding 
 */

#include "defs.h"

SCCSID(@(#)command.c	1.2);

static char eqformat[128] = "z";
static char stformat[128] = "X\"= \"^i";
static int  locmsk;

void command(buf, defcom)
    STRING  buf;
    char    defcom;
{
    int    itype;
    int    ptype;
    int    modifier;
    int    regptr;
    int    longpr;
    int    eqcom;
    char   wformat[1];
    char   savc;
    int    w;
    int    savdot;
    STRING savlp = lp;

    if (buf) { 
        if (*buf == EOR)
            return;
        else 
            lp = buf;
    }

    do {
        if (adrflg = expr(0))
            ditto = dot = expv;
        adrval = dot;

        TR1PRINT("Adrflg = %s, ", (adrflg ? "TRUE" : "FALSE"));
        TR1PRINT("adrval = %#x\n", adrval);

        if (rdc() == ',' && expr(0)) {
            cntflg = TRUE;
            cntval = expv;
        }
        else {
            cntflg = FALSE;
            cntval = 1;
            lp--;
        }

        TR1PRINT("Cntflg = %s, ", (cntflg ? "TRUE" : "FALSE"));
        TR1PRINT("cntval = %d\n", cntval);
   
        if ( ! eocmd(rdc()))
            lastcom = lastc;
        else { 
            if (adrflg == 0)
                dot = inkdot(dotinc);
            lp--;
            lastcom = defcom;
        }
   
        TR1PRINT("Type of command: %c\n", lastcom & STRIP);

        switch(lastcom & STRIP) {
   
            case '/':
                itype = DSP;
                ptype = DSYM;
                goto trystar;
   
            case '=':
                itype = NSP;
                ptype = NSYM;
                goto trypr;
   
            case '?':
                itype = ISP;
                ptype = ISYM;
                goto trystar;
   
trystar:
                if (rdc() == '*') {
                    lastcom |= QUOTE; 
                    itype |= STAR;
                }
                else
                    lp--;
   
trypr:
                longpr = FALSE;
                eqcom = lastcom == '=';

                switch (rdc()) {
   
                    case 'm': 
			{ /* Reset map data */
			short  fcount;
			MAP    *smap;
			union {
				MAP *m;
				int *mp;
			} amap;

			if (eqcom)
				error(BADEQ);
			smap = (itype & DSP ? &slshmap : &qstmap);
			amap.m = smap;
			fcount = 3;
			if (itype & STAR)
				amap.mp += 3;
			while (fcount-- && expr(0))
				*(amap.mp)++ = expv;
			if (rdc() == '?')
				smap->ufd = fsym;
			else if (lastc == '/')
				smap->ufd = fcor;
			else
				lp--;
		}
		break;


                case 'l': /* Search for exp */
                    if (eqcom) 
                        error(BADEQ);
                    savdot = dot;
		    expr(0);
                    locval = expv;
		    switch (rdc())
			{
				case 'b':
					locmsk = 0xff;
					dotinc = 1;
					break;
				case 'w':
					locmsk = 0xffff;
					dotinc = 2;
					break;
				case 'l':
					locmsk = -1L;
					dotinc = 4;
					break;
				case 'o':
					locmsk = 0xff;
					for(;;)
					{
						w=get(dot,itype);
						if(errflg||mkfault||locval==
							(w&locmsk))
							break;
						printins(itype,w,0);
						dot=inkdot(dotinc);
					}
					psymoff(dot,ptype,"");
					goto out;
				default:
					lp--;
					if(expr(0))
						locmsk = expv;
					else
						locmsk = -1L;
					dotinc = 1;
					break;
			}
			if(expr(0))
				dotinc = expv;
                        for ( ; ; ) {
                            w = get(dot, itype);
                            if (errflg != NULL || mkfault || 
                                (w & locmsk) == locval) 
                                break; 
                            dot = inkdot(dotinc);
                        }
                        if (errflg != NULL) {
                            dot = savdot;
                            errflg = NOMATCH;
                        }
                        psymoff(dot, ptype, "");
                        break;

                    case 'W':
                        longpr = TRUE;

                    case 'w':
                        if (eqcom)
                            error(BADEQ);
                        wformat[0] = lastc;
                        (void) expr(1);
                        do {
                            savdot = dot; 
                            psymoff(dot, ptype, ":%16t"); 
                            (void) exform(1, wformat, itype, ptype);
                            errflg = NULL;
                            dot = savdot;
                            put(dot, itype, expv);
                            savdot = dot;
                            adbpr("=%8t");
                            (void) exform(1, wformat, itype, ptype);
                            newline();
                        } while (expr(0) && errflg == NULL);
                        dot = savdot;
                        chkerr();
                        break;

                    default:
                        lp--;
                        getformat(eqcom ? eqformat : stformat);
                        if ( ! eqcom)
                            psymoff(dot, IDSYM, ":%16t");
                        scanform(cntval, (eqcom ? eqformat : stformat),
                            itype, ptype);
                }
                break;

            case '>':
                lastcom = 0;
                savc = rdc();
		{
			struct grres res;
			res = getreg(savc);
			if (res.sf) {
				regptr = res.roff;
				ADBREG(regptr) = dot;
				(void) ptrace(WUREGS, pid, 
				SYSREG(regptr), 
				ADBREG(regptr));
			}
			else if ((modifier = varchk(savc))!=-1)
				var[modifier] = dot;
			else 
				error(BADVAR);
		}
                break;

            case '!':
                lastcom = 0;
                shell();
                break;

            case '$':
                lastcom = 0;
                dollar(nextchar());
                break;

            case ':':
                if ( ! executing) { 
                    executing = TRUE;
                    subpcs(nextchar());
                    executing = FALSE;
                    lastcom = 0;
                }
                break;

            case 0:
                prints(DBNAME);
                break;

            default:
                error(BADCOM);
        }

out:        flushbuf();
    } while (rdc() == ';');
    if (buf)
        lp = savlp; 
    else
        lp--; 
}

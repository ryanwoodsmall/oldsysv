
# line 13 "att1.y"
/*	@(#)att1.y	1.2	*/

#include "stdio.h"
#include "ctype.h"
#include "time.h"

extern	int	gmtflag;
extern	int	mday[];
extern	struct	tm *tp, at, rt;
# define TIME 257
# define NOW 258
# define NOON 259
# define MIDNIGHT 260
# define MINUTE 261
# define HOUR 262
# define DAY 263
# define WEEK 264
# define MONTH 265
# define YEAR 266
# define UNIT 267
# define SUFF 268
# define AM 269
# define PM 270
# define ZULU 271
# define NEXT 272
# define NUMB 273
# define COLON 274
# define COMMA 275
# define PLUS 276
# define UNKNOWN 277
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#ifndef YYSTYPE
#define YYSTYPE int
#endif
YYSTYPE yylval, yyval;
typedef int yytabelem;
# define YYERRCODE 256

# line 195 "att1.y"

yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 23
# define YYLAST 50
yytabelem yyact[]={

    23,    16,    12,     4,    28,    17,    27,    13,    10,    12,
    28,    19,    22,    14,    28,    12,    30,    24,     8,     5,
     7,    18,     9,    25,    11,     3,    15,     6,     2,     1,
     0,     0,    20,     0,    21,     0,     0,     0,     0,    26,
     0,     0,     0,    29,     0,     0,     0,     0,     0,    31 };
yytabelem yypact[]={

  -254, -1000,  -245,  -266, -1000,  -260,  -271,  -262, -1000, -1000,
  -262,  -253, -1000,  -261, -1000,  -277,  -250,  -262,  -269, -1000,
  -259, -1000, -1000, -1000, -1000,  -251,  -263,  -262, -1000, -1000,
 -1000,  -263 };
yytabelem yypgo[]={

     0,    29,    28,    27,    26,    25,    22,    21,    24,    23 };
yytabelem yyr1[]={

     0,     1,     1,     2,     2,     2,     2,     3,     3,     3,
     3,     4,     4,     4,     5,     5,     8,     7,     7,     9,
     9,     6,     6 };
yytabelem yyr2[]={

     0,     7,     9,     5,     9,     7,     3,     1,     5,     9,
     3,     0,     5,     7,     3,     5,     5,     3,     5,     1,
     3,     1,     3 };
yytabelem yychk[]={

 -1000,    -1,    -2,    -5,   257,   273,    -3,   265,   263,    -6,
   274,    -8,   268,   273,   273,    -4,   272,   276,    -7,   273,
    -7,    -6,   273,   277,   267,    -9,    -7,   275,   273,    -6,
   267,    -7 };
yytabelem yydef[]={

     0,    -2,     7,    21,     6,    14,    11,     0,    10,     3,
     0,    21,    22,     0,    15,     1,     0,    19,     8,    17,
    21,     5,    16,     2,    12,     0,    20,     0,    18,     4,
    13,     9 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"TIME",	257,
	"NOW",	258,
	"NOON",	259,
	"MIDNIGHT",	260,
	"MINUTE",	261,
	"HOUR",	262,
	"DAY",	263,
	"WEEK",	264,
	"MONTH",	265,
	"YEAR",	266,
	"UNIT",	267,
	"SUFF",	268,
	"AM",	269,
	"PM",	270,
	"ZULU",	271,
	"NEXT",	272,
	"NUMB",	273,
	"COLON",	274,
	"COMMA",	275,
	"PLUS",	276,
	"UNKNOWN",	277,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"args : time date incr",
	"args : time date incr UNKNOWN",
	"time : hour opt_suff",
	"time : hour COLON number opt_suff",
	"time : hour minute opt_suff",
	"time : TIME",
	"date : /* empty */",
	"date : MONTH number",
	"date : MONTH number COMMA number",
	"date : DAY",
	"incr : /* empty */",
	"incr : NEXT UNIT",
	"incr : PLUS opt_number UNIT",
	"hour : NUMB",
	"hour : NUMB NUMB",
	"minute : NUMB NUMB",
	"number : NUMB",
	"number : number NUMB",
	"opt_number : /* empty */",
	"opt_number : number",
	"opt_suff : /* empty */",
	"opt_suff : SUFF",
};
#endif /* YYDEBUG */
/*	@(#)yaccpar	1.9	*/

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-1000)

/*
** global variables used by the parser
*/
YYSTYPE yyv[ YYMAXDEPTH ];	/* value stack */
int yys[ YYMAXDEPTH ];		/* state stack */

YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ YYMAXDEPTH ] )	/* room on stack? */
		{
			yyerror( "yacc stack overflow" );
			YYABORT;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 1:
# line 48 "att1.y"
{
		if (at.tm_min >= 60 || at.tm_hour >= 24)
			atabort("bad time");
		if (at.tm_mon >= 12 || at.tm_mday > mday[at.tm_mon])
			atabort("bad date");
		if (at.tm_year >= 100)
			at.tm_year -= 1900;
		if (at.tm_year < 70 || at.tm_year >= 100)
			atabort("bad year");
		return;
	} break;
case 2:
# line 59 "att1.y"
{
		yyerror();
	} break;
case 3:
# line 65 "att1.y"
{
	checksuff:
		at.tm_hour = yypvt[-1];
		switch (yypvt[-0]) {
		case PM:
			if (at.tm_hour < 1 || at.tm_hour > 12)
				atabort("bad hour");
				at.tm_hour %= 12;
				at.tm_hour += 12;
				break;
		case AM:
			if (at.tm_hour < 1 || at.tm_hour > 12)
				atabort("bad hour");
			at.tm_hour %= 12;
			break;
		case ZULU:
			if (at.tm_hour == 24 && at.tm_min != 0)
				atabort("bad time");
			at.tm_hour %= 24;
			gmtflag = 1;
		}
	} break;
case 4:
# line 87 "att1.y"
{
		at.tm_min = yypvt[-1];
		yypvt[-1] = yypvt[-3];
		goto checksuff;
	} break;
case 5:
# line 92 "att1.y"
{
		at.tm_min = yypvt[-1];
		yypvt[-1] = yypvt[-2];
		goto checksuff;
	} break;
case 6:
# line 97 "att1.y"
{
		switch (yypvt[-0]) {
		case NOON:
			at.tm_hour = 12;
			break;
		case MIDNIGHT:
			at.tm_hour = 0;
			break;
		case NOW:
			at.tm_hour = tp->tm_hour;
			at.tm_min = tp->tm_min;
			break;
		}
	} break;
case 7:
# line 114 "att1.y"
{
		at.tm_mday = tp->tm_mday;
		at.tm_mon = tp->tm_mon;
		at.tm_year = tp->tm_year;
		if ((at.tm_hour < tp->tm_hour)
			|| ((at.tm_hour==tp->tm_hour)&&(at.tm_min<tp->tm_min)))
			rt.tm_mday++;
	} break;
case 8:
# line 122 "att1.y"
{
		at.tm_mon = yypvt[-1];
		at.tm_mday = yypvt[-0];
		at.tm_year = tp->tm_year;
		if (at.tm_mon < tp->tm_mon)
			at.tm_year++;
	} break;
case 9:
# line 129 "att1.y"
{
		at.tm_mon = yypvt[-3];
		at.tm_mday = yypvt[-2];
		at.tm_year = yypvt[-0];
	} break;
case 10:
# line 134 "att1.y"
{
		at.tm_mon = tp->tm_mon;
		at.tm_mday = tp->tm_mday;
		at.tm_year = tp->tm_year;
		if (yypvt[-0] < 7) {
			rt.tm_mday = yypvt[-0] - tp->tm_wday;
			if (rt.tm_mday < 0)
				rt.tm_mday += 7;
		} else if (yypvt[-0] == 8)
			rt.tm_mday += 1;
	} break;
case 12:
# line 149 "att1.y"
{ addincr:
		switch (yypvt[-0]) {
		case MINUTE:
			rt.tm_min += yypvt[-1];
			break;
		case HOUR:
			rt.tm_hour += yypvt[-1];
			break;
		case DAY:
			rt.tm_mday += yypvt[-1];
			break;
		case WEEK:
			rt.tm_mday += yypvt[-1] * 7;
			break;
		case MONTH:
			rt.tm_mon += yypvt[-1];
			break;
		case YEAR:
			rt.tm_year += yypvt[-1];
			break;
		}
	} break;
case 13:
# line 171 "att1.y"
{ goto addincr; } break;
case 14:
# line 175 "att1.y"
{ yyval = yypvt[-0]; } break;
case 15:
# line 176 "att1.y"
{ yyval = 10 * yypvt[-1] + yypvt[-0]; } break;
case 16:
# line 179 "att1.y"
{ yyval = 10 * yypvt[-1] + yypvt[-0]; } break;
case 17:
# line 182 "att1.y"
{ yyval = yypvt[-0]; } break;
case 18:
# line 183 "att1.y"
{ yyval = 10 * yypvt[-1] + yypvt[-0]; } break;
case 19:
# line 186 "att1.y"
{ yyval = 1; } break;
case 20:
# line 187 "att1.y"
{ yyval = yypvt[-0]; } break;
case 21:
# line 190 "att1.y"
{ yyval = 0; } break;
case 22:
# line 191 "att1.y"
{ yyval = yypvt[-0]; } break;
	}
	goto yystack;		/* reset registers in driver code */
}

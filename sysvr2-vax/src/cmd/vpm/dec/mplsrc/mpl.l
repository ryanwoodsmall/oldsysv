/*	mpl.l 1.4 of 12/2/83	*/
/*	@(#)mpl.l	1.4	*/
%{
/*
 *	mpl.l - lexical analyzer for VPM compiler
 */
#include <stdio.h>
#include <ctype.h>
#define echo	if(debug == 2)strcat(sline,yytext);
#define NO	0
#define YES	1
#define MAXLEN	9
extern yylval;
extern int yyline;
extern char infile[];
char sline[80];
char name[MAXLEN+MAXLEN];
char opcode[16];
char *allupper();
%}
D	[0-9]
H	[0-9a-fA-F]
L	[a-zA-Z]
AN	[0-9a-zA-Z_]
A	[\000-\0177]
E	[\000-\0377]
W	[ \t\n]
%%
int		{echo; return(ARRAY);}
array		{echo; return(ARRAY);}
unsigned{W}+char	{echo; return(ARRAY);}
if		{echo; return(IF);}
goto		{echo; return(GOTO);}
continue	{echo; return(CONTINUE);}
return		{echo; return(RETURN);}
while		{echo; return(WHILE); }
for		{ echo; return(FOR); }
do		{ echo; return(DO); }
else		{ echo; return(ELSE); }
switch		{ echo; return(SWITCH); }
case		{ echo; return(CASE); }
break		{ echo; return(BREAK); }
default		{ echo; return(DEFAULT); }
atoe		|
etoa		|
crc16		|
xmt		|
getxfrm		|
rtnxfrm		|
xmtfrm		|
testop		|
timeout		|
timer		{ 	/* system call with primary argument */
			echo; allupper(opcode,yytext); return(SC_PRIM);
		}
put		{ echo; strcpy(opcode,"PUTBYTE"); return(SC_PRIM); }
exit		{ echo; strcpy(opcode,"HALT"); return(SC_PRIM); }
crcloc		|
getrbuf		|
getxbuf		|
rtnrbuf		|
rtnxbuf		|
rcvfrm		|
getcmd		|
snap		|
rtnrpt		{	/* system call with name argument */
			echo; allupper(opcode,yytext); return(SC_NAME);
		}
rcv		{	/* system call with lvalue argument */
			echo; allupper(opcode,yytext); return(RCV);
		}
get		{	/* system call with lvalue argument */
			echo; allupper(opcode,yytext); return(GETBYTE);
		}
rsom		|
xeom		|
xsom		{	/* system call with constant argument */
			echo; allupper(opcode,yytext); return(SC_CONST);
		}
rsrfrm		|
abtxfrm		|
rtnrfrm		|
rsxmtq		|
xmtbusy		|
xmtctl		|
dsrwait		|
norbuf		|
getopt		|
pause		{	/* system call with no argument */
			echo; allupper(opcode,yytext); return(SC_NULL);
		}
setctl		{
			echo; allupper(opcode,yytext); return(SC_NP);
		}
nxtfrm		{
			echo; strcpy(opcode,"NXTFRM"); return(SC_NP);
		}
nxtstate	{
			 echo; strcpy(opcode,"NXTSTA"); return(SC_NP); 
		}
trace		{ echo; strcpy(opcode,"TRACE"); return(TRACE); }
"=="		{echo; strcpy(opcode,"TSTEQL"); return(BINOP);}
"!="		{echo; strcpy(opcode,"TSTNEQ"); return(BINOP);}
"<"		{echo; strcpy(opcode,"TSTLSS"); return(BINOP);}
"<="		{echo; strcpy(opcode,"TSTLEQ"); return(BINOP);}
">"		{echo; strcpy(opcode,"TSTGTR"); return(BINOP);}
">="		{echo; strcpy(opcode,"TSTGEQ"); return(BINOP);}
"-"		{echo; strcpy(opcode,"SUB"); return(BINOP);}
"+"		{echo; strcpy(opcode,"ADD"); return(BINOP);}
"^"		{echo; strcpy(opcode,"XOR"); return(BINOP);}
"`"		{echo; strcpy(opcode,"XOR"); return(BINOP);}
"|"		{echo; strcpy(opcode,"OR"); return(BINOP);}
"&"		{echo; strcpy(opcode,"AND"); return(BINOP);}
"<<"		{echo; strcpy(opcode,"LSH"); return(BINOP);}
">>"		{echo; strcpy(opcode,"RSH"); return(BINOP);}
\&{W}*\~	{echo; strcpy(opcode,"BIC"); return(BINOP);}
"&&"		{ echo; return(LOGAND); }
"||"		{ echo; return(LOGOR); }
"="		{echo; yylval = 0;  return(REPOP);}
"|="		{echo; yylval = 1;  return(REPOP);}
"^="		{echo; yylval = 2;  return(REPOP);}
"`="		{echo; yylval = 2;  return(REPOP);}
"+="		{echo; yylval = 3;  return(REPOP);}
"-="		{echo; yylval = 4;  return(REPOP);}
"&="		{echo; yylval = 5;  return(REPOP);}
"<<="		{echo; yylval = 6;  return(REPOP);}
">>="		{echo; yylval = 7;  return(REPOP);}
"--"		{echo; return(DEC);}
"++"		{echo; return(INC);}
"~"		{echo; return(COMP);}
"!"		{echo; return(NOT);}
":"		{echo; return(CO);}
";"		{echo; yylval = 1;return(SC);}
\n		{
			yyline++;
			yylval = 2;
		}
","		{echo; return(CM);}
")"		{echo; return(RP);}
"("		{echo; return(LP);}
\({W}*\&	{echo; return(LPAND); }
"{"		{echo; return(LC);}
"}"		{echo; return(RC);}
"["		{echo; return(LB);}
"]"		{echo; return(RB);}
^#.*\n		{	/* preprocessor line - check for #include info */
			char s1[16],s2[16],s3[128];

			sscanf(yytext,"%s %s %s",s1,s2,s3);
			if((EQ(s1,"\#") || EQ(s1,"\#line")) && isnum(s2)) {
				yyline = atoi(s2);
				strcpy(infile,s3);
			} 
		}
"/*"		{	/* flush comment */
			for(;;) {
			char c;
				while((c = input()) != '*')
					if(c == '\n')
						++yyline;
				if((c = input()) == '/')
					break;
				unput(c);
			}
		}
{D}+		|
0[0-7]+		|
0x{H}+		|
'.'		|
'\\.'		{
			echo;
			yytext[MAXLEN-1] = '\0';
			strcpy(name,yytext);
			yylval = CONST;
			return(CONST);
		}
{L}{AN}*	{
			echo;
			yytext[MAXLEN-1] = '\0';
			strcpy(name,yytext);
			yylval = NAME;
			if(strcmp(name,fname) == 0)
				return(FNAME);
			else
				return(NAME);
		}
{W}		{ echo; }
.		{ echo; return(ERR); }
%%

char *
allupper(dstr,sstr)	/* change string in sstr to all upper case in dstr */
char *dstr, *sstr;
{
	char *result;

	result = dstr;
	while(*dstr++ = toupper((int)*sstr++))
		;
	return(result);
}

isnum(s)	/* Does the string s consist only of numbers? */
char *s;
{
	while(*s) 
		if (!isdigit(*s++))
			return(NO);
	return(YES);
}


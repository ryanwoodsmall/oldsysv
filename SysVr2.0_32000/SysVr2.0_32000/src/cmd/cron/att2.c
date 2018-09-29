# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX 200
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
/*	@(#)att2.l	1.1	*/

#include "att1.h"
#define	LL(t,v)	return(yylval = v, t)
#undef getc
#define	getc(x)	(*argp?tolower(*argp++):EOF)
#undef feof
#define	feof(x)	(*argp?0:1)
extern	int yylval;
char	*argp = "";
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
	;
break;
case 2:
	{ LL(MONTH, 0); }
break;
case 3:
{ LL(MONTH, 1); }
break;
case 4:
	{ LL(MONTH, 2); }
break;
case 5:
	{ LL(MONTH, 3); }
break;
case 6:
		{ LL(MONTH, 4); }
break;
case 7:
	{ LL(MONTH, 5); }
break;
case 8:
	{ LL(MONTH, 6); }
break;
case 9:
	{ LL(MONTH, 7); }
break;
case 10:
{ LL(MONTH, 8); }
break;
case 11:
	{ LL(MONTH, 9); }
break;
case 12:
{ LL(MONTH, 10); }
break;
case 13:
{ LL(MONTH, 11); }
break;
case 14:
	{ LL(DAY, 0); }
break;
case 15:
	{ LL(DAY, 1); }
break;
case 16:
	{ LL(DAY, 2); }
break;
case 17:
{ LL(DAY, 3); }
break;
case 18:
{ LL(DAY, 4); }
break;
case 19:
	{ LL(DAY, 5); }
break;
case 20:
{ LL(DAY, 6); }
break;
case 21:
		{ LL(DAY, 7); }
break;
case 22:
	{ LL(DAY, 8); }
break;
case 23:
		{ LL(TIME, NOON); }
break;
case 24:
	{ LL(TIME, MIDNIGHT); }
break;
case 25:
		{ LL(TIME, NOW); }
break;
case 26:
		{ LL(SUFF, AM); }
break;
case 27:
		{ LL(SUFF, PM); }
break;
case 28:
		{ LL(SUFF, ZULU); }
break;
case 29:
		{ LL(NEXT, 1); }
break;
case 30:
	{ LL(UNIT, MINUTE); }
break;
case 31:
	{ LL(UNIT, HOUR); }
break;
case 32:
		{ LL(UNIT, DAY); }
break;
case 33:
	{ LL(UNIT, WEEK); }
break;
case 34:
	{ LL(UNIT, MONTH); }
break;
case 35:
	{ LL(UNIT, YEAR); }
break;
case 36:
		{ yylval = yytext[0] - '0'; return(NUMB); }
break;
case 37:
		{ LL(COLON, 0); }
break;
case 38:
		{ LL(COMMA, 0); }
break;
case 39:
		{ LL(PLUS, 0); }
break;
case 40:
		{ LL(UNKNOWN, 0); }
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
int yyvstop[] = {
0,

40,
0,

1,
40,
0,

1,
0,

39,
40,
0,

38,
40,
0,

36,
40,
0,

37,
40,
0,

40,
0,

40,
0,

40,
0,

40,
0,

40,
0,

40,
0,

40,
0,

40,
0,

40,
0,

40,
0,

40,
0,

40,
0,

40,
0,

40,
0,

26,
0,

27,
0,

5,
0,

9,
0,

32,
0,

13,
0,

3,
0,

19,
0,

2,
0,

8,
0,

7,
0,

4,
0,

6,
0,

30,
0,

15,
0,

12,
0,

25,
0,

11,
0,

20,
0,

10,
0,

14,
0,

18,
0,

16,
0,

17,
0,

32,
0,

31,
0,

8,
0,

7,
0,

30,
0,

29,
0,

23,
0,

33,
0,

35,
0,

28,
0,

5,
0,

31,
0,

4,
0,

34,
0,

21,
0,

33,
0,

35,
0,

9,
0,

19,
0,

30,
0,

15,
0,

34,
0,

14,
0,

2,
0,

11,
0,

16,
0,

13,
0,

3,
0,

24,
0,

12,
0,

20,
0,

18,
0,

22,
0,

10,
0,

17,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	1,3,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,4,	1,5,	
4,5,	4,5,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	4,5,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
1,6,	1,7,	0,0,	0,0,	
0,0,	1,8,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	1,9,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,10,	0,0,	
0,0,	1,11,	11,27,	1,12,	
17,39,	1,13,	11,28,	1,14,	
21,47,	16,37,	1,15,	1,16,	
1,17,	1,18,	12,29,	13,31,	
1,19,	1,20,	14,32,	16,38,	
1,21,	2,10,	1,22,	1,23,	
2,11,	18,40,	2,12,	12,30,	
2,13,	10,24,	2,14,	22,48,	
10,25,	2,15,	2,16,	2,17,	
2,18,	10,26,	14,33,	2,19,	
2,20,	15,34,	23,49,	2,21,	
19,41,	2,22,	2,23,	20,44,	
19,42,	15,35,	25,50,	26,51,	
27,52,	28,53,	20,45,	15,36,	
29,54,	30,55,	31,56,	32,57,	
20,46,	33,58,	34,60,	33,59,	
19,43,	35,62,	36,64,	37,65,	
39,69,	34,61,	38,66,	41,70,	
42,71,	43,72,	44,73,	35,63,	
45,74,	38,67,	38,68,	46,76,	
47,77,	47,78,	48,79,	49,80,	
50,81,	45,75,	51,82,	52,83,	
53,84,	54,85,	55,86,	56,87,	
57,88,	58,89,	59,90,	60,91,	
62,92,	63,93,	64,95,	63,94,	
65,97,	66,98,	67,99,	69,100,	
70,101,	71,102,	72,103,	73,104,	
74,105,	75,106,	76,107,	77,108,	
78,109,	79,110,	64,96,	80,111,	
81,112,	82,113,	84,114,	85,115,	
86,116,	87,117,	88,118,	91,119,	
92,120,	94,121,	95,122,	96,123,	
99,124,	100,125,	101,126,	102,127,	
103,128,	104,129,	105,130,	106,131,	
107,132,	108,133,	109,134,	110,135,	
113,136,	114,137,	115,138,	116,139,	
118,140,	120,141,	121,142,	122,143,	
123,144,	124,145,	125,146,	126,147,	
127,148,	128,149,	129,150,	131,151,	
132,152,	133,153,	137,154,	138,155,	
140,156,	141,157,	142,93,	145,158,	
146,159,	147,160,	148,161,	150,162,	
151,163,	152,164,	153,165,	154,166,	
155,167,	157,168,	158,169,	160,170,	
161,171,	162,172,	163,173,	165,174,	
171,175,	174,176,	0,0,	0,0,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+-24,	yysvec+1,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+3,	0,		yyvstop+3,
yycrank+0,	yysvec+4,	yyvstop+6,
yycrank+0,	0,		yyvstop+8,
yycrank+0,	0,		yyvstop+11,
yycrank+0,	0,		yyvstop+14,
yycrank+0,	0,		yyvstop+17,
yycrank+20,	0,		yyvstop+20,
yycrank+5,	0,		yyvstop+22,
yycrank+13,	0,		yyvstop+24,
yycrank+4,	0,		yyvstop+26,
yycrank+21,	0,		yyvstop+28,
yycrank+44,	0,		yyvstop+30,
yycrank+8,	0,		yyvstop+32,
yycrank+5,	0,		yyvstop+34,
yycrank+16,	0,		yyvstop+36,
yycrank+47,	0,		yyvstop+38,
yycrank+43,	0,		yyvstop+40,
yycrank+7,	0,		yyvstop+42,
yycrank+30,	0,		yyvstop+44,
yycrank+25,	0,		yyvstop+46,
yycrank+0,	0,		yyvstop+48,
yycrank+36,	0,		0,	
yycrank+48,	0,		0,	
yycrank+31,	0,		0,	
yycrank+54,	0,		0,	
yycrank+58,	0,		0,	
yycrank+52,	0,		0,	
yycrank+41,	0,		0,	
yycrank+49,	0,		0,	
yycrank+53,	0,		0,	
yycrank+48,	0,		0,	
yycrank+65,	0,		0,	
yycrank+56,	0,		0,	
yycrank+47,	0,		0,	
yycrank+59,	0,		0,	
yycrank+52,	0,		0,	
yycrank+0,	0,		yyvstop+50,
yycrank+55,	0,		0,	
yycrank+60,	0,		0,	
yycrank+63,	0,		0,	
yycrank+57,	0,		0,	
yycrank+76,	0,		0,	
yycrank+78,	0,		0,	
yycrank+80,	0,		0,	
yycrank+85,	0,		0,	
yycrank+75,	0,		0,	
yycrank+79,	0,		yyvstop+52,
yycrank+69,	0,		yyvstop+54,
yycrank+72,	0,		yyvstop+56,
yycrank+87,	0,		yyvstop+58,
yycrank+75,	0,		yyvstop+60,
yycrank+90,	0,		yyvstop+62,
yycrank+77,	0,		0,	
yycrank+75,	0,		yyvstop+64,
yycrank+72,	0,		yyvstop+66,
yycrank+93,	0,		yyvstop+68,
yycrank+96,	0,		yyvstop+70,
yycrank+0,	0,		yyvstop+72,
yycrank+86,	0,		0,	
yycrank+82,	0,		yyvstop+74,
yycrank+98,	0,		yyvstop+76,
yycrank+84,	0,		0,	
yycrank+91,	0,		0,	
yycrank+101,	0,		yyvstop+78,
yycrank+0,	0,		yyvstop+80,
yycrank+92,	0,		yyvstop+82,
yycrank+87,	0,		yyvstop+84,
yycrank+89,	0,		yyvstop+86,
yycrank+106,	0,		yyvstop+88,
yycrank+93,	0,		yyvstop+90,
yycrank+111,	0,		0,	
yycrank+98,	0,		0,	
yycrank+95,	0,		yyvstop+92,
yycrank+101,	0,		yyvstop+94,
yycrank+105,	0,		0,	
yycrank+99,	0,		0,	
yycrank+98,	0,		0,	
yycrank+108,	0,		0,	
yycrank+102,	0,		0,	
yycrank+0,	0,		yyvstop+96,
yycrank+109,	0,		0,	
yycrank+102,	0,		0,	
yycrank+123,	0,		0,	
yycrank+106,	0,		yyvstop+98,
yycrank+125,	0,		0,	
yycrank+0,	0,		yyvstop+100,
yycrank+0,	0,		yyvstop+102,
yycrank+119,	0,		0,	
yycrank+119,	0,		0,	
yycrank+0,	0,		yyvstop+104,
yycrank+109,	0,		0,	
yycrank+129,	0,		0,	
yycrank+123,	0,		0,	
yycrank+0,	0,		yyvstop+106,
yycrank+0,	0,		yyvstop+108,
yycrank+119,	0,		0,	
yycrank+131,	0,		0,	
yycrank+116,	0,		0,	
yycrank+130,	0,		0,	
yycrank+135,	0,		0,	
yycrank+118,	0,		0,	
yycrank+113,	0,		0,	
yycrank+121,	0,		0,	
yycrank+136,	0,		0,	
yycrank+136,	0,		0,	
yycrank+123,	0,		yyvstop+110,
yycrank+124,	0,		yyvstop+112,
yycrank+0,	0,		yyvstop+114,
yycrank+0,	0,		yyvstop+116,
yycrank+124,	0,		0,	
yycrank+143,	0,		0,	
yycrank+145,	0,		0,	
yycrank+122,	0,		0,	
yycrank+0,	0,		yyvstop+118,
yycrank+130,	0,		0,	
yycrank+0,	0,		yyvstop+120,
yycrank+142,	0,		0,	
yycrank+145,	0,		0,	
yycrank+126,	0,		0,	
yycrank+133,	0,		yyvstop+122,
yycrank+151,	0,		0,	
yycrank+149,	0,		0,	
yycrank+151,	0,		0,	
yycrank+143,	0,		0,	
yycrank+132,	0,		0,	
yycrank+154,	0,		0,	
yycrank+0,	0,		yyvstop+124,
yycrank+141,	0,		0,	
yycrank+159,	0,		0,	
yycrank+142,	0,		0,	
yycrank+0,	0,		yyvstop+126,
yycrank+0,	0,		yyvstop+128,
yycrank+0,	0,		yyvstop+130,
yycrank+157,	0,		0,	
yycrank+145,	0,		0,	
yycrank+0,	0,		yyvstop+132,
yycrank+139,	0,		0,	
yycrank+157,	0,		0,	
yycrank+147,	0,		yyvstop+134,
yycrank+0,	0,		yyvstop+136,
yycrank+0,	0,		yyvstop+138,
yycrank+162,	0,		0,	
yycrank+150,	0,		0,	
yycrank+168,	0,		0,	
yycrank+168,	0,		0,	
yycrank+0,	0,		yyvstop+140,
yycrank+170,	0,		0,	
yycrank+157,	0,		0,	
yycrank+148,	0,		0,	
yycrank+170,	0,		0,	
yycrank+157,	0,		0,	
yycrank+151,	0,		0,	
yycrank+0,	0,		yyvstop+142,
yycrank+157,	0,		0,	
yycrank+160,	0,		0,	
yycrank+0,	0,		yyvstop+144,
yycrank+154,	0,		0,	
yycrank+175,	0,		0,	
yycrank+156,	0,		0,	
yycrank+159,	0,		0,	
yycrank+0,	0,		yyvstop+146,
yycrank+182,	0,		0,	
yycrank+0,	0,		yyvstop+148,
yycrank+0,	0,		yyvstop+150,
yycrank+0,	0,		yyvstop+152,
yycrank+0,	0,		yyvstop+154,
yycrank+0,	0,		yyvstop+156,
yycrank+166,	0,		0,	
yycrank+0,	0,		yyvstop+158,
yycrank+0,	0,		yyvstop+160,
yycrank+160,	0,		0,	
yycrank+0,	0,		yyvstop+162,
yycrank+0,	0,		yyvstop+164,
0,	0,	0};
struct yywork *yytop = yycrank+281;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
011 ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,'+' ,',' ,01  ,01  ,01  ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,':' ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
/*	@(#)ncform	1.2	*/
int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0   && feof(yyin) )
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}

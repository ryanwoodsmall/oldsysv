#ifndef lint
static char const 
yyrcsid[] = "$FreeBSD: src/usr.bin/yacc/skeleton.c,v 1.28 2000/01/17 02:04:06 bde Exp $";
#endif
#include <stdlib.h>
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYLEX yylex()
#define YYEMPTY -1
#define yyclearin (yychar=(YYEMPTY))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING() (yyerrflag!=0)
static int yygrowstack();
#define YYPREFIX "yy"
#line 24 "parser.y"
#define YYSTYPE union _yystype_
union _yystype_
{
	int	i;
	char	*cp;
};
# include "ldefs.c"
#line 34 "parser.y"
int i;
int j,k;
int g;
char *p;
#line 30 "y.tab.c"
#define YYERRCODE 256
#define CHAR 257
#define CCL 258
#define NCCL 259
#define STR 260
#define DELIM 261
#define SCON 262
#define ITER 263
#define NEWE 264
#define NULLS 265
#define CAT 266
const short yylhs[] = {                                        -1,
    0,    1,    1,    1,    5,    5,    2,    2,    3,    4,
    4,    6,    6,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,
};
const short yylen[] = {                                         2,
    1,    4,    3,    1,    1,    0,    3,    0,    1,    2,
    1,    2,    2,    1,    1,    1,    1,    1,    2,    2,
    2,    3,    2,    3,    5,    3,    4,    2,    2,    2,
    3,    1,
};
const short yydefred[] = {                                      0,
    4,    0,    1,    0,    0,    9,    0,    7,    0,   14,
   17,   18,   15,    0,   32,    0,    0,   16,    5,    0,
    3,   11,    0,   13,    0,    0,    0,    2,   10,    0,
   12,    0,    0,   30,   19,   20,   21,    0,   31,    0,
   26,    0,    0,    0,   27,   25,
};
const short yydgoto[] = {                                       2,
    3,    4,   19,   20,   21,   22,   38,
};
const short yysindex[] = {                                   -253,
    0,    0,    0, -236, -255,    0,   24,    0, -256,    0,
    0,    0,    0,   46,    0,   46,   46,    0,    0,   24,
    0,    0,  -36,    0,   -3,   35,  -27,    0,    0,  -42,
    0,   46,   46,    0,    0,    0,    0,    2,    0, -124,
    0,   -3,    6, -113,    0,    0,
};
const short yyrindex[] = {                                   -230,
    0,    0,    0,    0,    0,    0,   17,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   17,
    0,    0,    0,    0,   15,   14,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  -18,    0,    0,
    0,   48,   10,    0,    0,    0,
};
const short yygindex[] = {                                      0,
    0,    0,   22,    0,    1,   12,   87,
};
#define YYTABLESIZE 312
const short yytable[] = {                                      34,
   45,   40,    1,   17,    8,   35,   36,   24,   34,   18,
   32,   46,   17,   39,   35,   36,    6,   23,   18,   32,
   28,   23,   23,    5,    6,    7,   37,   23,   23,    8,
    8,   29,   34,    0,    0,   37,   17,    0,   35,   36,
    0,   34,   18,   35,   36,   17,    0,   35,   36,   29,
   22,   18,    0,    0,   29,   28,   22,   16,    0,   37,
   29,   28,    0,   17,   37,    0,   16,    0,   37,   18,
    0,    0,    0,    0,   17,   23,   35,   36,    0,    0,
   18,    0,   41,    0,    0,   17,    0,   33,   24,    0,
   16,   18,    0,   23,   24,    0,   33,   37,    0,   16,
   25,    0,   26,   27,    0,   23,   23,   29,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   16,   42,   43,
   33,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   22,    0,    0,    0,   29,   44,   16,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   10,   11,   12,   13,    0,   14,   30,   31,   15,   10,
   11,   12,   13,    0,   14,   30,    0,   15,   23,   23,
   23,   23,    0,   23,   23,   23,   23,    0,    0,    0,
    0,    0,    0,   10,   11,   12,   13,    0,    0,   30,
    0,   15,   10,   11,   12,   13,    0,    0,   30,    0,
   15,   22,    0,   22,    0,   29,   28,   29,   28,    9,
   10,   11,   12,   13,    6,   14,    0,    0,   15,    0,
    0,   10,   11,   12,   13,    0,    0,   30,    0,   15,
    0,    0,   10,   11,   12,   13,    0,   14,    0,   24,
   15,   24,
};
const short yycheck[] = {                                      36,
  125,   44,  256,   40,  260,   42,   43,  264,   36,   46,
   47,  125,   40,   41,   42,   43,    0,   36,   46,   47,
   20,   40,   41,  260,  261,    4,   63,   46,   47,  260,
  261,   20,   36,   -1,   -1,   63,   40,   -1,   42,   43,
   -1,   36,   46,   42,   43,   40,   -1,   42,   43,   36,
   41,   46,   -1,   -1,   41,   41,   47,   94,   -1,   63,
   47,   47,   -1,   40,   63,   -1,   94,   -1,   63,   46,
   -1,   -1,   -1,   -1,   40,   94,   42,   43,   -1,   -1,
   46,   -1,  125,   -1,   -1,   40,   -1,  124,   41,   -1,
   94,   46,   -1,    7,   47,   -1,  124,   63,   -1,   94,
   14,   -1,   16,   17,   -1,  124,   20,   94,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   94,   32,   33,
  124,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  124,   -1,   -1,   -1,  124,  263,   94,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  257,  258,  259,  260,   -1,  262,  263,  264,  265,  257,
  258,  259,  260,   -1,  262,  263,   -1,  265,  257,  258,
  259,  260,   -1,  262,  263,  264,  265,   -1,   -1,   -1,
   -1,   -1,   -1,  257,  258,  259,  260,   -1,   -1,  263,
   -1,  265,  257,  258,  259,  260,   -1,   -1,  263,   -1,
  265,  262,   -1,  264,   -1,  262,  262,  264,  264,  256,
  257,  258,  259,  260,  261,  262,   -1,   -1,  265,   -1,
   -1,  257,  258,  259,  260,   -1,   -1,  263,   -1,  265,
   -1,   -1,  257,  258,  259,  260,   -1,  262,   -1,  262,
  265,  264,
};
#define YYFINAL 2
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 266
#if YYDEBUG
const char * const yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,"'$'",0,0,0,"'('","')'","'*'","'+'","','",0,"'.'","'/'",0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,"'?'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'^'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'|'","'}'",0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,"CHAR","CCL","NCCL","STR","DELIM","SCON","ITER","NEWE",
"NULLS","CAT",
};
const char * const yyrule[] = {
"$accept : acc",
"acc : lexinput",
"lexinput : defns delim prods end",
"lexinput : defns delim end",
"lexinput : error",
"end : delim",
"end :",
"defns : defns STR STR",
"defns :",
"delim : DELIM",
"prods : prods pr",
"prods : pr",
"pr : r NEWE",
"pr : error NEWE",
"r : CHAR",
"r : STR",
"r : '.'",
"r : CCL",
"r : NCCL",
"r : r '*'",
"r : r '+'",
"r : r '?'",
"r : r '|' r",
"r : r r",
"r : r '/' r",
"r : r ITER ',' ITER '}'",
"r : r ITER '}'",
"r : r ITER ',' '}'",
"r : SCON r",
"r : '^' r",
"r : r '$'",
"r : '(' r ')'",
"r : NULLS",
};
#endif
#ifndef YYSTYPE
typedef int YYSTYPE;
#endif
#if YYDEBUG
#include <stdio.h>
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH 10000
#endif
#endif
#define YYINITSTACKSIZE 200
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short *yyss;
short *yysslim;
YYSTYPE *yyvs;
int yystacksize;
#line 231 "parser.y"
yylex(){
	register char *p;
	register int c, i;
	char  *t, *xp;
	int n, j, k, x;
	static int sectbegin;
	static char token[TOKENSIZE];
	static int iter;

# ifdef DEBUG
	yylval.i = 0;
# endif

	if(sect == DEFSECTION) {		/* definitions section */
		while(!eof) {
			if(prev == '\n'){		/* next char is at beginning of line */
				getl(p=buf);
				switch(*p){
				case '%':
					switch(c= *(p+1)){
					case '%':
						lgate();
						if(!ratfor)fprintf(fout,"# ");
						fprintf(fout,"define YYNEWLINE %d\n",ctable['\n']);
						if(!ratfor)fprintf(fout,"yylex(){\nint nstr; extern int yyprevious;\n");
						sectbegin = TRUE;
						i = treesize*(sizeof(*name)+sizeof(*left)+
							sizeof(*right)+sizeof(*nullstr)+sizeof(*parent))+ALITTLEEXTRA;
						c = (int)myalloc(i,1);
						if(c == 0)
							error("Too little core for parse tree");
						p = (char *)c;
						cfree((char *)p,i,1);
						name = (int *)myalloc(treesize,sizeof(*name));
						left = (int *)myalloc(treesize,sizeof(*left));
						right = (int *)myalloc(treesize,sizeof(*right));
						nullstr = myalloc(treesize,sizeof(*nullstr));
						parent = (int *)myalloc(treesize,sizeof(*parent));
						if(name == 0 || left == 0 || right == 0 || parent == 0 || nullstr == 0)
							error("Too little core for parse tree");
						return(freturn(DELIM));
					case 'p': case 'P':	/* has overridden number of positions */
						while(*p && !digit(*p))p++;
						maxpos = siconv(p);
# ifdef DEBUG
						if (debug) printf("positions (%%p) now %d\n",maxpos);
# endif
						if(report == 2)report = 1;
						continue;
					case 'n': case 'N':	/* has overridden number of states */
						while(*p && !digit(*p))p++;
						nstates = siconv(p);
# ifdef DEBUG
						if(debug)printf( " no. states (%%n) now %d\n",nstates);
# endif
						if(report == 2)report = 1;
						continue;
					case 'e': case 'E':		/* has overridden number of tree nodes */
						while(*p && !digit(*p))p++;
						treesize = siconv(p);
# ifdef DEBUG
						if (debug) printf("treesize (%%e) now %d\n",treesize);
# endif
						if(report == 2)report = 1;
						continue;
					case 'o': case 'O':
						while (*p && !digit(*p))p++;
						outsize = siconv(p);
						if (report ==2) report=1;
						continue;
					case 'a': case 'A':		/* has overridden number of transitions */
						while(*p && !digit(*p))p++;
						if(report == 2)report = 1;
						ntrans = siconv(p);
# ifdef DEBUG
						if (debug)printf("N. trans (%%a) now %d\n",ntrans);
# endif
						continue;
					case 'k': case 'K': /* overriden packed char classes */
						while (*p && !digit(*p))p++;
						if (report==2) report=1;
						cfree((char *)pchar, pchlen, sizeof(*pchar));
						pchlen = siconv(p);
# ifdef DEBUG
						if (debug) printf( "Size classes (%%k) now %d\n",pchlen);
# endif
						pchar=pcptr=myalloc(pchlen, sizeof(*pchar));
						continue;
					case 't': case 'T': 	/* character set specifier */
						ZCH = atoi(p+2);
						if (ZCH < NCH) ZCH = NCH;
						if (ZCH > 2*NCH) error("ch table needs redeclaration");
						chset = TRUE;
						for(i = 0; i<ZCH; i++)
							ctable[i] = 0;
						while(getl(p) && scomp(p,"%T") != 0 && scomp(p,"%t") != 0){
							if((n = siconv(p)) <= 0 || n > ZCH){
								warning("Character value %d out of range",n);
								continue;
								}
							while(!space(*p) && *p) p++;
							while(space(*p)) p++;
							t = p;
							while(*t){
								c = ctrans(&t);
								if(ctable[c]){
									if (printable(c))
										warning("Character '%c' used twice",c);
									else
										warning("Character %o used twice",c);
									}
								else ctable[c] = n;
								t++;
								}
							p = buf;
							}
						{
						char chused[2*NCH]; int kr;
						for(i=0; i<ZCH; i++)
							chused[i]=0;
						for(i=0; i<NCH; i++)
							chused[ctable[i]]=1;
						for(kr=i=1; i<NCH; i++)
							if (ctable[i]==0)
								{
								while (chused[kr] == 0)
									kr++;
								ctable[i]=kr;
								chused[kr]=1;
								}
						}
						lgate();
						continue;
					case 'r': case 'R':
						c = 'r';
					case 'c': case 'C':
						if(lgatflg)
							error("Too late for language specifier");
						ratfor = (c == 'r');
						continue;
					case '{':
						lgate();
						while(getl(p) && scomp(p,"%}") != 0)
							fprintf(fout, "%s\n",p);
						if(p[0] == '%') continue;
						error("Premature eof");
					case 's': case 'S':		/* start conditions */
						lgate();
						while(*p && index(*p," \t,") < 0) p++;
						n = TRUE;
						while(n){
							while(*p && index(*p," \t,") >= 0) p++;
							t = p;
							while(*p && index(*p," \t,") < 0)p++;
							if(!*p) n = FALSE;
							*p++ = 0;
							if (*t == 0) continue;
							i = sptr*2;
							if(!ratfor)fprintf(fout,"# ");
							fprintf(fout,"define %s %d\n",t,i);
							scopy(t,sp);
							sname[sptr++] = sp;
							sname[sptr] = 0;	/* required by lookup */
							if(sptr >= STARTSIZE)
								error("Too many start conditions");
							sp += slength(sp) + 1;
							if(sp >= schar+STARTCHAR)
								error("Start conditions too long");
							}
						continue;
					default:
						warning("Invalid request %s",p);
						continue;
						}	/* end of switch after seeing '%' */
				case ' ': case '\t':		/* must be code */
					lgate();
					fprintf(fout, "%s\n",p);
					continue;
				default:		/* definition */
					while(*p && !space(*p)) p++;
					if(*p == 0)
						continue;
					prev = *p;
					*p = 0;
					bptr = p+1;
					yylval.cp = buf;
					if(digit(buf[0]))
						warning("Substitution strings may not begin with digits");
					return(freturn(STR));
					}
				}
			/* still sect 1, but prev != '\n' */
			else {
				p = bptr;
				while(*p && space(*p)) p++;
				if(*p == 0)
					warning("No translation given - null string assumed");
				scopy(p,token);
				yylval.cp = token;
				prev = '\n';
				return(freturn(STR));
				}
			}
		/* end of section one processing */
		}
	else if(sect == RULESECTION){		/* rules and actions */
		while(!eof){
			switch(c=gch()){
			case '\0':
				return(freturn(0));
			case '\n':
				if(prev == '\n') continue;
				x = NEWE;
				break;
			case ' ':
			case '\t':
				if(sectbegin == TRUE){
					cpyact();
					while((c=gch()) && c != '\n');
					continue;
					}
				if(!funcflag)phead2();
				funcflag = TRUE;
				if(ratfor)fprintf(fout,"%d\n",30000+casecount);
				else fprintf(fout,"case %d:\n",casecount);
				if(cpyact()){
					if(ratfor)fprintf(fout,"goto 30997\n");
					else fprintf(fout,"break;\n");
					}
				while((c=gch()) && c != '\n');
				if(peek == ' ' || peek == '\t' || sectbegin == TRUE){
					warning("Executable statements should occur right after %%");
					continue;
					}
				x = NEWE;
				break;
			case '%':
				if(prev != '\n') goto character;
				if(peek == '{'){	/* included code */
					getl(buf);
					while(!eof && getl(buf) && scomp("%}",buf) != 0)
						fprintf(fout,"%s\n",buf);
					continue;
					}
				if(peek == '%'){
					c = gch();
					c = gch();
					x = DELIM;
					break;
					}
				goto character;
			case '|':
				if(peek == ' ' || peek == '\t' || peek == '\n'){
					if(ratfor)fprintf(fout,"%d\n",30000+casecount++);
					else fprintf(fout,"case %d:\n",casecount++);
					continue;
					}
				x = '|';
				break;
			case '$':
				if(peek == '\n' || peek == ' ' || peek == '\t' || peek == '|' || peek == '/'){
					x = c;
					break;
					}
				goto character;
			case '^':
				if(prev != '\n' && scon != TRUE) goto character;	/* valid only at line begin */
				x = c;
				break;
			case '?':
			case '+':
			case '.':
			case '*':
			case '(':
			case ')':
			case ',':
			case '/':
				x = c;
				break;
			case '}':
				iter = FALSE;
				x = c;
				break;
			case '{':	/* either iteration or definition */
				if(digit(c=gch())){	/* iteration */
					iter = TRUE;
				ieval:
					i = 0;
					while(digit(c)){
						token[i++] = c;
						c = gch();
						}
					token[i] = 0;
					yylval.i = siconv(token);
					munput('c',c);
					x = ITER;
					break;
					}
				else {		/* definition */
					i = 0;
					while(c && c!='}'){
						token[i++] = c;
						c = gch();
						}
					token[i] = 0;
					i = lookup(token,def);
					if(i < 0)
						warning("Definition %s not found",token);
					else
						munput('s',subs[i]);
					continue;
					}
			case '<':		/* start condition ? */
				if(prev != '\n')		/* not at line begin, not start */
					goto character;
				t = slptr;
				do {
					i = 0;
					c = gch();
					while(c != ',' && c && c != '>'){
						token[i++] = c;
						c = gch();
						}
					token[i] = 0;
					if(i == 0)
						goto character;
					i = lookup(token,sname);
					if(i < 0) {
						warning("Undefined start condition %s",token);
						continue;
						}
					*slptr++ = i+1;
					} while(c && c != '>');
				*slptr++ = 0;
				/* check if previous value re-usable */
				for (xp=slist; xp<t; )
					{
					if (strcmp(xp, t)==0)
						break;
					while (*xp++);
					}
				if (xp<t)
					{
					/* re-use previous pointer to string */
					slptr=t;
					t=xp;
					}
				if(slptr > slist+STARTSIZE)		/* note not packed ! */
					error("Too many start conditions used");
				yylval.cp = t;
				x = SCON;
				break;
			case '"':
				i = 0;
				while((c=gch()) && c != '"' && c != '\n'){
					if(c == '\\') c = usescape(c=gch());
					token[i++] = c;
					if(i > TOKENSIZE){
						warning("String too long");
						i = TOKENSIZE-1;
						break;
						}
					}
				if(c == '\n') {
					yyline--;
					warning("Non-terminated string");
					yyline++;
					}
				token[i] = 0;
				if(i == 0)x = NULLS;
				else if(i == 1){
					yylval.i = token[0];
					x = CHAR;
					}
				else {
					yylval.cp = token;
					x = STR;
					}
				break;
			case '[':
				for(i=1;i<NCH;i++) symbol[i] = 0;
				x = CCL;
				if((c = gch()) == '^'){
					x = NCCL;
					c = gch();
					}
				while(c != ']' && c){
					if(c == '\\') c = usescape(c=gch());
					symbol[c] = 1;
					j = c;
					if((c=gch()) == '-' && peek != ']'){		/* range specified */
						c = gch();
						if(c == '\\') c = usescape(c=gch());
						k = c;
						if(j > k) {
							n = j;
							j = k;
							k = n;
							}
						if(!(('A' <= j && k <= 'Z') ||
						     ('a' <= j && k <= 'z') ||
						     ('0' <= j && k <= '9')))
							warning("Non-portable Character Class");
						for(n=j+1;n<=k;n++)
							symbol[n] = 1;		/* implementation dependent */
						c = gch();
						}
					}
				/* try to pack ccl's */
				i = 0;
				for(j=0;j<NCH;j++)
					if(symbol[j])token[i++] = j;
				token[i] = 0;
				p = ccptr;
				if(optim){
					p = ccl;
					while(p <ccptr && scomp(token,p) != 0)p++;
					}
				if(p < ccptr)	/* found it */
					yylval.cp = p;
				else {
					yylval.cp = ccptr;
					scopy(token,ccptr);
					ccptr += slength(token) + 1;
					if(ccptr >= ccl+CCLSIZE)
						error("Too many large character classes");
					}
				cclinter(x==CCL);
				break;
			case '\\':
				c = usescape(c=gch());
			default:
			character:
				if(iter){	/* second part of an iteration */
					iter = FALSE;
					if('0' <= c && c <= '9')
						goto ieval;
					}
				if(alpha(peek)){
					i = 0;
					yylval.cp = token;
					token[i++] = c;
					while(alpha(peek))
						token[i++] = gch();
					if(peek == '?' || peek == '*' || peek == '+')
						munput('c',token[--i]);
					token[i] = 0;
					if(i == 1){
						yylval.i = token[0];
						x = CHAR;
						}
					else x = STR;
					}
				else {
					yylval.i = c;
					x = CHAR;
					}
				}
			scon = FALSE;
			if(x == SCON)scon = TRUE;
			sectbegin = FALSE;
			return(freturn(x));
			}
		}
	/* section three */
	ptail();
# ifdef DEBUG
	if(debug)
		fprintf(fout,"\n/*this comes from section three - debug */\n");
# endif
	while(getl(buf) && !eof)
		fprintf(fout,"%s\n",buf);
	return(freturn(0));
	}
/* end of yylex */
# ifdef DEBUG
freturn(i)
  int i; {
	if(yydebug) {
		printf("now return ");
		if(i < NCH) allprint(i);
		else printf("%d",i);
		printf("   yylval = ");
		switch(i){
			case STR: case CCL: case NCCL:
				strpt(yylval.cp);
				break;
			case CHAR:
				allprint(yylval.i);
				break;
			default:
				printf("%d",yylval.i);
				break;
			}
		putchar('\n');
		}
	return(i);
	}
# endif
#line 733 "y.tab.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack()
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;
    i = yyssp - yyss;
    newss = yyss ? (short *)realloc(yyss, newsize * sizeof *newss) :
      (short *)malloc(newsize * sizeof *newss);
    if (newss == NULL)
        return -1;
    yyss = newss;
    yyssp = newss + i;
    newvs = yyvs ? (YYSTYPE *)realloc(yyvs, newsize * sizeof *newvs) :
      (YYSTYPE *)malloc(newsize * sizeof *newvs);
    if (newvs == NULL)
        return -1;
    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab

#ifndef YYPARSE_PARAM
#if defined(__cplusplus) || __STDC__
#define YYPARSE_PARAM_ARG void
#define YYPARSE_PARAM_DECL
#else	/* ! ANSI-C/C++ */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif	/* ANSI-C/C++ */
#else	/* YYPARSE_PARAM */
#ifndef YYPARSE_PARAM_TYPE
#define YYPARSE_PARAM_TYPE void *
#endif
#if defined(__cplusplus) || __STDC__
#define YYPARSE_PARAM_ARG YYPARSE_PARAM_TYPE YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else	/* ! ANSI-C/C++ */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL YYPARSE_PARAM_TYPE YYPARSE_PARAM;
#endif	/* ANSI-C/C++ */
#endif	/* ! YYPARSE_PARAM */

int
yyparse (YYPARSE_PARAM_ARG)
    YYPARSE_PARAM_DECL
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register const char *yys;

    if ((yys = getenv("YYDEBUG")))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate])) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#if defined(lint) || defined(__GNUC__)
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#if defined(lint) || defined(__GNUC__)
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
#line 40 "parser.y"
{	
# ifdef DEBUG
		if(debug) sect2dump();
# endif
	}
break;
case 3:
#line 48 "parser.y"
{
		if(!funcflag)phead2();
		funcflag = TRUE;
	}
break;
case 4:
#line 53 "parser.y"
{
# ifdef DEBUG
		if(debug) {
			sect1dump();
			sect2dump();
			}
# endif
		}
break;
case 7:
#line 64 "parser.y"
{	scopy(yyvsp[-1],dp);
		def[dptr] = dp;
		dp += slength(yyvsp[-1]) + 1;
		scopy(yyvsp[0],dp);
		subs[dptr++] = dp;
		if(dptr >= DEFSIZE)
			error("Too many definitions");
		dp += slength(yyvsp[0]) + 1;
		if(dp >= dchar+DEFCHAR)
			error("Definitions too long");
		subs[dptr]=def[dptr]=0;	/* for lookup - require ending null */
	}
break;
case 9:
#line 79 "parser.y"
{
# ifdef DEBUG
		if(sect == DEFSECTION && debug) sect1dump();
# endif
		sect++;
		}
break;
case 10:
#line 87 "parser.y"
{	yyval.i = mn2(RNEWE,yyvsp[-1].i,yyvsp[0].i);
		}
break;
case 11:
#line 90 "parser.y"
{	yyval.i = yyvsp[0].i;}
break;
case 12:
#line 93 "parser.y"
{
		if(divflg == TRUE)
			i = mn1(S1FINAL,casecount);
		else i = mn1(FINAL,casecount);
		yyval.i = mn2(RCAT,yyvsp[-1].i,i);
		divflg = FALSE;
		casecount++;
		}
break;
case 13:
#line 102 "parser.y"
{
# ifdef DEBUG
		if(debug) sect2dump();
# endif
		}
break;
case 14:
#line 108 "parser.y"
{	yyval.i = mn0(yyvsp[0].i); }
break;
case 15:
#line 110 "parser.y"
{
		p = yyvsp[0].cp;
		i = mn0(*p++);
		while(*p)
			i = mn2(RSTR,i,*p++);
		yyval.i = i;
		}
break;
case 16:
#line 118 "parser.y"
{	symbol['\n'] = 0;
		if(psave == FALSE){
			p = ccptr;
			psave = ccptr;
			for(i=1;i<'\n';i++){
				symbol[i] = 1;
				*ccptr++ = i;
				}
			for(i='\n'+1;i<NCH;i++){
				symbol[i] = 1;
				*ccptr++ = i;
				}
			*ccptr++ = 0;
			if(ccptr > ccl+CCLSIZE)
				error("Too many large character classes");
			}
		else
			p = psave;
		yyval.i = mn1(RCCL,p);
		cclinter(1);
		}
break;
case 17:
#line 140 "parser.y"
{	yyval.i = mn1(RCCL,yyvsp[0].i); }
break;
case 18:
#line 142 "parser.y"
{	yyval.i = mn1(RNCCL,yyvsp[0].i); }
break;
case 19:
#line 144 "parser.y"
{	yyval.i = mn1(STAR,yyvsp[-1].i); }
break;
case 20:
#line 146 "parser.y"
{	yyval.i = mn1(PLUS,yyvsp[-1].i); }
break;
case 21:
#line 148 "parser.y"
{	yyval.i = mn1(QUEST,yyvsp[-1].i); }
break;
case 22:
#line 150 "parser.y"
{	yyval.i = mn2(BAR,yyvsp[-2].i,yyvsp[0].i); }
break;
case 23:
#line 152 "parser.y"
{	yyval.i = mn2(RCAT,yyvsp[-1].i,yyvsp[0].i); }
break;
case 24:
#line 154 "parser.y"
{	if(!divflg){
			j = mn1(S2FINAL,-casecount);
			i = mn2(RCAT,yyvsp[-2].i,j);
			yyval.i = mn2(DIV,i,yyvsp[0].i);
			}
		else {
			yyval.i = mn2(RCAT,yyvsp[-2].i,yyvsp[0].i);
			warning("Extra slash removed");
			}
		divflg = TRUE;
		}
break;
case 25:
#line 166 "parser.y"
{	if(yyvsp[-3].i > yyvsp[-1].i){
			i = yyvsp[-3].i;
			yyvsp[-3].i = yyvsp[-1].i;
			yyvsp[-1].i = i;
			}
		if(yyvsp[-1].i <= 0)
			warning("Iteration range must be positive");
		else {
			j = yyvsp[-4].i;
			for(k = 2; k<=yyvsp[-3].i;k++)
				j = mn2(RCAT,j,dupl(yyvsp[-4].i));
			for(i = yyvsp[-3].i+1; i<=yyvsp[-1].i; i++){
				g = dupl(yyvsp[-4].i);
				for(k=2;k<=i;k++)
					g = mn2(RCAT,g,dupl(yyvsp[-4].i));
				j = mn2(BAR,j,g);
				}
			yyval.i = j;
			}
	}
break;
case 26:
#line 187 "parser.y"
{
		if(yyvsp[-1].i < 0)warning("Can't have negative iteration");
		else if(yyvsp[-1].i == 0) yyval.i = mn0(RNULLS);
		else {
			j = yyvsp[-2].i;
			for(k=2;k<=yyvsp[-1].i;k++)
				j = mn2(RCAT,j,dupl(yyvsp[-2].i));
			yyval.i = j;
			}
		}
break;
case 27:
#line 198 "parser.y"
{
				/* from n to infinity */
		if(yyvsp[-2].i < 0)warning("Can't have negative iteration");
		else if(yyvsp[-2].i == 0) yyval.i = mn1(STAR,yyvsp[-3].i);
		else if(yyvsp[-2].i == 1)yyval.i = mn1(PLUS,yyvsp[-3].i);
		else {		/* >= 2 iterations minimum */
			j = yyvsp[-3].i;
			for(k=2;k<yyvsp[-2].i;k++)
				j = mn2(RCAT,j,dupl(yyvsp[-3].i));
			k = mn1(PLUS,dupl(yyvsp[-3].i));
			yyval.i = mn2(RCAT,j,k);
			}
		}
break;
case 28:
#line 212 "parser.y"
{	yyval.i = mn2(RSCON,yyvsp[0].i,yyvsp[-1].i); }
break;
case 29:
#line 214 "parser.y"
{	yyval.i = mn1(CARAT,yyvsp[0].i); }
break;
case 30:
#line 216 "parser.y"
{	i = mn0('\n');
		if(!divflg){
			j = mn1(S2FINAL,-casecount);
			k = mn2(RCAT,yyvsp[-1].i,j);
			yyval.i = mn2(DIV,k,i);
			}
		else yyval.i = mn2(RCAT,yyvsp[-1].i,i);
		divflg = TRUE;
		}
break;
case 31:
#line 226 "parser.y"
{	yyval.i = yyvsp[-1].i; }
break;
case 32:
#line 228 "parser.y"
{	yyval.i = mn0(RNULLS); }
break;
#line 1166 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}

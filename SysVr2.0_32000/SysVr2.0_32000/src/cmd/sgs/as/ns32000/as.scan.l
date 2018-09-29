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
alpha			[a-zA-Z_.]
real                    [0-9eE+.-]
digit			[0-9]
octal			[0-7]
hex                     [0-9a-fA-F]
octch                   "\\"[0-7][0-7]?[0-7]?
char                    [^'\"\\]
cchar                   ("\\"?({char}|"\""))|("\\'")|("\\\\")|{octch}
schar                   ("\\"?({char}|"'"))|("\\\"")|("\\\\")|{octch}
ws			[ \t]
%a      3000
%o      4500

%Start REG SDB

%%
"."				{return HERE;}
".def"				{return _DEF;}
".endef"			{return _ENDEF;}
".dim"				{return _DIM;}
".line"				{return _LINE;}
".scl"				{return _SCL;}
".size"				{return _SIZE;}
".tag"				{return _TAG;}
".type"				{return _TYPE;}
".val"				{return _VAL;}
".ln"				{return _LN;}
".space"			{return _SPACE;}
".enter"			{return _ENTER;}
".text"			{return _TEXT;}
".data"			{return _DATA;}
".bss"				{return _BSS;}
".glob""a"?"l"			{return _GLOBAL;}
".set"				{return _SET;}
".comm"				{return _COMM;}
".align"			{return _ALIGN;}
".org"				{return _ORG;}
".ascii"				{return _BYTE;}
".byte"				{return _BYTE;}
".word"				{return _WORD;}
".int"				{return _INT;}
".long"			        {return zflag ? _LONG : _DOUBLE;}
".double"                       {return zflag ? _DOUBLE : _LONG;}
".float"                        {return _FLOAT;}
".file"				{return _FILE;}
<REG>"upsr"			{yylval.unlong = 0x0; return PREG;}
<REG>"fp"			{yylval.unlong = 0x8; return BREG;}
<REG>"sp"			{yylval.unlong = 0x9; return BREG;}
<REG>"sb"			{yylval.unlong = 0xa; return BREG;}
<REG>"psr"			{yylval.unlong = 0xd; return PREG;}
<REG>"intbase"			{yylval.unlong = 0xe; return PREG;}
<REG>"mod"			{yylval.unlong = 0xf; return PREG;}
<REG>"bpr0"			{yylval.unlong = 0x0; return MREG;}
<REG>"bpr1"			{yylval.unlong = 0x1; return MREG;}
<REG>"pf0"			{yylval.unlong = 0x4; return MREG;}
<REG>"pf1"			{yylval.unlong = 0x5; return MREG;}
<REG>"sc"			{yylval.unlong = 0x8; return MREG;}
<REG>"msr"			{yylval.unlong = 0xa; return MREG;}
<REG>"bcnt"			{yylval.unlong = 0xb; return MREG;}
<REG>"ptb0"			{yylval.unlong = 0xc; return MREG;}
<REG>"ptb1"			{yylval.unlong = 0xd; return MREG;}
<REG>"eia"			{yylval.unlong = 0xf; return MREG;}
<REG>"tos"			{return TOS;}
<REG>"ext"			{return EXTR;}
<REG>"i"			{yylval.unlong = 0x01; return CFG;}
<REG>"f"			{yylval.unlong = 0x02; return CFG;}
<REG>"m"			{yylval.unlong = 0x04; return CFG;}
<REG>"c"     			{yylval.unlong = 0x08; return CFG;}
<REG>"b"     			{yylval.unlong = 0x01; return OPT;}
<REG>"w"     			{yylval.unlong = 0x02; return OPT;}
<REG>"u"     			{yylval.unlong = 0x06; return OPT;}
":"{ws}*"b"/"]"			{yylval.unlong = 0; return SCALE;}
":"{ws}*"w"/"]"			{yylval.unlong = 1; return SCALE;}
":"{ws}*"d"/"]"			{yylval.unlong = 2; return SCALE;}
":"{ws}*"q"/"]"			{yylval.unlong = 3; return SCALE;}
<REG>"r"{octal}			{yylval.unlong = yytext[1]-'0'; return IREG;}
<REG>"f"{octal}			{yylval.unlong = yytext[1]-'0'; return FREG;}
{alpha}({alpha}|{digit})*	{return id(yytext);}
0{octal}+                       {yylval.unlong = stoi(1+yytext,8); return INUM;}
0(x|X){hex}+                    {yylval.unlong = stoi(2+yytext,16); return INUM;}
0(d|f|l|D|F|L){real}+           {yylval.undble = atof(2+yytext); return FNUM;}
{digit}+			{yylval.unlong = atoi(yytext); return INUM;}
"'"({cchar})"'"                 {char *s = &yytext[1];
                                    yylval.unlong = esc(&s); return INUM;}
"\""({schar})*"\""              {yylval.unlong = (long) yytext; return STRING;}
":"+				{return COLON;}
";"				{return SEMI;}
","				{return COMMA;}
"*"				{return STAR;}
"("				{return LP;}
")"				{return RP;}
"["				{return LB;}
"]"				{return RB;}
"$"				{return DOLLAR;}
"@"                             {;}
"+"				{return PLUS;}
"-"				{return MINUS;}
"/"				{return DIV;}
"%"				{return MOD;}
"^"				{return XOR;}
"&"				{return AND;}
"|"				{return OR;}
"~"                   {return NOT;}
"<<"				{return LSH;}
">>"				{return RSH;}
"\n"				{return NL;}
{ws}+				{;}
"#"[^\n]*			{;}
.                               {return ERRCH;}
%%

int id(name)
   char *name;
{
   INST *find();
   SYM *lookup();

   if (instr && (yylval.uninst = find(name))) {
      instr = NO;
      return yylval.uninst->itype;
      }
   yylval.unsym = lookup(name, nodefseen | tagseen);
   return IDENT;
}

stoi(p,base)
   char *p;
   int base;
{
   unsigned long n = 0;

   while( *p ) {
      if (*p >= 'a' && *p <= 'f')
         *p += '0' + 10 - 'a';
      else if (*p >= 'A' && *p <= 'F')
         *p += '0' + 10 - 'A';
      else if (*p < '0' || *p > '9')
         break;
      n = n * base + *p++ - '0';
      }
   return n;
}

int esc(str)
   char **str;
{
   char c, *t = *str;

   if (*t != '\\')
      c = *t++;
   else if (*++t >= '0' && *t <= '7') {
      c = (char) stoi(t, 8);
      for (; *t >= '0' && *t <= '7'; t++);
      }
   else
      switch ( c = *t++ ) {
      case 'n':   c = '\n'; break;
      case 't':   c = '\t'; break;
      case 'b':   c = '\b'; break;
      case 'r':   c = '\r'; break;
      case 'f':   c = '\f'; break;
         }
   *str = t;
   return c;
}

yywrap() {
   return 1;
}

set_sdb() {
        BEGIN SDB;
}

set_reg() {
        BEGIN REG;
}

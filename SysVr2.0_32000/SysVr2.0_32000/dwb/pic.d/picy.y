%{
#include	<stdio.h>
#include	"pic.h"
YYSTYPE	y;
%}

%token	<i>	BOX	1
%token	<i>	ARROW	2
%token	<i>	CIRCLE	3
%token	<i>	ARC	4
%token	<i>	ELLIPSE	5
%token	<i>	LINE	6
%token	<i>	MOVE	7
%token	<i>	TEXT	8
%token	<i>	TROFF	9
%token	<i>	SPLINE	10
%token	<i>	BLOCK	11
%token	<i>	BLOCKEND 12
%token	<i>	PRINT
%token	<i>	PLACE
%token	<i>	ATTR
%token	<i>	SPREAD FILL LJUST RJUST ABOVE BELOW
%token	<i>	LEFT RIGHT UP DOWN FROM TO AT BY WITH HEAD CW CCW THEN
%token	<i>	HEIGHT WIDTH RADIUS DIAMETER LENGTH SIZE
%token	<i>	PLACENAME VARNAME DEFNAME CORNER HERE LAST NTH SAME BETWEEN AND
%token	<i>	EAST WEST NORTH SOUTH NE NW SE SW CENTER START END
%token	<i>	DOTX DOTY DOTHT DOTWID DOTRAD
%token	<f>	NUMBER
%token	<i>	DIR
%token	<i>	DOT DASH CHOP
%token	<o>	ST	/* statement terminator */

%left	<f>	'+' '-'
%left	<f>	'*' '/' '%'
%right	<f>	UMINUS

%type	<f>	expr opt_expr
/* this is a lie:  picture and position are really the whole union */
%type	<o>	leftbrace picture piclist position lbracket
%type	<o>	prim place blockname
%type	<i>	textattr last type

%%

top:
	  piclist
	| /* empty */
	| error		{ yyerror("syntax error"); }
	;

piclist:
	  picture
	| piclist picture
	;

picture:
	  prim ST			{ codegen = 1; }
	| leftbrace piclist '}'		{ rightthing($1, '}'); $$ = $2; }
	| PLACENAME ':' picture		{ y.o=$3; makevar($1,PLACENAME,y); $$ = $3; }
	| PLACENAME ':' ST picture	{ y.o=$4; makevar($1,PLACENAME,y); $$ = $4; }
	| PLACENAME ':' position ST	{ y.o=$3; makevar($1,PLACENAME,y); $$=$3; }
        | VARNAME '=' expr ST		{ y.f=$3; makevar($1,VARNAME,y);
					  checkscale($1); }
	| DIR				{ setdir($1); }
	| PRINT expr ST			{ printexpr($2); }
	| PRINT position ST		{ printpos($2); }
	| ST
	;

leftbrace:
	  '{'			{ $$ = leftthing('{'); }
	;

prim:
	  BOX attrlist		{ $$ = boxgen($1); }
	| CIRCLE attrlist	{ $$ = circgen($1); }
	| ELLIPSE attrlist	{ $$ = circgen($1); }
	| ARC attrlist		{ $$ = arcgen($1); }
	| LINE attrlist		{ $$ = linegen($1); }
	| ARROW attrlist	{ $$ = linegen($1); }
	| SPLINE attrlist	{ $$ = splinegen($1); }
	| MOVE attrlist		{ $$ = movegen($1); }
	| TEXT attrlist		{ $$ = textgen($1); }
	| TROFF			{ $$ = troffgen($1); }
	| lbracket piclist ']' { $<o>$=rightthing($1,']'); } attrlist
				{ $$ = blockgen($1, $2, $<o>4); }
	;

lbracket:
	  '['			{ $$ = leftthing('['); }
	;

attrlist:
	  attrlist attr
	| /* empty */		{ makeiattr(0, 0); }
	;

attr:
	  ATTR opt_expr		{ makefattr($1, $2); }
	| DIR opt_expr		{ makefattr($1, $2); }
	| FROM position		{ makeoattr($1, $2); }
	| TO position		{ makeoattr($1, $2); }
	| AT position		{ makeoattr($1, $2); }
	| BY position		{ makeoattr($1, $2); }
	| WITH CORNER		{ makeiattr(WITH, $2); }
	| WITH '.' PLACENAME	{ makeoattr(PLACE, getblock(getlast(1,BLOCK), $3)); }
	| WITH position		{ makeoattr(PLACE, $2); }
	| SAME			{ makeiattr(SAME, $1); }
	| textattr		{ makeiattr($1, 0); }
	| HEAD			{ makeiattr(HEAD, $1); }
	| DOT opt_expr		{ makefattr(DOT, $2); }
	| DASH opt_expr		{ makefattr(DASH, $2); }
	| CHOP opt_expr		{ makefattr(CHOP, $2); }
	| textlist
	;

opt_expr:
	  expr
	| /* empty */		{ $$ = 0; }
	;

textlist:
	  TEXT			{ makeiattr(CENTER, $1); }
	| TEXT textattr		{ makeiattr($2, $1); }
	| textlist TEXT		{ makeiattr(CENTER, $2); }
	| textlist TEXT textattr { makeiattr($3, $2); }
	;

textattr:
	  LJUST
	| RJUST
	| SPREAD
	| FILL
	| CENTER
	| ABOVE
	| BELOW
	;

position:		/* absolute, not relative */
	  place
	| expr ',' expr			{ $$ = makepos($1, $3); }
	| position '+' expr ',' expr	{ $$ = fixpos($1, $3, $5); }
	| position '-' expr ',' expr	{ $$ = fixpos($1, -$3, -$5); }
	| '(' expr ',' expr ')'			{ $$ = makepos($2, $4); }
	| position '+' '(' expr ',' expr ')'	{ $$ = fixpos($1, $4, $6); }
	| position '-' '(' expr ',' expr ')'	{ $$ = fixpos($1, -$4, -$6); }
	| '(' place ',' place ')'	{ $$ = makepos(getcomp($2,DOTX), getcomp($4,DOTY)); }
	| expr '<' position ',' position '>'	{ $$ = makebetween($1, $3, $5); }
	| expr BETWEEN position AND position	{ $$ = makebetween($1, $3, $5); }
	;

place:
	  PLACENAME		{ y = getvar($1); $$ = y.o; }
	| PLACENAME CORNER	{ y = getvar($1); $$ = getpos(y.o, $2); }
	| CORNER PLACENAME	{ y = getvar($2); $$ = getpos(y.o, $1); }
	| HERE			{ $$ = gethere($1); }
	| last type		{ $$ = getlast($1, $2); }
	| last type CORNER	{ $$ = getpos(getlast($1, $2), $3); }
	| CORNER last type	{ $$ = getpos(getlast($2, $3), $1); }
	| NTH type		{ $$ = getfirst($1, $2); }
	| NTH type CORNER	{ $$ = getpos(getfirst($1, $2), $3); }
	| CORNER NTH type	{ $$ = getpos(getfirst($2, $3), $1); }
	| blockname
	| blockname CORNER	{ $$ = getpos($1, $2); }
	| CORNER blockname	{ $$ = getpos($2, $1); }
	;

blockname:
	  last BLOCK '.' PLACENAME	{ $$ = getblock(getlast($1,$2), $4); }
	| NTH BLOCK '.' PLACENAME	{ $$ = getblock(getfirst($1,$2), $4); }
	| PLACENAME '.' PLACENAME	{ y = getvar($1); $$ = getblock(y.o, $3); }
	;

last:
	  last LAST		{ $$ = $1 + 1; }
	| NTH LAST		{ $$ = $1; }
	| LAST			{ $$ = 1; }
	;

type:
	  BOX
	| CIRCLE
	| ELLIPSE
	| ARC
	| LINE
	| ARROW
	| SPLINE
	| BLOCK
	;

expr:
	  expr '+' expr		{ $$ = $1 + $3; }
	| expr '-' expr		{ $$ = $1 - $3; }
	| expr '*' expr		{ $$ = $1 * $3; }
	| expr '/' expr		{ $$ = $1 / $3; }
	| expr '%' expr		{ $$ = (long)$1 % (long)$3; }
	| '-' expr %prec UMINUS	{ $$ = -$2; }
	| '(' expr ')'		{ $$ = $2; }
	| VARNAME		{ $$ = getfval($1); }
	| NUMBER
	| place DOTX		{ $$ = getcomp($1, $2); }
	| place DOTY		{ $$ = getcomp($1, $2); }
	| place DOTHT		{ $$ = getcomp($1, $2); }
	| place DOTWID		{ $$ = getcomp($1, $2); }
	| place DOTRAD		{ $$ = getcomp($1, $2); }
	;

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)chrtbl:chrtbl.c	1.2"
/*
 * 	chrtbl - generate character class definition table
 */
#include <stdio.h>
#include <varargs.h>
#include <string.h>
#include  <signal.h>

/*	Definitions	*/

#define HEX    1
#define OCTAL  2
#define RANGE  1
#define UL_CONV 2
#define SIZE	2 * 257
#define	ISUPPER		1
#define ISLOWER		2
#define ISDIGIT		4 
#define ISSPACE		8	
#define ISPUNCT		16
#define ISCNTRL		32
#define ISBLANK		64
#define ISXDIGIT	128
#define UL		256
#define CHRCLASS	10


/*	Static variables	*/

struct  classname	{
	char	*name;
	int	num;
	char	*repres;
}  cln[]  =  {
	"isupper",	ISUPPER,	"_U",
	"islower",	ISLOWER,	"_L",
	"isdigit",	ISDIGIT,	"_N",
	"isspace",	ISSPACE,	"_S",
	"ispunct",	ISPUNCT,	"_P",
	"iscntrl",	ISCNTRL,	"_C",
	"isblank",	ISBLANK,	"_B",
	"isxdigit",	ISXDIGIT,	"_X",
	"ul",		UL,		NULL,
	"chrclass",	CHRCLASS,		NULL,
	NULL,		NULL,		NULL
};

int	readstd;			/* Process the standard input */
char	linebuf[256], *p = linebuf;	/* Current line in input file */
int	chrclass = 0;			/* set if chrclass is defined */
char	chrclass_name[20];		/* save current chrclass name */
int	chrclass_num;			/* save current chrclass number */
int	ul_conv = 0;			/* set when left angle bracket
					 * is encountered. 
					 * cleared when right angle bracket
					 * is encountered */
int	cont = 0;			/* set if the description continues
					 * on another line */
int	action = 0;			/*  action = RANGE when the range
					 * character '-' is ncountered.
					 *  action = UL_CONV when it creates
					 * the conversion tables.  */
int	in_range = 0;			/* the first number is found 
					 * make sure that the lower limit
					 * is set  */
int	ctype[SIZE];			/* character class and character
					 * conversion table */
int	range = 0;			/* set when the range character '-'
					 * follows the string */
char	tablename[24];			/* save data file name */  
char	*cmdname;			/* Save command name */
char	input[24];			/* Save input file name */

/* Error  messages */
/* vprintf() is used to print the error messages */

char	*msg[] = {
/*    0    */ "Usage: chrtbl [ - | file ] ",
/*    1    */ "the character class \"chrclass\" is not defined",
/*    2    */ "incorrect character class name \"%s\"",
/*    3    */ "--- %s --- left angle bracket  \"<\" is missing",
/*    4    */ "--- %s --- right angle bracket  \">\" is missing",
/*    5    */ "--- %s --- wrong input specification \"%s\"",
/*    6    */ "--- %s --- number out of range \"%s\"",
/*    7    */ "--- %s --- nonblank character after (\"\\\") on the same line",
/*    8    */ "--- %s --- wrong upper limit \"%s\"",
/*    9    */ "--- %s --- wrong character \"%c\"",
/*   10    */ "--- %s --- number expected",
/*   11    */ "--- %s --- too many range \"-\" characters"
};


main(argc, argv)
int	argc;
char	**argv;
{
	int	clean();

	if (cmdname = strrchr(*argv, '/'))
		++cmdname;
	else
		cmdname = *argv;
	if ( (argc != 1)  && (argc != 2) )
		error(cmdname, msg[0]);
	if ( argc == 1 || strcmp(argv[1], "-") == 0 )
		{
		readstd++;
		strcpy(input, "standard input");
		}
	else
		strcpy(input, argv[1]);
	if (signal(SIGINT, SIG_IGN) == SIG_DFL)
		signal(SIGINT, clean);
	if (!readstd && freopen(argv[1], "r", stdin) == NULL)
		perror(argv[1]), exit(1);
	init();
	process();
	if (!chrclass) 
		error(input, msg[1]);
	create1();
	create2();
	exit(0);
}


/* Initialize the ctype array */

init()
{
	register i;
	for(i=0; i<256; i++)
		(ctype + 1)[257 + i] = i;
}

/* Read line from the input file and check for correct
 * character class name */

process()
{

	char	*token();
	register  struct  classname  *cnp;
	register char *c;
	int	ch;
	for (;;) {
		if (fgets(linebuf, sizeof linebuf, stdin) == NULL ) {
			if (ferror(stdin)) {
				perror("chrtbl (stdin)");
				exit(1);	
				}
				break;	
	        }
		p = linebuf;
		/*  comment line   */
		if ( *p == '#' ) 
			continue; 
		/*  empty line  */
		if ( empty_line() )  
			continue; 
		if ( ! cont ) 
			{
			c = token();
			for (cnp = cln; cnp->name != NULL; cnp++) 
				if(strcmp(cnp->name, c) == NULL) 
					break; 
			}	
		switch(cnp->num) {
		default:
		case NULL:
			error(input, msg[2], c);
		case ISUPPER:
		case ISLOWER:
		case ISDIGIT:
		case ISXDIGIT:
		case ISSPACE:
		case ISPUNCT:
		case ISCNTRL:
		case ISBLANK:
		case UL:
				strcpy(chrclass_name, cnp->name);
				chrclass_num = cnp->num;
				parse(cnp->num);
				break;
		case CHRCLASS:
				chrclass++;
				if ( (c = token()) == NULL )
					error(input, msg[1]);
				strcpy(tablename, "\0");
				strcpy(tablename, c);
				if (freopen("ctype.c", "w", stdout) == NULL)
					perror("ctype.c"), exit(1);
				break;
		}
	} /* for loop */
	return;
}

empty_line()
{
	register char *cp;
	cp = p;
	for (;;) {
		if ( (*cp == ' ') || (*cp == '\t') ) {
				cp++;
				continue; }
		if ( (*cp == '\n') || (*cp == '\0') )
				return(1); 
		else
				return(0);
	}
}

/* 
 * token() performs the parsing of the input line. It is sensitive
 * to space characters and returns a character pointer to the
 * function it was called from. The character string itself
 * is terminated by the NULL character.
 */ 

char *
token()
{
	register  char  *cp;
	for (;;) {
	check_chrclass(p);
	switch(*p) {
		case '\0':
		case '\n':
			in_range = 0;
			cont = 0;
			return(NULL);
		case ' ':
		case '\t':
			p++;
			continue;
		case '>':
			if (action == UL)
				error(input, msg[10], chrclass_name);
			ul_conv = 0;
			p++;
			continue;
		case '-':
			if (action == RANGE)
				error(input, msg[11], chrclass_name);
			action = RANGE;
			p++;
			continue;
		case '<':
			if (ul_conv)
				error(input, msg[4], chrclass_name);
			ul_conv++;
			p++;
			continue;
		default:
			cp = p;
			while(*p!=' ' && *p!='\t' && *p!='\n' && *p!='>' && *p!='-')  
				p++;   
			check_chrclass(p);
			if (*p == '>')
				ul_conv = 0;
			if (*p == '-')
				range++;
			*p++ = '\0';
			return(cp);

		}
	}
}


/* conv_num() is the function which converts a hexadecimal or octal
 * string to its equivalent integer represantation. Error checking
 * is performed in the following cases:
 *	1. The input is not in the form 0x<hex-number> or 0<octal-mumber>
 *	2. The input is not a hex or octal number.
 *	3. The number is out of range.
 * In all error cases a descriptive message is printed with the character
 * class and the hex or octal string.
 * The library routine sscanf() is used to perform the conversion.
 */


conv_num(s)
char	*s;
{
	char	*cp;
	int	i, j;
	int	num;
	cp = s;
	if ( *cp != '0' ) 
		error(input, msg[5], chrclass_name, s);
	if ( *++cp == 'x' )
		num = HEX;
	else
		num = OCTAL;
	switch (num) {
	case	HEX:
			cp++;
			for (j=0; cp[j] != '\0'; j++) 
				if ((cp[j] < '0' || cp[j] > '9') && (cp[j] < 'a' || cp[j] > 'f'))
					break;
				
				break;
	case   OCTAL:
			for (j=0; cp[j] != '\0'; j++)
				if (cp[j] < '0' || cp[j] > '7')
					break;
			break;
	default:
			error(input, msg[5], chrclass_name, s);
	}
	if ( num == HEX )  { 
		if (cp[j] != '\0' || sscanf(s, "0x%x", &i) != 1)  
			error(input, msg[5], chrclass_name, s);
		if ( i > 0xff ) 
			error(input, msg[6], chrclass_name, s);
		else
			return(i);
	}
	if (cp[j] != '\0' || sscanf(s, "0%o", &i) != 1) 
		error(input, msg[5], chrclass_name, s);
	if ( i > 0377 ) 
		error(input, msg[6], chrclass_name, s);
	else
		return(i);
}

/* parse() gets the next token and based on the character class
 * assigns a value to corresponding table entry.
 * It also handles ranges of consecutive numbers and initializes
 * the uper-to-lower and lower-to-upper conversion tables.
 */

parse(type)
int type;
{
	char	*c;
	int	ch1, ch2;
	int 	lower, upper;
	while ( (c = token()) != NULL) {
		if ( *c == '\\' ) {
			if ( ! empty_line()  || strlen(c) != 1) 
				error(input, msg[7], chrclass_name);
			cont = 1;
			break;
			}
		switch(action) {
		case	RANGE:
			upper = conv_num(c);
			if ( (!in_range) || (in_range && range) ) 
				error(input, msg[8], chrclass_name, c);
			((ctype + 1)[upper]) |= type;
			if ( upper <= lower ) 
				error(input, msg[8], chrclass_name, c);
			while ( ++lower <= upper ) 
				((ctype + 1)[lower]) |= type;
			action = 0;
			range = 0;
			in_range = 0;
			break;
		case	UL_CONV:
			ch2 = conv_num(c);
			(ctype + 1)[ch1 + 257] = ch2;
			(ctype + 1)[ch2 + 257] = ch1;
			action = 0;
			break;   
		default:
			lower = ch1 = conv_num(c);
			in_range ++;
			if (type == UL) 
				if (ul_conv)
					{
					action = UL_CONV;
					break;
					}
				else
					error(input, msg[3], chrclass_name);
			else
				if (range)
					{
					action = RANGE;
					range = 0;
					}
				else
					;
			
			((ctype + 1)[lower]) |= type;
			break;
		}
	}
	if (action)
		error(input, msg[10], chrclass_name);
}

/* create1() produces a C source file based on the definitions
 * read from the input file. For example for the current ASCII
 * character classification where CHRCLASS=ascii it produces a C source
 * file named ctype.c.
 */


create1()
{
	struct  field {
		char  ch[20];
	} out[8];
	char	*hextostr();
	char	outbuf[256];
	char	*cp;
	int	j;
	int	cond = 0;
	int	flag=0;
	int i, index1, index2;
	int	line_cnt = 0;
	int	len;
	register struct classname *cnp;
	int 	num;


	comment1();
	sprintf(outbuf,"char\t_ctype[] =  { 0,");
	printf("%s\n",outbuf);
	
	index1 = 0;
	index2 = 7;
	while (flag <= 1) {
		for (i=0; i<=7; i++)
			strcpy(out[i].ch, "\0");
		for(i=index1; i<=index2; i++) {
			if ( ! ((ctype + 1)[i]) )  {
				strcpy(out[i - index1].ch, "0");
				continue; }
			num = (ctype + 1)[i];
			if (flag) {      
				strcpy(out[i - index1].ch, "0x");  
				strcat(out[i - index1].ch, hextostr(num));
				continue; }
			while (num)  {
				for(cnp=cln;cnp->num != UL;cnp++) {
					if(!(num & cnp->num))  
						continue; 
					if ( (len=strlen(out[i - index1].ch))  == NULL)  
						strcat(out[i - index1].ch,cnp->repres);
					else  {
						strcat(out[i - index1].ch,"|");
						strcat(out[i - index1].ch,cnp->repres); }  
				num = num & ~cnp->num;  
					if (!num) 
						break; 
				}  /* end inner for */
			}  /* end while */
		} /* end outer for loop */
		sprintf(outbuf,"\t%s,\t%s,\t%s,\t%s,\t%s,\t%s,\t%s,\t%s,",
		out[0].ch,out[1].ch,out[2].ch,out[3].ch,out[4].ch,out[5].ch,
		out[6].ch,out[7].ch);
		if ( ++line_cnt == 32 ) {
			line_cnt = 0;
			flag++; 
			cond = flag; }
		switch(cond) {
		case	1:
			printf("%s\n", outbuf);
			comment2();
			printf("\t0,\n");
			index1++;
			index2++;
			cond = 0;
			break;
		case	2:
			cp = outbuf + strlen(outbuf);
			*--cp = ' ';
			*++cp = '\0';
			printf("%s\n", outbuf);
			printf("};\n");
			break;
		default:
			printf("%s\n", outbuf);
			break;
		}
		index1 += 8;
		index2 += 8;
	}  /* end while loop */
}

/* create2() produces a data file containing the ctype array
 * elements. The name of the file is the same as the value
 * of the environment variable CHRCLASS.
 */


create2()
{
	register   i=0;
	if (freopen(tablename, "w", stdout) == NULL)
		perror(tablename), exit(1);
	for (i=0; i< SIZE; i++)
		printf("%c", ctype[i]);
}

/* Convert a hexadecimal number to a string */


char *
hextostr(num)
int	num;
{
	char	*idx;
	char	buf[64];
	idx = buf + sizeof(buf);
	*--idx = '\0';
	do {
		*--idx = "0123456789abcdef"[num % 16];
		num /= 16;
	  } while (num);
	return(idx);
}

comment1()
{
	printf("#include <ctype.h>\n\n\n");
	printf("\t/*\n");
	printf("\t ************************************************\n");
	printf("\t *		%s  CHARACTER  SET                \n", tablename);
	printf("\t ************************************************\n");
	printf("\t */\n\n");
	printf("\t/* The first 257 characters are used to determine\n");
	printf("\t * the character class */\n\n");
}

comment2()
{
	printf("\n\n\t/* The next 257 characters are used for \n");
	printf("\t * upper-to-lower and lower-to-upper conversion */\n\n");
}


error(va_alist)
va_dcl
{
	va_list	args;
	char	*fmt;

	va_start(args);
	fprintf(stderr, "ERROR in %s: ", va_arg(args, char *));
	fmt = va_arg(args, char *);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
	clean();
}

check_chrclass(cp)
char	*cp;
{
	if (chrclass_num != UL)
		if (*cp == '<' || *cp == '>')
			error(input, msg[9], chrclass_name, *cp);
		else
			;
	else
		if (*cp == '-')
			error(input, msg[9], chrclass_name, *cp);
		else
			;
}

clean()
{
	signal(SIGINT, SIG_IGN);
	unlink("ctype.c");
	unlink(tablename);
	exit(1);
}

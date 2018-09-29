/*	plistg.c 1.2 of 9/29/81
	@(#)plistg.c	1.2
 */

/*   Generates listing   */

#include <stdio.h>

#define LSIZE    100

static char Sccsid[] = "@(#)plistg.c	1.2";

int NF, label;

main()
{
	char lines[LSIZE];
	char *linep = lines;
	int lc;

	lc = 0;
	while(gets(linep) != NULL)  {
		parse(linep);
		if (strlen(linep) > 0 && *linep != '#')  {
			if (NF >= 1 && !label) {
				if(NF > 2) {
					printf("	%s\n",linep);
					continue;
				}
			printf("%o	%s\n",lc,linep);
			lc += 2;
			continue;
			}
		}
		printf("	%s\n",linep);
	}
}
#define BLANK	' '
#define BLACK	'_'
#define WHITE	' '
#define NULLCHR	'\0'
#define TAB	'\t'

parse(lp)
char *lp;
{
	char lastchar;
	register char c;
	int num;

	num = -1;
	lastchar = WHITE;
	label = 0;

	for ( ; *lp != NULLCHR; lp++)  {
		switch(lastchar)  {
			case BLANK:
			case TAB:
				switch (*lp)  {
					case BLANK:
					case TAB:
					   continue;	/* keep gobbling up
							white characters */
					default:	/* new token */
						lastchar = BLACK;
						num++;
						c = *lp;
						continue;
				}
			default:	/* lastchar is BLACK */
				switch (*lp)  {
					case BLANK:
					case TAB:	/* end of current
							token */
						lastchar = WHITE;
						continue;
					default:	/* still another
							nonwhite char */
						c = *lp;
						continue;
				}	/* end inner switch */
		}	/* end outer switch */
	}	/* end of loop */

	/* if lastchar is BLACK, then end last token */
	
	switch(lastchar)  {
		case BLANK:
		case TAB:
		   break;
		default:
			if (c == ':')
				label++;
	}
	NF = ++num;
}

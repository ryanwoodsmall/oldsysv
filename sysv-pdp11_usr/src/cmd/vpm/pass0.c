/*	pass0.c 1.2 of 9/29/81
 */

/*	pass0	   */

#include <stdio.h>

#define EQ(x,y)   (strcmp(x,y) == 0)
#define MAX	32
#define RSIZE	20
#define TEN	10
#define LSIZE	100
#define NTOKNS	5
#define SIZE	1000

static char Sccsid[] = "@(#)pass0.c	1.2";

int	NF,			/* Number of fields in input line */
	label;			/* label flag */
char	*token[NTOKNS];
char	lines[LSIZE];
char	mem[NTOKNS][MAX];

char	*malloc();
int	strlen();
FILE	*fopen(), *fp;

char *ALLOC(len)
int len;
{
	static int avail = -1;
	static char *ptr = 0;
	char *p;
	int sz;

	sz = (len + 1) & (~1);
	if (sz > avail)  {
		if ((ptr = (char *)malloc(SIZE)) == NULL)  {
			printf("no more space for allocation\n");
			exit(2);
		}
		if( (int)ptr & 1 )  {
			ptr++;
			avail = SIZE - 1;
		}
		else
		  avail = SIZE;
	}
	p = ptr;
	ptr += sz;
	avail -= sz;
	return(p);
}

main()
{
	char *linep = lines;
	char *p, *waitlist[RSIZE];
	int nw, w, len;
	char pv1[RSIZE], pv2[RSIZE], pv3[RSIZE];
	char	*prev1 = pv1,	/* storage for previous labels */
		*prev2 = pv2,
		*prev3 = pv3;

	nw = 0;
	if ((fp = fopen("sas_tempb", "w")) == NULL)  {
	   fprintf(stderr,"pass0: Can't write sas_tempb\n");
	   exit(2);
	}

	/* allocate memory for tokens */

	for (w = 0; w < NTOKNS; w++)
		token[w] = mem[w];	/* init pointers for tokens */

	while ( gets(linep) != NULL)  {
			if (*linep == '\0')
			    continue;
	
			parse(linep);
			if (NF == 1 && label)  {
				len = strlen(token[0]) + 1;
				p = ALLOC(len);
				strcpy(p,token[0]);
				waitlist[++nw] = p;
			}
			if (NF >= 1 && !label)  {
				if (nw > 0)  {
					if ( EQ(token[0], "JMP" ))
					    strcpy(prev1, token[1]);
					else  {
						strcpy(prev1, waitlist[nw--]);
						printf("%s:\n", prev1);
						strcpy(prev2, "");
						strcpy(prev3, "");
					}
					while (nw > 0)
						fprintf(fp,"#define %s %s\n",waitlist[nw--], prev1);
				}
				if ( EQ(token[0], "MOV") && EQ(prev3, token[1]) &&
				   ( EQ(prev2, "MOVM") || EQ (prev2, "INC") ))  {
					printf("/* redundant MOV omitted */\n");
					continue;
				}
				if ( EQ(prev2, "JMP") || EQ(prev2, "RETURN") || EQ(prev2, "RETURNI")
				   || EQ(prev2, "HALT") || EQ(prev2, "HALTI") )  {
					printf("/* unreachable code deleted */\n");
					continue;
				}
		
				printf("%s\n", linep);
				if (NF == 1) {		/* inst with no operand */
					strcpy(prev2, token[0]);
					strcpy(prev3, "");
				}
				if (NF > 2)  {
					strcpy(prev2, token[1]);
					strcpy(prev3, token[2]);
				}
				else  {
					strcpy(prev2, token[0]);
					strcpy(prev3, token[1]);
				}
			}
	}
	fclose(fp);
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
	int len, num;

	label = 0;
	num = -1;
	lastchar = WHITE;
	for( ; *lp != NULLCHR; lp++)  {
		switch(lastchar)  {
			case BLANK:
			case TAB:
				switch (*lp)  {
					case BLANK:
					case TAB:
					   continue;	/* keep gobbling up
							white chars */

					default:	/* new token */
						lastchar = BLACK;
						len = 0;
						token[++num][len] = *lp;
						continue;
				}
			default:	/* lastchar = BLACK */
				switch (*lp)  {
					case BLANK:
					case TAB:	/* end of current token */
						lastchar = WHITE;
						token[num][++len] = NULLCHR;
						continue;
					default:	/* still another nonwhite
							character */
						token[num][++len] = *lp;
						continue;
				}	/* end of inner switch */
		}	/* end of outer switch */
	}	/* end of for loop */

	/* if lastchar is BLACK, then end last token */

	switch (lastchar)  {
		case BLANK:
		case TAB:
		   break;
		default:
			if (token[num][len] == ':')  {
				label++;
				token[num][len] = NULLCHR;
			}
			else
			   token[num][++len] = NULLCHR;
	}
	NF = ++num;
}

/*	pass12.c 1.2 of 9/29/81
 */

/*	One-pass Assembler	*/

#include <stdio.h>

/* ST stands for Symbol Table.
   PAP stands for Partially Assembled Program.
   UST stands for Undefined Symbol Table.
   It is a pointer from ST to a corresponding PAP instruction.
   UST_PAP keeps a chain of pointers of instructions with the
   same undefined label.
   UST_DEF keeps a chain of pointers of labels that will have the
   same address (#define statement in sas_tempb) as a result of 
   previous optimization.
   NF stands for number of fields in an input line.
*/

#define MAX	32			/* max token length */
#define SIZE	1000			/* block size alloc for tables */
#define PSIZE	1500			/* PAP size */
#define TABLESZ	1000			/* ST size */
#define LSIZE	80			/* max input line size */
#define NTOKNS	5			/* max number of tokens per line */

#define EQ(x,y)	(!strcmp(x,y))

static char Sccsid[] = "@(#)pass12.c	1.2";

char 	*inst[PSIZE],
	*stsymbol[TABLESZ],
	*token[NTOKNS],
	lines[LSIZE],
	mem[NTOKNS][MAX];

int	staddress[TABLESZ],
	oprand[PSIZE],
	UST_PAP[PSIZE],
	UST[TABLESZ],
	Array[TABLESZ],
        u_dflag[TABLESZ],
	index[TABLESZ],
	UST_DEF[TABLESZ];
char	*malloc();
int	strlen();

int	NF,			/* number of fields in input line */
	plusflg,		/* plus sign flag */
	label;			/* label flag */

char *ALLOC(len)
int len;
{
	static int avail = -1;
	static char *ptr = 0;
	char *p;
	int sz;

	sz = (len + 1) & (~1);
	if (sz > avail)  {
		if((ptr = (char *)malloc(SIZE)) == 0) { 
			fprintf(stderr,"no more space for allocation\n");
			exit(2);  
		}
		if ( (int) ptr & 1) {
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

main(argc,argv)
char **argv;
{
	char *linep = lines;
	char *p;
	char *ALLOC();
	int n,l,w,s,k,temp,temp1,temp2;
	int len;
	int 	Stack = 255,		/* stack size */
		j = 0,			/* ST index */
		i = 0,			/* PAP index */
		nv = 0,			/* current symbol address */
		lc = 0,			/* current label address */
		no = 0;			/* array var flag */

/* Allocate memory for tokens */

	for (w=0; w < NTOKNS; w++)
	    token[w] = mem[w];		/* init pointers for tokens */

	while(gets(linep) != NULL)  {
		if (*linep == '\0')		/* Skip blank line */
		    continue;

		parse(linep);
		if ( EQ(linep, "# 1 \"\"")) {   /* Skip `# 1 ""' line */
		    continue;
              }

		if ( NF > 1)  {
		   if ( NF > 2 )  {
			if ( EQ(token[0], "#define") )  {
			   if ((s = search(token[2],j,0)) == -1)  {
					/* label not in ST */
				len = strlen(token[2]) + 1;
				p = ALLOC(len);
				strcpy(p,token[2]);
				stsymbol[j] = p;
				UST[j] = -1;
				if ((l = search(token[1],j + 1,0)) == -1)  {
					UST_DEF[j] = j + 1;
					len = strlen(token[1]) + 1;
					p = ALLOC(len);
					strcpy(p,token[1]);
					stsymbol[++j] = p;
					UST_DEF[j] = UST[j] = -1;
				}
				else
				    UST_DEF[j] = l;
				j++;
			   }

			   else  {		/* Label found in ST */
				if ((l = search(token[1],j,0)) == -1)  {
					len = strlen(token[1]) + 1;
					p = ALLOC(len);
					strcpy(p,token[1]);
					stsymbol[j] = p;
					UST[j] = UST_DEF[j] = -1;
					while(UST_DEF[s] != -1)
					    s = UST_DEF[s];
					UST_DEF[s] = j++;
				}
				else  {
					while (UST_DEF[s] != -1)
					   s = UST_DEF[s];
					UST_DEF[s] = l;
				}
			   }
			   continue;
			}

			if (EQ(token[1], "BSS"))  {
				len = strlen(token[1]) + 1;
				p = ALLOC(len);
				strcpy(p,token[1]);
				inst[i] = p;
				len = strlen(token[0]) + 1;
				p = ALLOC(len);
				strcpy(p,token[0]);
				stsymbol[j] = p;
				oprand[i] = j;
				UST_PAP[i++] = index[j]  = -1;
				Array[j] = 1;
				staddress[j++] = nv;
				n = atoi(token[2]);
				nv += n;
				continue;
			}
		    }		/* end of NF > 2 */

		    if (EQ(token[0],"JMP") || EQ(token[0],"CALL") ||
		      !strncmp(token[0], "JMP", 3))  {
			len = strlen(token[0]) + 1;
			p = ALLOC(len);
			strcpy(p,token[0]);
			inst[i] = p;
			if ((s = search(token[1],j,0)) == -1)  {
					/* Not in ST */
				len = strlen(token[1]) + 1;
				p = ALLOC(len);
				strcpy(p,token[1]);
				stsymbol[j] = p;
				UST[j] = i;
				oprand[i] = j;
				UST_DEF[j++] = UST_PAP[i++] = -1;
			}

			else  {		/* Found in ST */
				if (!u_dflag[s])  {	/* undefined label */
				    if(UST[s] == -1)
					UST[s] = i++;
				    else  {
					UST_PAP[i] = UST[s];
					UST[s] = i++;
				    }
				}
				else  {		/* Label defined in ST */
				    UST_PAP[i] = -1;
				    oprand[i++] = s;
				}
			   }
		    }			

		    else  {		/* variable names or numbers */
			len = strlen(token[0]) + 1;
			p = ALLOC(len);
			strcpy(p,token[0]);
			inst[i] = p;
			if ((s = search(token[1],j,0)) == -1)  {
					/* Operand not in ST */
				len = strlen(token[1]) + 1;
				p = ALLOC(len);
				strcpy(p,token[1]);
				stsymbol[j] = p;
				oprand[i] = j;
				UST_PAP[i++] = -1;
				if ((*token[1] < '0' || *token[1] > '9') && *token[1] != '\'' )
				/* operand is a variable name */
					staddress[j] = nv++;
				else
				    staddress[j] = -1;  /* indicates it is not 
							   a variable */

				j++;
			}

			else  {		/* Operand found in ST */
			    if (Array[s])  {	/* Variable is an array */
				if (plusflg)  {
				   temp = s;
				   n = atoi(token[2]);
				   while (n != index[s])  { /* Is corresponding index in table already? */
					if((s = search(token[1],j,s+1)) != -1)
					    continue;
					else  {
					   no++;
					   break;
					}
				   }
				   if (no)  {
                                           no = 0;
					   index[j] = n;
					   len = strlen(token[1]) + 1;
					   p = ALLOC(len);
					   strcpy(p,token[1]);
					   stsymbol[j] = p;
					   Array[j] = 1;
					   oprand[i] = j;
					   staddress[j++] = staddress[temp];
				   }
				   else
					oprand[i] = s;
				   plusflg = 0;
				}
				else
				   oprand[i] = s;
				UST_PAP[i++] = -1;
			   }
			   else  {		/* Not an array variable */
				oprand[i] = s;
				UST_PAP[i++] = -1;
			   }
			}
		     }

		     lc += 2;
		}				/* NF > 1 */

		if ( NF == 1)  {
		   if (label)  {
			label = 0;
			if ((s = search(token[0],j,0)) == -1)  {
					/* First appearance of label */
				len = strlen(token[0]) + 1;
				p = ALLOC(len);
				strcpy(p,token[0]);
				stsymbol[j] = p;
				staddress[j] = lc;
				UST[j] = -1;
				u_dflag[j] = 1;
				UST_DEF[j++] = -1;
			}
			else  {			/* Found in ST */
				while (UST_DEF[s] != -1)  {
				   if (UST[s] != -1)  {
					temp2 = UST[s];
					while(UST_PAP[temp2] != -1)  {
						oprand[temp2] = s;
						temp1 = temp2;
						temp2 = UST_PAP[temp2];
						UST_PAP[temp1] = -1;
					}
					oprand[temp2] = s;
				    }
       				    staddress[s] = lc;
   				    u_dflag[s] = 1;
   				    s = UST_DEF[s];
				}
				u_dflag[s] = 1;
				staddress[s] = lc;
				if (UST[s] != -1)  {
					temp2 = UST[s];
					while(UST_PAP[temp2] != -1)  {
					    oprand[temp2] = s;
					    temp1 = temp2;
					    temp2 = UST_PAP[temp2];
					    UST_PAP[temp1] = -1;
					}
					oprand[temp2] = s;
				}
			   }
       		    }

		    else  {		/* Instruction with no operand */
			len = strlen(token[0]) + 1;
			p = ALLOC(len);
			strcpy(p,token[0]);
			inst[i] = p;
			oprand[i++] = -1;
			lc += 2;
		}
	    }
	}

	--nv;
	Stack -= nv;
	printf("#define\tSSTACK\t%d\n",Stack);
	for (k = 0; k < i; k++)  {
		if (EQ(inst[k],"BSS"))
			continue;
		if (EQ(inst[k],"JMP") || EQ(inst[k],"CALL") ||
			!strncmp(inst[k],"JMP",3))  {

		   if (oprand[k] == -1)		/* no operand */
			printf(".byte %s,0\n",inst[k]);
		   else  {
			temp = oprand[k];
			if (!u_dflag[temp])  {
			   printf(".byte %s,..%s\n",inst[k],stsymbol[temp]);
			   fprintf(stderr,"error: label <%s> is undefined\n",
				stsymbol[temp]);
			   continue;
			}
			if ( staddress[temp] > 255 )
			   printf(".byte %s|%d,%d\n", inst[k],staddress[temp] / 256,
				staddress[temp] % 256);
			else
			   printf(".byte %s,%d\n",inst[k],staddress[temp]);
		   }
		}
		else  {		/* all other instructions */
		   if (oprand[k] == -1)		/* no operand */
			printf(".byte %s,0\n",inst[k]);
		   else  {
			temp = oprand[k];
			if (staddress[temp] != -1)  {
			   if (Array[temp] && index[temp] != -1)
				printf(".byte %s,%d+%d\n",inst[k],
					Stack + staddress[temp],index[temp]);
			   else
				printf(".byte %s,%d\n",inst[k],Stack+staddress
					[temp]);
			}
			else
			   printf(".byte %s,%s\n",inst[k],stsymbol[temp]);
		  }
		}
	}
}

search(tok,size,start)
char *tok;
int size,start;
{
	int i;

	if (!size)  return(-1);
	for (i = start; i < size; ++i)
		if (EQ(tok,stsymbol[i])) {
		   return(i);
                }
	return(-1);
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

	num = -1;
	lastchar = WHITE;

	for ( ; *lp != NULLCHR; lp++ )  {
		switch (lastchar)  {
			case BLANK:
			case TAB:
				switch (*lp)  {
					case BLANK:
					case TAB:
					   continue;

					default:  /* new token */
						lastchar = BLACK;
						len = 0;
						token[++num][len] = *lp;
						continue;
				}
			default:	/* last char was BLACK */
				switch (*lp)  {
					case BLANK:
					case TAB:	/* end of current token */
						lastchar = WHITE;
						token[num][++len] = NULLCHR;
						continue;
					default:  /* still another char in 
						     current token */
						if(*lp == '+' && token[num][len] != '\'')
						   plusflg = 1;
						if (!plusflg)
						   token[num][++len] = *lp;
						else {
						   token[num][++len] = NULLCHR;
						   lastchar = WHITE;
						}
						continue;
				}	/* end inner switch */
			}	/* end outer switch */
		}	/* end for loop */

/* if lastchar is BLACK, then terminate last token */

	switch (lastchar) {
		case BLANK:
		case TAB:
			break;
		default:
			if (token[num][len] == ':') {
			   label++;
			   token[num][len] = NULLCHR;
			}
			else
			   token[num][++len] = NULLCHR;
	}

	if (!plusflg)
	  num++;
	NF = num;
}

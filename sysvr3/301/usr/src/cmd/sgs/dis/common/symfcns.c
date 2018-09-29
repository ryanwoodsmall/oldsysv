/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dis:common/symfcns.c	1.3"


#include	<stdio.h>
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"ldfcn.h"
#include	"syms.h"
#include	"dis.h"
#include	"structs.h"

/*
   extsympr(val, pos) finds the external or static symbol name corresponding to 
   the address val and prints the name in the string pointed to by pos.  If no 
   external symbol name is found, then val is printed in the string
*/
extsympr(val,pos)
long val;	/* address of current operand */
char **pos;	/* position in output string */
{
	extern pessymrec	eshashval(); /* in hashfcns.c */

	extern int		oflag;	/* for format of printing */

	pessymrec	prec;  /* pointer to an extern-static hash table entry */
	int		found; /* found is 1 if a symbol-address match was found */

	found=0;
	if (prec= eshashval(val)) {
		do {
			if (val == prec->symval) {
				*pos+=sprintf(*pos,"%s",prec->name);
				found= 1;
				break;
			}
			prec= prec->next;
		} while (prec != NULL);
	}

	if (!found) 
		searchua(val,pos);
}


/*
   eshashval() returns a pointer to the external-static hash table list 
   that might contain an entry whose symval field is equal to val.  If no
   such list exists, then return NULL
*/
pessymrec
eshashval(val)
long	val;
{
	extern pessymrec	eshash[]; /* in hashfcns.c */
	pessymrec	prec;
	int idx;

	idx= comphash(val,ESHTSIZ);
	if (eshash[idx]->next == NULL)
		prec= NULL;
	else
		prec= eshash[idx]->next;
	return(prec);
}


/*
   searchua() searches through the external-static union-array list and checks
   to see if val falls in the range of any of the symbols.  If a match is 
   found then the symbol name + the offset from the beginning of the symbol is
   printed in *pos. Otherwise, val is printed in *pos.
*/
searchua(val,pos)
long val;
char **pos;
{
	extern puarec ualist; /* in hashfcns.c */
	
	puarec	ptrua;
	long	tval;
	long	offset;

	if ((ptrua= ualist->next) != NULL) {
		do {
			tval= ptrua->symval;
			if ( (val >= tval) && (val < tval + ptrua->range)) {
				offset= val - tval;
				if (ISARY(ptrua->type))
					/* if offset not zero, then print name+offset
					   otherwise just print name. This makes the
					   output look better for index modes */
					*pos+=sprintf(*pos,(offset)?"(%s+%ld)":"%s",
						ptrua->name, offset);
				else
					*pos+=sprintf(*pos,"(%s+%ld)",ptrua->name,
						offset);
				return;
			}
			ptrua= ptrua->next;
		} while (ptrua != NULL);
	}
	*pos+= sprintf(*pos, oflag?"0%o":"0x%x",val);
}




/*
   If regno is the frame pointer or argument pointer, then
   locsympr(val, regno, pos) finds the local symbol name corresponding to 
   the address val(regno) and prints the name in the string pointed to by *pos.
   If the symbol to be found is a register variable, then regno will be set
   to -1 and val will hold the register number.  If a register variable is
   found then the symbol name will be printed in the string pointed to by *pos,
   otherwise the register name is printed in pos.
   If regno is not the FP, AP or -1, locsympr() will check to see if val 
   is possibly an external or static array name.
   If no local symbol name or array name is found, then val+regno is 
   printed in the string
*/
locsympr(val, regno, pos)
long val;	/* offset from base register */
int regno;	/* base register */
char **pos;	/* postion in output string */
{
	extern int		oflag;	/* for format of printing */
	extern plocsymrec	lochashval(); /* in hashfcns.c */
	extern char		*regname[];
	extern SCNHDR		scnhdr;

	plocsymrec	prec;  /* pointer to a local hash table entry */
	int		found; /* found is 1 if a symbol-address match was found */

	found=0;
	/* local symbols are desribed only by offsets from the frame ptr. (FP)
	   and the argument ptr. (AP) */
	if ((regno == FPNO) || (regno == APNO)) {
		if (prec= lochashval(val)) {
			do {
				if (((prec->sclass==C_ARG) && (regno != APNO)) ||
					((prec->sclass==C_AUTO) && (regno!=FPNO))) {
					prec= prec->next;
					continue;
				}
				if (val == prec->symval) {
					*pos+=sprintf(*pos,"%s",prec->name);
					found= 1;
					break;
				}
				prec= prec->next;
			} while (prec != NULL);
		}
	
		if (!found) {
			if (searchlocua(val,regno,pos))
				found=1;
		}
	} else if (regno == -1) {
		/* regno is set to -1 only if we are searching for a 
		   register variable */
		if (prec = lochashval(val)) {
			do {
				if (prec->sclass != C_REG) {
					prec= prec->next;
					continue;
				}
				if (val == prec->symval) {
					*pos+=sprintf(*pos,"%s",prec->name);
					found= 1;
					break;
				}
				prec= prec->next;
			} while (prec != NULL);
		}
		if (!found)
			*pos+=sprintf(*pos,"%s",regname[val]);
		return;
	} else if (((unsigned long)val > 
			(unsigned long)((unsigned long)scnhdr.s_paddr+scnhdr.s_size))
					&& (!((val & 0xffff0000) == 0xffff0000))) {
		/* Sometimes an external array is accessed by a displacement 
		   addressing mode where the displacement is the starting address
		   of the array and the base register holds the index.  Since,
		   generally, the data for a text section follows the text section
		   in the object file, the search for the external array name will
		   only occur if the displacement has a value greater than the end
		   of the current text section.  Also, if the number is very large
		   (> 0xffff0000), it is probably a small negative number. */
		*pos+=sprintf(*pos,"(");
		extsympr(val,pos);
		*pos+=sprintf(*pos,"+%s)",regname[regno]);
		found=1;
	}
	if (!found)
		*pos+=sprintf(*pos,"(%ld+%s)",val,regname[regno]);
}


/*
   lochashval() returns a pointer to the local hash table list 
   that might contain an entry whose symval field is equal to val.  If no
   such list exists, then return NULL
*/
plocsymrec
lochashval(val)
long	val;
{
	extern plocsymrec	lochash[]; /* in hashfcns.c */
	plocsymrec	plocrec;
	int idx;

	idx= comphash(val,LOCHTSIZ);
	if (lochash[idx]->next == NULL)
		plocrec= NULL;
	else
		plocrec= lochash[idx]->next;
	return(plocrec);
}


/*
   searchlocua() searches through the local union-array list and checks
   to see if val falls in the range of any of the symbols.  If a match is 
   found then the symbol name + the offset from the beginning of the symbol is
   printed in *pos and 1 is returned. Otherwise, 0 is returned
*/
searchlocua(val,regno,pos)
long val;
int  regno;
char **pos;
{
	extern plocuarec locualist; /* in hashfcns.c */
	
	plocuarec	ptrua;
	long	tval;
	long	offset;

	if ((ptrua= locualist->next) != NULL) {
		do {
			tval= ptrua->symval;
			if ( (val >= tval) && (val < tval + ptrua->range)) {
				if (((ptrua->sclass==C_ARG) && (regno != APNO)) ||
					((ptrua->sclass==C_AUTO) && (regno!=FPNO))) {
					ptrua= ptrua->next;
					continue;
				}
				offset= val - tval;
				if (ISARY(ptrua->type))
					/* if offset not zero, then print name+offset
					   otherwise just print name. This makes the
					   output look better for index modes */
					*pos+=sprintf(*pos,(offset)?"(%s+%ld)":"%s",
						ptrua->name, offset);
				else
					*pos+=sprintf(*pos,"(%s+%ld)",ptrua->name,
						offset);
				return(1);
			}
			ptrua= ptrua->next;
		} while (ptrua != NULL);
	}
	return(0);
}

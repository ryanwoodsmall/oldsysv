/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/vtoc.d/idcmp.c	1.1"
extern char *nodename;
/* <:t-5 d:> */
#include	"vtoc.h"
#define	MAXLEN	15

/*
	idcmp compares id1 and id2 numerically by field, where
field is:

		field1.field2.field3
*/

idcmp(id1,id2)
	char	*id1,*id2;			/* id fields to be compared */
{
	int	id_num1, id_num2;		/* storage for converted fields */
	int	terminal=0;			/* flag to determine smaller id
							   in the case of fields being equal
							   up to a terminal */
	char	*ptr,tmp_num[MAXLEN];	/* pointer and storage for conversion
							   of a field to an integer */

	while (1){

		/* convert field of first id to an integer. */

		ptr=tmp_num;
		for	(; *id1 != '.'; id1++){
			if	(*id1 == '\0'){
				terminal= -1;
				break;
			}
			else	*ptr++= *id1;
		}
		id1++;
		*ptr='\0';
		id_num1=atoi(tmp_num);

		/* convert field of second id to an integer */

		ptr=tmp_num;
		for	(; *id2 != '.'; id2++){
			if	(*id2 == '\0'){
				terminal=1;
				break;
			}
			else	*ptr++= *id2;
		}
		id2++;
		*ptr='\0';
		id_num2=atoi(tmp_num);

		/*
			If the fields are still equal and neither has
		terminated, move on to the next field; otherwise
		return the difference.  If fields are equal until
		one terminates, the shorter is considered smaller.
		*/

		if	(id_num1 != id_num2)
			return(id_num1-id_num2);
		else	if	(terminal)
				return(terminal);
			else	;
	}
}

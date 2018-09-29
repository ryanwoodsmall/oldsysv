/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:include/form.h	1.2"
#define DPLEN	66
#define DPWIDTH	80
#define DNP	1
#define	DLPITCH	6
#define DCPITCH	10
#define DCHSET	NAME_ANY
#define	DRCOLOR	NAME_ANY
#define DCONTYP NAME_SIMPLE
#define ENDENV	"#ENDOF_ENV\n"
#define MANSTR 	"mandatory"

#define NO_FORM		0
#define REG_FORM	1
#define ALLFORMS	2		

/*
 * These are the filenames that may be used for storing a form
 */
#define DESCRIBE	"describe"
#define COMMENT		"comment"
#define ALIGN_PTRN	"align_ptrn"
#define ALERTSH		"alert.sh"
#define ALERTVARS	"alert.vars"

/*
 * These define the appropriate indices in the fspec array
 */
#define FO_PLEN		0
#define FO_PWID		1
#define FO_NP		2
#define FO_LPI		3
#define FO_CPI		4
#define FO_CHSET	5
#define FO_RCOLOR	6
#define FO_CMT	 	7	
#define FO_ALIGN	8	

extern char *fspec[];
# define	NFSPEC	9

struct form {
    	SCALED plen,pwid,lpi,cpi;
    	int np;
    	char *chset;
    	short mandatory;
    	char *rcolor;
    	char *comment;
    	char *conttype;
	char *name;
	};
typedef struct form FORM; 

extern int	linenum;

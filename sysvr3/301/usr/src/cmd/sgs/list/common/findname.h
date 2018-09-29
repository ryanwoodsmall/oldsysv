/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)list:common/findname.h	1.3"
    /*	findname.h
     *
     *	defines the state table (statable[ ][8]) used in scanning a C 
     *	source file to recognize a function name
     *
     *	the state table has 16 states and 8 inputs
     *	      the inputs are tokens recognized by getoken( )
     *        the states represent the parts of a function definition seen 
     */

    /*  LISTER TOKENS */
#define	STAR		0	/* the character:  *  */
#define	LPAR		1	/* the character:  (  */
#define	LISTID		2	/* a string:  [A-Za-z_][A-Za-z_0-9]*  */
#define	RPAR		3	/* the character:  )  */
#define	COMMA		4	/* the character:  ,  */
#define	LBRACE		5	/* the character:  {  */
#define	DONTCARE	6	/* anything else */
#define LISTEOF		7	/* end-of-file */


    /*  Function definitions are expected to conform to the following syntax:
     *  (bracketed items are optional)
     *
     *      LISTID ( [parameter list] ) [declarations] {
     *		- or -
     *      ( [*] LISTID ( [parameter list] )) [declarations] {
     *		- the function name and parameter list may be enclosed in parens
     *
     *
     *  In describing the state table states, the following conventions are used
     *		      0/:  null string
     *		      A*:  zero or more occurrences of string A
     *		      A+:  one or more occurrences of string A
     *		      [A]:  optional string A
     *		      the last token seen is underlined
     */


    /*	LISTER STATES */
#define START	0	/*  0/
			 *  initial state
			 */
#define	BEGLP	1	/*  LISTID* L_P_A_R_ STAR*
			 *  the function name and parameter list may be enclosed
			 *  in parenthesis
			 */
#define	BEGID	2	/*  L_I_S_T_I_D_+
			 *  one or more LISTID's (the last in the series
			 *  being the function name)
			 */
#define	LPID	3	/*  LISTID* LPAR STAR* L_I_S_T_I_D_
			 *  state after BEGLP
			 *  the last LISTID seen may be a function name
			 */
#define IDLP	4	/*  LISTID+ L_P_A_R_
			 *  the LPAR beginning the function parameter list
			 */
#define	LPIDLP	5	/*  LISTID* LPAR STAR* LISTID L_P_A_R_
			 *  the LPAR beginning the function parameter list --
			 *  when the function name and parameter list
			 *  are enclosed in parentheses
			 */
#define	BEGPRM	6	/*  LISTID+ LPAR L_I_S_T_I_D_
			 *  the first parameter in the parameter list -- but
			 *  caution, if the last LISTID is followed by a LPAR
			 *  it might be a function name (enclosed in parens)
			 */
#define	ENDDEF	7	/*  LISTID+ LPAR [ LISTID [ COMMA LISTID ]* ] R_P_A_R_
			 *      - or -
			 *  LISTID* LPAR STAR* LISTID LPAR [ LISTID [ COMMA LISTID ]* ] RPAR R_P_A_R_
			 *  a function name and parameter list -- enclosed in
			 *  parenthesis or not
			 */
#define	LPPRM	8	/*  LISTID* LPAR STAR* LISTID LPAR [ LISTID COMMA ]* L_I_S_T_I_D_
			 *  a parameter in the parameter list of a function name
			 *  ... enclosed in parenthesis
			 */
#define LPEND	9	/* LISTID* LPAR STAR* LISTID LPAR [ LISTID [ COMMA LISTID ]* ] R_P_A_R_
			 *  the RPAR ending the function parameter list
			 */
#define	PRMCMA	10	/*  LISTID+ LPAR [ LISTID C_O_M_M_A_ ]+
			 *  a comma after a parameter:  implies another parameter
			 */
#define	LPCMA	11	/*  LISTID* LPAR STAR* LPAR [ LISTID C_O_M_M_A_ ]+
			 *  a comma after a parameter:  implies another parameter
			 */
#define	PRMLST	12	/*  LISTID+ LPAR LISTID [ COMMA L_I_S_T_I_D_ ]+
			 *  a parameter in the parameter list
			 */
#define	DECL	13	/*  function name ( parameter list ) [ declaration ] L_I_S_T_I_D_
			 *  a LISTID following the parameter list implies a 
			 *  declaration
			 */
#define	ERROR	14	/*  L_I_S_T_E_O_F_
			 *  unexpected end-of-file
			 */
#define	ACCEPT	15	/*  function name ( parameter list ) [ declaration ]* L_B_R_A_C_E_
			 *  the LBRACE opening the function body
			 */



static int	statable[ ][8] = {
/*	TOKENS:
 *	STAR	LPAR	LISTID	RPAR	COMMA	LBRACE	DONTCARE	EOF
 *
 * STATES:
 */
/* START */
    {	START,	BEGLP,	BEGID,	START,	START,	START,	START,		ERROR },
/* BEGLP */
    {	BEGLP,	START,	LPID,	START,	START,	START,	START,		ERROR },
/* BEGID */
    {	START,	IDLP,	BEGID,	START,	START,	START,	START,		ERROR },
/* LPID */
    {	START,	LPIDLP,	START,	START,	START,	START,	START,		ERROR },
/* IDLP */
    {	BEGLP,	START,	BEGPRM,	ENDDEF,	START,	START,	START,		ERROR },
/* LPIDLP */
    {	START,	START,	LPPRM,	LPEND,	START,	START,	START,		ERROR },
/* BEGPRM */
    {	START,	LPIDLP, START,	ENDDEF,	PRMCMA,	START,	START,		ERROR },
/* ENDDEF */
    {	START,	START,	DECL,	START,	START,	ACCEPT,	START,		ERROR },
/* LPPRM */
    {	START,	START,	START,	LPEND,	LPCMA,	START,	START,		ERROR },
/* LPEND */
    {	START,	START,	START,	ENDDEF,	START,	START,	START,		ERROR },
/* PRMCMA */
    {	START,	START,	PRMLST,	START,	START,	START,	START,		ERROR },
/* LPCMA */
    {	START,	START,	LPPRM,	START,	START,	START,	START,		ERROR },
/* PRMLST */
    {	START,	START,	START,	ENDDEF,	PRMCMA,	START,	START,		ERROR },
/* DECL */
    {	START,	START,	DECL,	START,	START,	ACCEPT,	START,		ERROR }
};

/*  RELEVANT STATE TRANSITIONS:  STATE + INPUT  ->  NEXTSTATE
 *
 *	START + LPAR  -> BEGLP
 *	START + LISTID  -> BEGID
 *	START + anything else  ->  START
 *
 *	BEGLP + STAR  ->  BEGLP
 *	BEGLP + LISTID  ->  LPID
 *	BEGLP + anything else  ->  START
 *
 *	BEGID + LISTID  ->  BEGID
 *	BEGID + LPAR  ->  IDLP
 *	BEGID + anything else  ->  START
 *
 *	LPID + LPAR  ->  LPIDLP
 *	LPID + anything else  ->  START
 *
 *	IDLP + STAR  ->  BEGLP
 *	IDLP + LISTID  ->  BEGPRM
 *	IDLP + RPAR  ->  ENDDEF
 *	IDLP + anything else  ->  START
 *
 *	LPIDLP + LISTID  ->  LPPRM
 *	LPIDLP + RPAR  ->  LPEND
 *	LPIDLP + anything else  ->  START
 *
 *	BEGPRM + LPAR  ->  LPIDLP
 *	BEGPRM + COMMA  ->  PRMCMA
 *	BEGPRM + RPAR  ->  ENDDEF
 *	BEGPRM + anything else  ->  START
 *
 *	ENDDEF + LISTID  ->  DECL
 *	ENDDEF + LBRACE  ->  ACCEPT
 *	ENDDEF + anything else  ->  START
 *
 *	LPPRM + RPAR  ->  LPEND
 *	LPPRM + COMMA  ->  LPCMA
 *	LPPRM + anything else  ->  START
 *
 *	LPEND + RPAR  ->  ENDDEF
 *	LPEND + anything else  ->  START
 *
 *	PRMCMA + LISTID  ->  PRMLST
 *	PRMCMA + anything else  ->  START
 *
 *	LPCMA + LISTID  ->  LPPRM
 *	LPCMA + anything else  ->  START
 *
 *	PRMLST + COMMA  ->  PRMCMA
 *	PRMLST + RPAR  ->  ENDDEF
 *	PRMLST + anything else  ->  START
 *
 *	DECL + EOF  ->  EOF
 *	DECL + '='  ->  START
 *	DECL + [ {.*} ] ";"  ->  DECL
 *		possible structure declaration
 *	DECL + LISTID  ->  DECL
 */


/*
*/

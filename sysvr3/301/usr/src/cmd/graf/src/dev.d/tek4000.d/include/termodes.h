/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/include/termodes.h	1.3"
/* <: t-5 d :> */
#define RAWMODE      ioctl(1,TCSETAW,&tm.raw)
#define COOKMODE     ioctl(1,TCSETAW,&tm.cook)
#define ERASE       printf("\033\014")  /*  ESC NP  */
#define INQUIRE     printf("\033\005")  /*  ESC ENQ  */
#define ALPHAMODE   printf("\037")      /*  US  */
#define GINMODE     printf("\033\032")  /*  ESC SUB  */
#define GRAPHMODE   printf("\035")      /*  GS  */

#define SMALLFONT	printf("\033;")
#define GIN 6
#define GRAPH 7

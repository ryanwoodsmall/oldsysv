/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/include/gedstructs.h	1.2"
#include <termio.h>
struct control{
	short change; /* TRUE when display buffer has been altered */
	short textf; /* TRUE when text is to be drawn */
	double wratio; /* window ratio: univ units per screen units */
	struct area w;  /*  window low x,y and high x,y  */
};
struct tekterm{
	int curls,curlw,curmode;
	struct termio cook;
	struct termio raw;
	int alphax,alphay;
};

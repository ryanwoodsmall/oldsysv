/* @(#)opt.h	1.4 */
#include "sys/maus.h"
#if MAUS==0

maus()
{
	nosys();
}

mausinit()
{
	return(0);
}

struct mausmap mausmap[] = {
	-1,	-1
};

#else

struct mausmap mausmap[] = {
	0,	2,		/* 128-byte region */
	2,	128,		/* 8K byte region */
	130,	128,		/* 8K byte region */
	258,	128,		/* 8K byte region */
	-1,	-1
};

#endif

#ifndef X25_0

x25init()
{
	return(0);
}

#endif

#if MESG==0
msgsys(){nosys();}
msginit(){return(0);}
#endif

#if SEMA==0
semsys(){nosys();}
seminit(){}
semexit(){}
#endif

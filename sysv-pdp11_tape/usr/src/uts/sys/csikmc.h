/*	csikmc.h 1.2 of 3/23/82
	@(#)csikmc.h	1.2
 */




/* Reports passed from interpreter */
#define	RRTNXBUF	0
#define	RRTNRBUF	1
#define	RTRACE		2
#define	ERRTERM		3
#define	RTNOK		4
#define RTNSNAP		5
#define RTNER1		6
#define	RTNER2		7
#define	RTNSRPT		8
#define STARTUP		9
#define RTNACK		10
#define ITRACE		11
#define BASEACK		12
#define RTNSTOP		13


/* Commands from the driver to the interpreter */
#define XBUFINCMD	0
#define RBUFINCMD	1
#define RUNCMD		2
#define HALTCMD		3
#define OKCMD		4
#define SCRIPTCMD	5
#define GETECMD		6
#define BASEIN		7



/* interpreter options */
#define HWLOOP 		(01<<1)
#define ADRSWTCH	(01<<2)
#define X25MODE		(01<<0)

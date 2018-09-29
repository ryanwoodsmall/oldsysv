/* @(#)uba.h	1.1 */
#define	ubmdev(X)	(X)
#define	ubmdata(X)	(paddr_t)((unsigned)X)
#define	ubmrev(L, H)	((((int)H&03)<<16)|((int)L&0xffff))

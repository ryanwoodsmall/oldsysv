/* @(#)clock.h	6.1 */
/*
 * VAX clock registers
 */

#define ICCS_RUN 0x1
#define ICCS_TRANS 0x10
#define ICCS_SS 0x20
#define ICCS_IE 0x40
#define ICCS_INT 0x80
#define ICCS_ERR 0x80000000
 
#define	SECHR	(60*60)	/* seconds/hr */
#define	SECDAY	(24*SECHR)	/* seconds/day */
#define	SECYR	(365*SECDAY)	/* seconds/common year */

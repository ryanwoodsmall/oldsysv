/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/diagnostic.h	10.2"

/********************************************************************/
/*                     DIAGNOSTIC HEADER FILE                       */
/********************************************************************/

#define DOWNADDR (MAINSTORE + 0xC000)	/* address for loading dgn files */


	struct request {
		unsigned uncond:1;        /* one is an unconditional   */
					  /* option is given           */
		unsigned rpt:1;           /* one if a rpt is given     */
		unsigned partl:1;         /* one if ph range is given  */
		unsigned dmnd:1;          /* one if a demand request   */
		unsigned long rept_cnt;   /* number of times to repeat */
		unsigned long beg_phsz;   /* beginning phase           */
		unsigned long end_phsz;   /* ending phase              */
	};



#define DEMAND   0x01
#define INTERACT 0x10
#define NORML    0x02
#define END      0xED

	struct phtab{
		unsigned char (*phase)();
		unsigned char type;
		unsigned char title[40];
		};

/* call and return interface for diagnostics */

#define PASS 1
#define FAIL 0
#define NTR 2
#define ENABLE	1	/* enable IO routines */
#define DISABLE	0	/* disable IO routines */


#ifdef m3b2
#define LED 0x3b2led
#else
#define LED (*((unsigned char *)0x20007))
#endif

#define MAXPHSZ 64	/* upper number limit for phases */
#define MAXRPT 65536	/* upper limit repetition of diagnostics */

#define DIRLEN 80	/* size of array for directory names */


/********************************************************************/
/*            Structures for Soak Command Recording                 */
/********************************************************************/


/* There should be as many fboard structures as there are elements
* in the edt.  Each contains the number of times the diagnostics for
* that board were run, and the failure array for up to MAXPHSZ phases
*/

struct result{
	long phases[MAXPHSZ];              /* failing phase numbers        */
	long times;                   /* times the diagnostic was run */
	};


/*********************************************************/
/*	MACROS FOR RESET AND SYSGEN OF PERIPHERALS       */
/*	-definitions from sbd.h and iobd.h used-	 */
/*********************************************************/


	/* variable to reset EDT device number X at opt_slot position */
#define SL_RESET(X)	(IO_SMART(EDTP(X)->opt_slot)->reset)

	/* variable to send INT1 to (smart) device number X at opt_slot for SYSGEN */
#define S_GEN(X)	(IO_SMART(EDTP(X)->opt_slot)->level1)

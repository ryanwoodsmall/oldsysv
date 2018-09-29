/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/todc.c	10.2"
/*
 * This file contains routines to access the hardware
 * time of day clock:  rtodc to read it, wtodc to write it.
 */


#include "sys/sbd.h"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/psw.h"
#include "sys/firmware.h"
#include "sys/sys3b.h"
#include "sys/systm.h"
#include "sys/immu.h"
#include "sys/nvram.h"
#include "sys/todc.h"
#include "sys/cmn_err.h"

#define PASS 1
#define FAIL 0

#define	dysize(A) (((A)%4)? 365: 366)	/* number of days per year */

static int dmsize[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


/*********************************
* rtodc - read time of day clock *
*********************************/

rtodc(hdtime)
register struct todc *hdtime;
{

	register char trys;


	SBDTOD->test = 0;  /* take the clock out of test mode */

	/* stay in this loop until an error free read of the clock is accomplished */
	for (trys = 0; trys < 10; trys++) {

		/* a dummy read to clear any errors */
		hdtime->htenths = SBDTOD->tenths;

		/* read the tenths of a second in bcd into tenths */
		/* if 0x0F is returned, then an error occured so start */
		/* the next loop iteration */
		/*******if ((hdtime->htenths = SBDTOD->tenths & 0x0F) == 0x0F)
			continue; ********/
	
		/* weekday */
		if ((hdtime->hweekday = SBDTOD->dayweek & 0x0F) == 0x0F)
			continue;

		/*
		 * Load the tens and units of the seconds from SBDTOD
		 * into secs. If getseg returns 0 then an error occured.
		 * If all are read with no errors then exit the loop.
		 */

		if (getseg(&(SBDTOD->secs),&hdtime->hsecs))
	  	  if (getseg(&(SBDTOD->mins),&hdtime->hmins))
	    	    if (getseg(&(SBDTOD->hours),&hdtime->hhours))
	      	      if (getseg(&(SBDTOD->days),&hdtime->hdays))
		        if (getseg(&(SBDTOD->months),&hdtime->hmonth))
				break;
		
	}
	if (trys >= 10)
		cmn_err(CE_NOTE,"READ CLOCK -- TOO MANY TRIES\n");
}


/* routine to read in a segment of the clock and check for errors */

getseg(seg,dest)
struct seg *seg;  /* a pointer to the Clock segment to read */
short *dest;	  /* the destination for the data           */
{
	short un;

	if ((*dest = seg->tens & 0x0F) == 0x0F)
		return(FAIL);			/* an error occured */

	else
		*dest = (*dest) * 10;

	/* read the units of the segment */
	if ((un = seg->units & 0x0F) == 0x0F)
		return(FAIL);            /* an error occured */

	else {
		*dest += un;
		return(PASS);
	}

}



/****************************************
* wtodc - write (set) time of day clock *
*	  and month & year to nvram     *
****************************************/

wtodc()
{
	register int d0, d1;
	long hms, day;
	long tim;
	char nv_data[2];	/* buffer for holding data to write to nvram */
	char *nv_addr;		/* address of clock data in nvram */
	extern wnvram();


	tim = time;

	SBDTOD->test = 0;
	SBDTOD->stop_star = 0;

	/* break time into days */

	hms = tim % 86400L;
	day = tim / 86400L;
	if (hms < 0) {
		hms += 86400L;
		day -= 1;
	}

	/* generate hours:minutes:seconds */

	SBDTOD->secs.tens = (hms % 60) / 10;
	SBDTOD->secs.units = (hms % 60) % 10;

	d1 = hms / 60;
	SBDTOD->mins.tens = (d1 % 60) / 10;
	SBDTOD->mins.units = (d1 % 60) % 10;

	d1 /= 60;
	SBDTOD->hours.tens = d1 / 10;
	SBDTOD->hours.units = d1 % 10;

	/*
	 * day is the day number.
	 * generate day of the week.
	 * The addend is 4 mod 7 (1/1/1970 was Thursday)
	 */

	SBDTOD->dayweek = ((day + 7340036L) % 7) + 1;

	/* year number */

	if (day >= 0)
		for (d1=70; day >= dysize(d1); d1++)
			day -= dysize(d1);
	else
		for (d1=70; day < 0; d1--)
			day += dysize(d1-1);
	SBDTOD->years = 1<< (3 - (d1 % 4));
	nv_data[1] = d1;	/* year to write to nvram */

	d0 = day;

	/* generate month */

	if (dysize(d1) == 366)
		dmsize[1] = 29;
	for (d1=0; d0 >= dmsize[d1]; d1++)
		d0 -= dmsize[d1];
	dmsize[1] = 28;

	SBDTOD->days.tens = (d0 + 1) / 10;
	SBDTOD->days.units = (d0 + 1) % 10;

	SBDTOD->months.tens = (d1 + 1) / 10;
	SBDTOD->months.units = (d1 + 1) % 10;
	nv_data[0] = d1 + 1;	/* month to write to nvram */

	SBDTOD->stop_star = 1;	/* start clock */

	nv_addr = (char *) &(UNX_NVR->nv_month);
	wnvram(nv_data, nv_addr, 2);	/* write year & month to nvram */
}

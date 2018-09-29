/*	@(#)dumconfig.c	1.2	*/
/* dummy config() */
/*
 *  config -- update station-id's and device names
 *	from file gcosconfig.
 */

#include "gcos.h"

/* The following parameters are now defined in "gcos.h". */
char	ph_id[3] = PHONE_ID;	/*PHONE_ID must be defined in header file gcos.h*/
char	*phone_id = ph_id;
char	fg_id[3] = FGET_ID;	/*FGET_ID must be defined in header file gcos.h*/
char	*fget_id = fg_id;
char	*du = DEVDU;		/*dataphone or VPM device*/
char	*dn = DEVDN;		/*ACU device*/
char	*numbers[6] = NUMBERS;	/*phone numbers to dial GCOS*/
int	NNUMB = sizeof(numbers)/sizeof(char *);
char	*console = CONSOLE;	/*device for manual dial message*/

config()
{
	return(0);
}

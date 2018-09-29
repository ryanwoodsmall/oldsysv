/*	@(#)gcos.h	1.2	*/
/*	3.0 SID #	1.2	*/
/*  Header file for dpd package, interfacing to HIS
 *	containing all site-dependant definitions.
 *
 *  Installer must define "PHONE_ID" and "FGET_ID", at least,
 *	and check definitions of "DEVDU", "DEVDN", and "NUMBERS".
 */

/*
 * All GCOS station-id's are to be defined as a 2-character string,
 *	e.g.  #define PHONE_ID "XX"
 *
 * For systems with only one path to GCOS, (either spider or dataphone),
 *  only one GCOS station-id is needed, and can be used for both
 *  PHONE_ID and FGET_ID.
 *
 * For systems with two paths to GCOS, (both spider and dataphone),
 *  three GCOS station-ids are needed -- one for each of the
 *  sending daemons, and a third for the fget spooler and the two
 *  fget daemons.
 */

#define	PHONE_ID "PHONE_ID" 	/*GCOS station-id for dataphone sending daemon*/
#define	FGET_ID "FGET_ID" 	/*GCOS station-id for fget and fget daemons*/
			/*  may be the same as PHONE_ID for systems with
			/*  no spider connection. */
#define	SPIDER_ID  "SPIDER_ID"	/*GCOS station-id for spider sending daemon*/


/*  Site-dependant definitions for dataphone interface (subroutine gerts()). */

#define	DEVDU "/dev/du0"	/*dataphone or vpmc device name.*/
#define	DEVDN "/dev/dn1"	/*acu device name.*/
#define	NUMBERS {"4318", "4319", "4320", "4321", 0}
		/* correct phone numbers to dial GCOS at right speed.
		/* NB  Numbers may have to be massaged for individual acu,
		/* by adding '-' for delays, or '<' for end-of-number.
		/* For direct connection, NUMBERS should be null strings.*/
#define	CONSOLE "/dev/console"	/*device to write message for manual dial.*/
		/* NB  For manual dial, DEVDN must be null,
		/* and at least first phone number must be non-null.*/

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:ml/cdump.c	10.3"

#include "sys/sbd.h"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/psw.h"
#include "sys/immu.h"
#include "sys/nvram.h"
#include "sys/firmware.h"
#include "sys/cdump.h"
#include "sys/todc.h"
#define NVRBYTES  512		/* number of bytes of NVRAM dumped */

extern char *mmusrama, *mmusramb, *mmufltcr, *mmufltar, *mmucr;

cdump()
/*******************************************************************
 *
 *	This routine writes to floppy disk the following information:
 *		-the contents of NVRAM
 *		-the contents of the following mmu goodies
 *			-srama
 *			-sramb
 *			-fault code register
 *			-fault address register
 *			-configuration register
 *		-as much of the contents of mainstore as can be stored
 *		 	on the  floppy
 *
 *	procedural outline: (here is basically what the routine does)
 *		-dump the contents of the first 710K bytes of mainstore 
 *			to the floppy (or all of mainstore if the system
 *			contains less than 710K bytes of mainstore)
 *		-write out two more blocks containing:
 *			-contents of NVRAM
 *			-mmu information
 *		-allow the user to put in a second floppy if the
 *			rest of mainstore is to be dumped
 *
 ***************************************************************************/
{	/* cdump */

	char	*memaddr;
	char	*restart;	/* address at start of a floppy */
	int	bindx;		/* counter for blocks written to disk */
	int	indx;	
	struct	crash_hdr	crash_hdr ;
	char	*crash_cptr ;
	int	*crash_iptr ;
	int	blocks_left ;
	int	blks_2_dmp ;
	char	crash_buff[512] ;
	char	*addr ;
	int	seq_flag ;
	char	*crashsanity = CRASHSANITY ;
	
	/* ask the operator if a dump is to be made */
	if (!query("\nDo you want to dump a system image to floppy diskette?"))
	{
		PRINTF("\nNo dump made.\n");
		PRINTF("\nReturning to firmware\n") ;
		return ;
	}

	/* initialize crash header to write to floppy disks */
	
	for (indx = 0; crashsanity[indx] != '\0'; indx++)
		crash_hdr.sanity[indx] = crashsanity[indx] ;
	crash_hdr.sanity[indx] = '\0' ;
	crash_hdr.timestamp = crash_timestamp() ;
	crash_hdr.mem_size = SIZOFMEM ;
	crash_hdr.seq_num = 1 ;
	
	/*
	 * load last 1K of firmware stack with crash header,
	 * MMU information, and the contents of NVRAM
	 */
	
	crash_cptr = (char *) SPMEM - CHDR_OFFSET ; /* SPMEM is start of kernel text */

	*((struct crash_hdr *) crash_cptr) = crash_hdr ;
	crash_cptr += sizeof (struct crash_hdr) ;

	/* set the pointer for mmu info */
	crash_iptr = (int *) crash_cptr;	/* re-use the buffer */
	
	/* read the srams */
	/* SRAMA entries */
	for (indx = 0; indx < 4; indx++)
		*crash_iptr++ = *(int *) (srama + indx);
	
	/* SRAMB entries */
	for (indx = 0; indx < 4; indx++)
		*crash_iptr++ = *(int *) (sramb + indx);
	
	/* get the other goodies, too */
	*crash_iptr++ = *(int *) fltcr;	/* fault code register */
	*crash_iptr++ = *fltar;		/* fault address register */
	*crash_iptr = *(int *) crptr;	/* configuration register */
	
	/* set up pointer for NVRAM */
	crash_cptr = (char *) SPMEM - NVR_OFFSET;
				
	/* read in the NVRAM info */
	/* RNVRAM(NVRaddr, buffaddr, # of bytes) */
	RNVRAM(ONVRAM, crash_cptr, NVRBYTES); 

	if (!query("\nInsert first sysdump floppy."))
	{
		PRINTF("\nNo dump made\n") ;
		PRINTF("\nReturning to firmware.\n") ;
		return ;
	}
	/* make the dump */
	
	PRINTF("\nDumping mainstore\n");

	/* start dump at the beginning of mainstore */
	memaddr = (char *)MAINSTORE;
	restart = memaddr;		/* set initial restart address */
	blocks_left = SIZOFMEM / BLKSZ ;
	
	while (blocks_left > 0)
	{
		*(struct crash_hdr *) crash_buff = crash_hdr ;

		/* set up to write the first block */

		if (crash_hdr.seq_num == 1)
		{
			if (blocks_left > MAXBLKS)
				blks_2_dmp = MAXBLKS ;
			else
				blks_2_dmp = blocks_left ;
		}
		else
		{
			/* allow for block of crash header */
			if (blocks_left > (MAXBLKS - 1))
				blks_2_dmp = MAXBLKS ;
			else
				blks_2_dmp = blocks_left + 1;
		}
		
		/* write the floppy */
		for (bindx = 0; bindx < blks_2_dmp; bindx++)
		{
			if (bindx == 0)
			{
				seq_flag = FIRST ;
				if (crash_hdr.seq_num == 1)
					addr = memaddr ;
				else
					addr = crash_buff ;
			}
			else if (bindx == blks_2_dmp - 1)
			{
				seq_flag = LAST ;
				addr = memaddr ;
			}
			else
			{
				seq_flag = NOCHANGE ;
				addr = memaddr ;
			}
		
			/* write out a block */
			if (FD_ACS(bindx, addr, DISKWT, seq_flag) == FW_FAIL)
			{
				PRINTF("\nUnable to write floppy.\n") ;
				if (!query("\nInsert new floppy for retry."))
				{
					PRINTF("\nDump Terminated.\n");
					PRINTF("%d floppies written\n\n",crash_hdr.seq_num - 1) ;
					PRINTF("Returning to firmware\n") ;
					return ;
				}
				memaddr = restart;	/* restart floppy */
				break ;
			}
			
			/* print some dots for the folks to see */
			if (bindx % 10 == 9) PRINTF(".");
	
			if (bindx != 0 || crash_hdr.seq_num == 1)
				memaddr += BLKSZ ;
		}

		/* for loop terminated abnormally */
		if (bindx != blks_2_dmp)
			continue ;
			
		restart = memaddr;		/* set new restart address */

		if (crash_hdr.seq_num == 1)
			blocks_left -= blks_2_dmp ;
		else
			blocks_left -= blks_2_dmp - 1 ;
			
		if (blocks_left > 0)
		{
			PRINTF("\nIf you wish to dump more of mainstore,\n");
			if (!query("insert new floppy.\n"))
				break ;
				
			crash_hdr.seq_num++ ;
			PRINTF("\nDumping more mainstore\n") ;
		}
	}

	PRINTF("\nDump completed.\n");
	PRINTF("%d floppies written\n",crash_hdr.seq_num) ;
	PRINTF("\nReturning to firmware\n");

}	/* cdump */

int
query(s)

char	*s ;

{
	char	reply[100] ;
	
	PRINTF("%s\n",s) ;
	while (1)
	{
		PRINTF("Enter 'c' to continue, 'q' to quit: ") ;
		GETS(reply) ;
		
		if (STRCMP(reply,"q") == 0)
			return (0) ;

		if (STRCMP(reply,"c") == 0)
			return (1) ;
			
		PRINTF("\nreply \"%s\" unrecognized\n",reply) ;
	}
}


/* This code has been blatently copied from rtodc() */

#define	MAXTRIES 10
#define FAILLOG(msg) { continue ; }
	
int
crash_timestamp()

{
	int		trys ,
			tmp ,
			timestamp ;
	
	/* timestamp is built from the seconds, minutes, hours, and
	 * days fields of the clock converted to seconds
	 */

	for (trys = 0 ; trys < MAXTRIES; trys++)
	{
		/* dummy read to clear errors */
		tmp = SBDTOD->tenths ;
		
		if ((timestamp = SBDTOD->secs.units & 0x0F) == 0x0F)
			FAILLOG("secs.units") ;
		
		if ((tmp = SBDTOD->secs.tens & 0x0F) == 0x0F)
			FAILLOG("secs.tens") ;
		timestamp += 10 * tmp ;

		if ((tmp = SBDTOD->mins.units & 0x0F) == 0x0F)
			FAILLOG("mins.units") ;
		timestamp += 60 * tmp ;
		
		if ((tmp = SBDTOD->mins.tens & 0x0F) == 0x0F)
			FAILLOG("mins.tens") ;
		timestamp += 60 * 10 * tmp ;
		
		if ((tmp = SBDTOD->hours.units & 0x0F) == 0x0F)
			FAILLOG("hours.units") ;
		timestamp += 60 * 60 * tmp ;
		
		if ((tmp = SBDTOD->hours.tens & 0x0F) == 0x0F)
			FAILLOG("hours.tens") ;
		timestamp += 60 * 60 * 10 * tmp ;
		
		if ((tmp = SBDTOD->days.units & 0x0F) == 0x0F)
			FAILLOG("days.units") ;
		timestamp += 24 * 60 * 60 * tmp ;
		
		if ((tmp = SBDTOD->days.tens & 0x0F) == 0x0F)
			FAILLOG("days.tens") ;
		timestamp += 24 * 60 * 60 * 10 * tmp ;

		return(timestamp) ;
	}
	return(-1) ;
}

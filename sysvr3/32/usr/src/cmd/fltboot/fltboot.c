/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)fltboot:fltboot.c	1.5.1.3"
/**********************************************************************/
/*                                                                    */
/*    Function Name: fltboot.c                                        */
/*                                                                    */
/*          Purpose: Interface to Simple Admin Menu for Autoboot      */
/*                                                                    */
/* Input Parameters: none (prompted for from within this code)        */
/*                                                                    */
/*     Return Value: none                                             */
/*                                                                    */
/* Globals Affected: boot command buffer, NVRAM                       */
/*                                                                    */
/**********************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/psw.h>
#include <sys/edt.h>
#include <sys/sys3b.h>
#include <sys/sbd.h>
#include <sys/csr.h>
#include <sys/firmware.h>
#include <sys/boot.h>
#include <sys/immu.h>
#include <sys/nvram.h>

struct load_dev {		/* declaration of local load device table */
	unsigned short edti;
	unsigned short ibd;
	unsigned short slot;
	char name[E_NAMLEN];		/* E_NAMLEN is defined in edt.h */
};
unsigned char def;
#define NUMDEVS 16
struct load_dev ld_dev[NUMDEVS];

#define FALSE 0
#define TRUE 1
#define BREAK -1
#define NEDTP(X) (nedtp + X)


struct bootcmd bcmd;	        /* allocation of boot command buffer */
extern dispedt();               /* function to show edt contents */

extern int errno;
short satos();
struct nvparams	nvparams;
struct fw_nvr fw_nvr;	
short i;
short s;

main()
{
char dev[80], verify[80], buf[80];
char found, printname;
unsigned char numdev, subdev, opt; 
int sd_addr;
char *a;
int k, j;
int xdev;
int num_dev;
struct edt *nedtp;

/* Preliminary read of edt to get size information */

	sys3b(S3BEDT,&k,sizeof(int));

	num_dev = k >> 16;

	j = num_dev * sizeof(struct edt) * (k & 0xffff) * sizeof(struct subdevice) + sizeof(int);

/* allocate amount of space needed as determined from previous two lines
   and malloc below */

	nedtp = (struct edt *)(malloc(j));

	sys3b(S3BEDT,nedtp,j);

	nedtp = (struct edt *) ((char *)nedtp + sizeof(int));


	/* save default program name in dev[] */

	nvparams.addr = (char *)(ONVRAM+FW_OFSET);
 	nvparams.data = (char *)&fw_nvr;
	nvparams.cnt = sizeof(struct fw_nvr);

/* read nvram to get current autoload program data */

	sys3b(RNVR,&nvparams,0);
	if ( errno != 0 )
	{
		printf("ERROR: Not invoked by superuser.\n");
		exit(1);
	}

	printf("\nEnter name of default program for manual load [ %s ]: ",fw_nvr.b_name);
	a=gets(buf);

/* if user enters q or Q or BREAK, return back to simple admin */

	if( (a == NULL) || (*a == BREAK) || (*a == 'q') || (*a == 'Q') )
	{
		printf("\n");
		return;
	}

/* check for a carriage return without any data, if so restore current value
   by re-reading nvram contents into nvparams since it would be cleared out
   by the direct read into nvparams by the gets above. If the user really
   wants to clear nvram they should enter a space char. prior to return */

	if (*a == NULL)
	{
	
		printf("NULL response detected, current value will be retained.\n");
		printf("To clear value, enter space before return.\n");
	}
	else
	{
		if( strcmp(a," ") == 0 ) *a = NULL;
		strcpy(fw_nvr.b_name, a);
	}
	
/* initialize local load device table */

	for( i = 0; i < NUMDEVS; i++ )
	{
			ld_dev[i].edti = 0;
			ld_dev[i].ibd = 0;
			ld_dev[i].slot = 0;
			ld_dev[i].name[0] = '\0';
	}

	for( i = 0; i < num_dev; i++ )
	{
	/* The following is a bit of magic to map the physical pointer
                   address returned by the sys3b call into a virtual address */

		NEDTP(i)->subdev = (i == 0) ?
		(struct subdevice *) ((char *)nedtp + num_dev * sizeof(struct edt)):
		(struct subdevice *) ((char *)NEDTP(i-1)->subdev +
		NEDTP(i-1)->n_subdev * sizeof(struct subdevice));
	}
	numdev = 0;

/* floppy is always presumed present */

	strcpy(ld_dev[numdev].name,NEDTP(0)->subdev[0].name);
	ld_dev[numdev].ibd = TRUE;
	ld_dev[numdev++].edti = FLOPDISK;

	if( ( NEDTP(0)->n_subdev) > 1 )
	{
		strcpy(ld_dev[numdev].name,NEDTP(0)->subdev[1].name);
		ld_dev[numdev].ibd = TRUE;
		ld_dev[numdev++].edti = HARDDISK0;
	}
	if( ( NEDTP(0)->n_subdev) > 2)
	{
		strcpy(ld_dev[numdev].name,NEDTP(0)->subdev[2].name);
		ld_dev[numdev].ibd = TRUE;
		ld_dev[numdev++].edti = HARDDISK1;
	}
	if( ( NEDTP(0)->n_subdev) > 3)
	{
		strcpy(ld_dev[numdev].name,NEDTP(0)->subdev[1].name);
		ld_dev[numdev].ibd = TRUE;
		ld_dev[numdev++].edti = HARDDISK0;
		strcpy(ld_dev[numdev].name,NEDTP(0)->subdev[2].name);
		ld_dev[numdev].ibd = TRUE;
		ld_dev[numdev++].edti = HARDDISK1;
	}

/* scan EDT for bootable peripherals */

	for( i = 1; i < num_dev; i++ )
	{
			if( NEDTP(i)->boot_dev )     /* boot device if set */
			{
				strcpy(ld_dev[numdev].name,NEDTP(i)->dev_name);
				ld_dev[numdev].ibd = FALSE;
				ld_dev[numdev].slot = NEDTP(i)->opt_slot;
				ld_dev[numdev++].edti = i;
			}
			else if( (NEDTP(i)->dev_name[0] == '\0') || 
					(!strcmp(NEDTP(i)->dev_name,"*VOID*")) )
			{
				/* boot device if name is null or "*VOID*" */
				/* ld_dev[i].name is left NULL */

				ld_dev[numdev].ibd = FALSE;
				ld_dev[numdev].slot = NEDTP(i)->opt_slot;
				ld_dev[numdev++].edti = i;
			}
	}

/* print possiblities and prompt for device */

	printf("\tPossible load devices are:\n\n");
	printf("Option Number    Slot     Name\n");
	printf("---------------------------------------\n");
	for( i = 0; i < numdev; i++ )
	{
		printf("      %2d         %2d",i,ld_dev[i].slot);
		if( strcmp(ld_dev[i].name,"*VOID*") )
			printf("       %-10s", ld_dev[i].name);
		printf("\n");
	}
		
	def = 0;
	for( i=0; i < numdev; i++ )
	{
		if( ld_dev[i].ibd )
		{
			if( fw_nvr.b_dev == ld_dev[i].edti )
			{
				def = i;
				break;
			}

		}
		else if( ld_dev[i].slot == (fw_nvr.b_dev >> 4) )
		{
			def = i;
			break;
		}
	}


retry:
	printf("enter number corresponding to autoload device desired [ %d ]:",def);

	a=gets(buf);
	if ((*a == BREAK) || (*a == 'q') || (*a == 'Q') )
	{
		printf("\n");
		printf("Quit request received, no load values will be changed \n");
		exit(0);
	}

/* if the user entered a return here it could be that only the program name
   is to be changed or a subdevice of the default autoload device. 
   should be changed  Print default message and continue processing */

	if( *a == NULL )
	{
		printf("NULL response detected, current value will be maintained. \n\n");
		i = def;
	}
	else
	{
		i = satos(buf);
		if( (i < 0) || (i >= numdev) )
		{
			printf("\n%s is not a valid option number.\n", buf);
			goto retry;
		}
	}

	if ( ld_dev[i].ibd )
	{
/* If its an integral we got all we need . Just put the edti data in 
   and call wrapup to write nvram and print closing messages */

		fw_nvr.b_dev = ld_dev[i].edti;
		wrapup();
		exit(0);
	}

/* If the found board is not integral, we must shift the slot number
   4 bits to the left to combine it with subdevice info that will
   be collected and stored later. */

	xdev = ld_dev[i].slot << 4;

	if( def == i )	subdev = fw_nvr.b_dev &0xF;
	else		subdev = 0;

	i = ld_dev[i].edti;	/* chosen edt index */

/* If we get here it's a board with subdevices.  Set things up
   and let the user choose a subdevice */

/* initialize local bootdev table */

	for( s = 0; s < num_dev; s++ )
	{
		ld_dev[s].edti = 0;
		ld_dev[s].ibd = 0;
		ld_dev[s].slot = s;
		ld_dev[s].name[0] = '\0';
	}

/* determine possible subdevices */

	if( (numdev = NEDTP(i)->n_subdev) == 0 )
	{
		numdev = 15;
	}
	else
	{
		for( s = 0; s < numdev; s++ )
			strcpy(ld_dev[s].name,NEDTP(i)->subdev[s].name);
	}

/* print possiblities and prompt for subdevice */

	printf("Possible subdevices are:\n\n");
	printf("Option Number   Subdevice    Name\n");
	printf("--------------------------------------------\n");
	for( s = 0; s < numdev; s++ )
	{
		printf("      %2d          %2d", s, ld_dev[s].slot);
		if( strcmp(ld_dev[s].name,"*VOID*") )
			printf("         %-10s", ld_dev[s].name);
		printf("\n");
	}

/* Determine if the default subdevice from NVRAM 
   is currently a valid device in option table	 */

	def = 0;	/* make option 0 the default: arbitrary choice */

	if( subdev < numdev )	def = subdev;

tryagain:
	printf("\nEnter Subdevice Option Number ");
	printf("[%d",def);
	if( strcmp(ld_dev[def].name,"*VOID*") )
		printf("(%s)",ld_dev[def].name);
	printf("]: ");

	a = gets(buf);
	if( (*a == BREAK) || (*a == 'q') || (*a == 'Q') )
	{
		printf("\n");
		printf("Quit request received.  No load values will be changed.\n");
		exit(0);
	}

	if( *a == NULL )
	{
		printf("Null response. Current value will be maintained.\n");
		s = def;
	}
	else
	{
		s = satos(buf);
		if( (s < 0) || (s >= numdev) )
		{
			printf("\n%s is not a valid option number.\n",buf);
			goto tryagain;
		}
	}

	def = s;

	xdev |= def;
	fw_nvr.b_dev = xdev;
	wrapup();
	exit(0);
}



/* convert string to short - returns -1 if non decimal character is found */

short
satos(s)
char *s;
{
	char c;
	short i, x;

	i = x = 0;
	while( (c = *s++) != '\0' )
	{
		if( ('0' <= c) && (c <= '9') )	x = (c - '0');
		else return( (short) -1);

		i = i * 10 + x;
	}
	return(i);
}
wrapup()
{
	sys3b(WNVR,&nvparams,0);
	printf("\n LOAD PARAMETER UPDATE COMPLETE \n");
}


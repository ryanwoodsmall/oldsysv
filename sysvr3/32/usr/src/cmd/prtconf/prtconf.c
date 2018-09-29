/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)prtconf:prtconf.c	1.7"
#include	<stdio.h>
#include	<ctype.h>
#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/edt.h>
#include	<sys/libxedt.h>
#include	<sys/signal.h>
#include	<sys/sys3b.h>

extern struct mainedt	*getedt();

main (argc,argv)
int	argc;
char	*argv[];
{
	int 	i;
	int	j;
	int 	edt_size;
     
	char 	name[E_NAMLEN];
	char 	command[64];

     	struct mainedt		*edtptr;
	struct subdevice	*Subdev;
     
	printf ("AT&T 3B2 SYSTEM CONFIGURATION:\n\n");

	if ((i = sys3b (S3BMEM)) >= (1024 * 1024))
		printf ("Memory size: %d Megabytes\n", i / (1024 * 1024));
	else
		printf ("Memory size: 512 Kilobytes\n");

	printf ("System Peripherals:\n\n");

	/* Get the main EDT */
	if ((edtptr = getedt ()) == NULL) {
		exit(2);
	}
	/*  set pointer to the first subdevice entry in EDT  */
	Subdev = (struct subdevice *)(&edtptr->mainedt[edtptr->edtsze.esize]);

	/* print device and subdevice data */
	printf ("        Device Name");
 	printf ("        Subdevices");
 	printf ("           Extended Subdevices\n\n");

     	for (i = 0; i < edtptr->edtsze.esize; ++i) {
		printf ("        %s\n", edtptr->mainedt[i].dev_name);

		if (edtptr->mainedt[i].indir_dev) { 
			fflush (stdout);

			for (j = 0; j < E_NAMLEN && edtptr->mainedt[i].dev_name[j] != '\0'; j++) {
				name[j] = tolower(edtptr->mainedt[i].dev_name[j]);
			}
			name[j] = '\0';
			sprintf (command, "/etc/prtconf.d/%s %d", name, edtptr->mainedt[i].opt_slot);
			system (command);

			for (j = 0; j < edtptr->mainedt[i].n_subdev; j++)
				Subdev++;

			continue;
		}
		while (edtptr->mainedt[i].n_subdev--) {

			if (strncmp (edtptr->mainedt[i].dev_name, "CTC", 3) == 0) {
				Subdev++;	/* do not print subdevices for
				continue;	 * the CTC since the edt table
						 * is kludged to always contain
						 * both floppy tape and disk
						 * devices if CTC is available
						 */
			}
			else if (strcmp(edtptr->mainedt[i].dev_name , "SBD") == 0 &&
					(strncmp (Subdev->name, "FD", 2) == 0)) {
				printf("                        Floppy Disk\n");
			}
			else if ((strcmp (edtptr->mainedt[i].dev_name, "SBD") == 0 &&
					(strncmp (Subdev->name, "HD", 2) == 0)) ||
				(strcmp (edtptr->mainedt[i].dev_name, "XDC") == 0 &&
					(strncmp (Subdev->name, "HD", 2 ) == 0))) {
				printf ("                        %d Megabyte Disk\n", 
							dsksize (Subdev->name));
			}
			else {
				printf ("                %s\n", Subdev->name);
			}
			if (strcmp (edtptr->mainedt[i].dev_name , "CTC") != 0)
				Subdev++;
		}
     	}
	/*  determine if MAU present */
	/*  Ignore SIGSYS signal set due to prtconf use  */
	/*  in pre_1.2 software with XDC  */

	signal (SIGSYS, SIG_IGN);

	sys3b (S3BFPHW, &i);

	if (i == 1)
		printf ("        MAU\n");

	signal (SIGSYS, SIG_DFL);

     	if (edtptr != NULL)
		free (edtptr);

     	exit (0);
} /* main */

dsksize (disk)
char	*disk;
{
	char	first[10];
	char	size[10];
	char	last[10];
	int 	numb;

	/*  pull out the numbers from the string for the size of the disk */

	sscanf (disk, "%[A-Z,a-z]%[0-9]%[A-Z,a-z]", first, size, last);

	numb = atoi (size);

	return (numb);
} /* dsksize */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)prtconf:prtconf.c	1.6"
#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/edt.h>
#include <sys/signal.h>
#include <sys/sys3b.h>

extern int errno;

main (argc,argv)
int	argc;
char	*argv[];
{
	int 	i;
	int 	edt_size;
	int	memsize;
	int	m = 0;
     
	struct edtsze
	{
		short esize;
		short ssize;
	} edtsize;

	struct myedt
	{
		struct edtsze mysize;
		struct edt myedt[1];
		  };

	int	name_size = E_NAMLEN;

     	struct myedt	*edtptr = NULL;
	struct subdevice	*Subdev;
     



	printf("AT&T 3B2 SYSTEM CONFIGURATION:\n\n");
	memsize = sys3b(S3BMEM);
	if ( memsize >= (1024 * 1024) )
		printf("Memory size: %d Megabytes\n", memsize/(1024 *1024));
	else
		printf("Memory size: 512 Kilobytes\n");
	printf("System Peripherals:\n\n");

	/* determine size of EDT */

     	if(sys3b(S3BEDT,&edtsize,sizeof(int)) != 0)
	{
		perror("S3BEDT failed\n");
		exit(1);
	}
    
     	edt_size = edtsize.esize * sizeof(struct edt) + edtsize.ssize * sizeof(struct subdevice) + sizeof(int);

	/*  allocate buffer to read EDT */

     	if((edtptr = (struct myedt *)malloc(edt_size)) == NULL)
	{
		perror("Unabled to allocate memory for edt\n");
		exit(2);
	}

     	if(sys3b(S3BEDT,edtptr,edt_size) != 0)
		{
		perror ("S3BEDT failed\n");
		exit(3);
	}

	/*  set pointer to the first subdevice entry in EDT  */

	Subdev = (struct subdevice *)(&edtptr->myedt[edtsize.esize]);

	/* print device and subdevice data */
	printf("        Device Name");
	printf("        Subdevices\n\n");

     	for(i = 0; i < edtsize.esize; ++i)
	{

		printf("        %s\n", edtptr->myedt[i].dev_name);
		while ( edtptr->myedt[i].n_subdev-- )
		{

			if ( strncmp(edtptr->myedt[i].dev_name, "CTC", 3 ) == 0 )
			{
				Subdev++;	/* do not print subdevices for
				continue;	 * the CTC since the edt table
						 * is kludged to always contain
						 * both floppy tape and disk
						 * devices if CTC is available
						 */
			}
			else if ( strcmp(edtptr->myedt[i].dev_name , "SBD" ) == 0 && (strncmp(Subdev->name, "FD", 2 ) == 0) )
				printf("                        Floppy Disk\n");

			else if ( (strcmp(edtptr->myedt[i].dev_name, "SBD" ) == 0 && (strncmp(Subdev->name, "HD", 2 ) == 0) )  || 
(strcmp(edtptr->myedt[i].dev_name, "XDC" ) == 0 && (strncmp(Subdev->name, "HD", 2 ) == 0) ) )
			{
				printf("                        %d Megabyte Disk\n", 
						dsksize(Subdev->name));
			}
			else
				printf("                %s\n", Subdev->name);

			if ( strcmp(edtptr->myedt[i].dev_name , "CTC" ) != 0 )
				Subdev++;
		}
     	}
		/*  determine if MAU present */
		/*  Ignore SIGSYS signal set due to prtconf use  */
		/*  in pre_1.2 software with XDC  */

		signal(SIGSYS, SIG_IGN);

		sys3b(S3BFPHW, &m);
		if (m == 1)
			printf("        MAU\n");

		signal(SIGSYS, SIG_DFL);


     	if(edtptr != NULL)
		free(edtptr);

     	exit(0);

}

dsksize(disk)
char	*disk;
{

	char	first[10];
	char	size[10];
	char	last[10];
	int 	numb;


	/*  pull out the numbers from the string for the size of the disk */

	sscanf(disk, "%[A-Z,a-z]%[0-9]%[A-Z,a-z]", first, size, last);

	numb = atoi(size);

	return(numb);


}

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)gettype:gettype.c	1.1"

#include <sys/pump.h>
#include <sys/extbus.h>
#include <stdio.h>
#include <errno.h>

extern int errno;

main(argc,argv)
int 	argc;
char 	*argv[];

{

int fd,i;
struct bus_type retptr;

	if ((fd = open(argv[argc-1],O_PUMP)) == -1 )
	{
		fprintf(stderr,"Open fail.  errno = %d\n",errno);
		exit(1);
	}
	if ((ioctl(fd,B_GETTYPE,&retptr)) != -1) 
	{
		printf("%s",retptr.bus_name);
		close(fd);
		exit(0);
	}
	else 
		{
		fprintf(stderr,"ioctl failed.  errno = %d\n",errno);
		fprintf(stderr,"Format command not supported for this device");
		close(fd);
		exit(1);
		}
}

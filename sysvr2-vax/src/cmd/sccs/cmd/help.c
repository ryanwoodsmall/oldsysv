#include	<stdio.h>
#include	<sys/types.h>
#include	<macros.h>

SCCSID(@(#)help.c	1.1);

char	Ohelpcmd[]   =   "/usr/lib/help/lib/help2";
extern	int	errno;

main(argc,argv)
int argc;
char *argv[];
{
	execv(Ohelpcmd,argv);
	fprintf(stderr,"help: Could not exec: %s.  Errno=%d\n",Ohelpcmd,errno);
	exit(1);
}

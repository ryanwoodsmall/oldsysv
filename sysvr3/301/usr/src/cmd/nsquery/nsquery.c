/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nsquery:nsquery.c	1.7"
#include  <stdio.h>
#include  <ctype.h>
#include  <nserve.h>

void	exit();
static	char	*cmd;
extern	int	errno;
extern	int	optind,	opterr;
extern	char	*optarg;
extern	char	*dompart();
extern	char	*namepart();

main(argc,argv)
int	argc;
char	*argv[];
{

	int	chr;
	int	errflag = 0, hflag = 0;
	char	*name = "*";
	char 	*usage = "nsquery [-h] [<name>]";


	cmd = argv[0];

	while ((chr = getopt(argc,argv,"h")) != EOF)
		switch(chr) {
	 	case 'h':
			if (hflag)
				errflag = 1;
			else
				hflag = 1;	
			break;
		case '?':
			errflag = 1;
			break;
		}

	if (errflag || argc > optind + 1) {
		fprintf(stderr,"Usage: %s\n",usage);
		exit(1);
	}

	if (argv[optind] != NULL) {
		name = argv[optind];
		verify_name(name);
	}
	
	if (!hflag)
		fprintf(stdout,"RESOURCE        ACCESS      SERVER                    DESCRIPTION\n\n");

	if (ns_info(name) == FAILURE) {
		nserror(cmd);
		exit(1);
	}
	exit(0);
}

static
verify_name(name)
char	*name;
{

	char	*mach;
	char	*domain;
	int	qname = 0, dname = 0;

	if (name[strlen(name)-1] == SEPARATOR)
		dname = 1;

	if (*(domain = dompart(name)) != '\0') {
		qname = 1;
		if (strlen(domain) > SZ_DELEMENT) {
			fprintf(stderr,"%s: domain name %s<%s> exceeds <%d> characters\n",cmd,dname ? "":"in ",name,SZ_DELEMENT);
			exit(1);
		}

		if (v_dname(domain) != 0) {
			fprintf(stderr,"%s: domain name %s<%s> contains invalid characters\n",cmd,dname ? "":"in ",name);
			exit(1);
		}
	}

	if (*(mach = namepart(name)) != '\0') {
		if (strlen(mach) > SZ_MACH) {
			fprintf(stderr,"%s: nodename %s<%s> exceeds <%d> characters\n",cmd,qname ? "in ":"",name,SZ_MACH);
			exit(1);
		}

		if (v_uname(mach) != 0) {
			fprintf(stderr,"%s: nodename %s<%s> contains invalid characters\n",cmd,qname ? "in ":"",name);
			exit(1);
		}
	}
}

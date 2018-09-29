/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)uname:uname.c	1.17"
#include        <stdio.h>
#include        "sys/utsname.h"

#if u3b2
#include	<string.h>
#include        "sys/types.h"
#include	"sys/psw.h"	/* psw.h:  nvram info, must follow "sys/types.h" */
#include        "sys/sbd.h"
#include        "sys/firmware.h"
#include	"sys/param.h"
#include	"sys/immu.h"
#include        "sys/nvram.h"
#include        "sys/sys3b.h"
#define ROOTUID 0
#endif

struct utsname  unstr, *un;

extern void exit();
extern int fprintf();
extern int uname();
extern int optind;
extern char *optarg;

main(argc, argv)
char **argv;
int argc;
{
#if u3b2
        char nv_data[9]; /* buffer for holding data read from nvram */
        char *nv_addr;  /* address of sysname data in nvram */
        struct nvparams nvp;

	int Sflg=0;
	char *optstring="asnrvmS:";
#else
	char *optstring="asnrvm";
#endif

	char *sysname;
        int sflg=0, nflg=0, rflg=0, vflg=0, mflg=0, errflg=0, optlet;

        un = &unstr;
        uname(un);

        while((optlet=getopt(argc, argv, optstring)) != EOF)
		switch(optlet) {
		case 'a':
			sflg++; nflg++; rflg++; vflg++; mflg++;
			break;
		case 's':
			sflg++;
			break;
		case 'n':
			nflg++;
			break;
		case 'r':
			rflg++;
			break;
		case 'v':
			vflg++;
			break;
		case 'm':
			mflg++;
			break;
#if u3b2
		case 'S':
			Sflg++;
			sysname = optarg;
			break;
#endif

		case '?':
			errflg++;
	        }

        if(errflg || (optind != argc))
		usage();

#if u3b2
	if((Sflg > 1) || 
	   (Sflg && (sflg || nflg || rflg || vflg || mflg))) {
		usage();
	}

        /* If we're changing the system name */
	if(Sflg) {
                if (getuid() != ROOTUID) {
                        if (geteuid() != ROOTUID) {
                                (void) fprintf(stderr, "uname: not super user\n");
                                exit(1);
                        }
                }
                /* name must contain less than 9 characters */
		if(strlen(sysname) > 8) {
                        (void) fprintf(stderr, "uname: name must be <= 8 letters\n");
                        exit(1);
                }
                /* replace name in nvram */
                strcpy(nv_data, sysname);
                nv_addr = (char *) &(UNX_NVR->sys_name[0]);
                nvp.addr = nv_addr;
                nvp.data = nv_data;
                nvp.cnt = sizeof(UNX_NVR->sys_name);
                sys3b(WNVR, &nvp, 0);

                /* replace name in kernel data section */
                sys3b(SETNAME, nv_data, 0);
                exit(0);
        }
#endif
						    /* "uname -s" is the default */
        if( !(sflg || nflg || rflg || vflg || mflg))
		sflg++;
        if(sflg)
                (void) fprintf(stdout, "%.*s", sizeof(un->sysname), un->sysname);
        if(nflg) {
                if(sflg) (void) putchar(' ');
                (void) fprintf(stdout, "%.*s", sizeof(un->nodename), un->nodename);
        }
        if(rflg) {
                if(sflg || nflg) (void) putchar(' ');
                (void) fprintf(stdout, "%.*s", sizeof(un->release), un->release);
        }
        if(vflg) {
                if(sflg || nflg || rflg) (void) putchar(' ');
                (void) fprintf(stdout, "%.*s", sizeof(un->version), un->version);
        }
        if(mflg) {
                if(sflg || nflg || rflg || vflg) (void) putchar(' ');
                (void) fprintf(stdout, "%.*s", sizeof(un->machine), un->machine);
        }
        (void) putchar('\n');
        exit(0);
}

usage()
{
#if u3b2
	(void) fprintf(stderr, "usage:  uname [-snrvma]\n\tuname [-S system name]\n");
#else
	(void) fprintf(stderr, "usage:  uname [-snrvma]\n");
#endif

	exit(1);
}

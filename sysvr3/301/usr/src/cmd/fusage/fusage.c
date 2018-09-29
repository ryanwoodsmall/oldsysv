/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)fusage:fusage.c	1.15"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <mnttab.h>
#include <sys/utsname.h>
#include <nserve.h>
#include <ctype.h>
#include "fumount.h"

#define MNTTAB "/etc/mnttab"
#define ADVTAB "/etc/advtab"
#define REMOTE 2

char *malloc();

#ifndef SZ_PATH
#define SZ_PATH 128
#endif
struct advlst {
	char resrc[SZ_RES+1];
	char dir[SZ_PATH+1];
} *advlst;

struct clnts *client;
struct mnttab *mnttab;
int nmount;

main(argc, argv)
int argc;
char *argv[];
{
	char *resrc, *adv_dir, str[SZ_RES+SZ_MACH+SZ_PATH+20];
	int nadv, mtab, clntsum, advsum, local, prtflg, fswant, i, j, k;
	FILE *atab;
	struct stat stbuf;
	struct statfs fsi;
	struct utsname myname;

	for(i = 0; i < argc; i++)
		if(argv[i][0] == '-') {
			printf("Usage: %s [mounted file system]\n", argv[0]);
			printf("          [advertised resource]\n");
			printf("          [mounted block special device]\n");

			exit(1);
		}
	uname(&myname);
	printf("\nFILE USAGE REPORT FOR %.6s\n\n",myname.nodename);

		/* load /etc/mnttab data */

	if(stat(MNTTAB,&stbuf) == -1) {
		fprintf(stderr,"fusage: cannot stat %s.  ", MNTTAB);
		perror("");
		exit(1);
	}
	if((mnttab = (struct mnttab *)malloc(stbuf.st_size)) == 0) {
		fprintf(stderr,"fusage: cannot get memory for mnttab\n");
		exit(1);
	}
	if((mtab = open(MNTTAB,0)) == -1) {
		fprintf(stderr,"fusage: cannot open %s",MNTTAB);
		perror("open");
		exit(1);
	}
	if((read(mtab,mnttab,stbuf.st_size)) == -1) {
		fprintf(stderr,"fusage: cannot read %s",MNTTAB);
		perror("read");
		exit(1);
	}
	nmount = stbuf.st_size / sizeof(struct mnttab);

		/* load resource and directory names from /etc/advtab */

	if(stat(ADVTAB,&stbuf) == -1) {
		nadv = 0;
		goto noadv;
	}
	if((atab = fopen(ADVTAB,"r")) == 0) {
		fprintf(stderr,"fusage: cannot open %s",ADVTAB);
		perror("fopen");
		exit(1);
	}
		/* we are not going to store the complete line for each entry
		   in /etc/advtab.  Only the resource name and the path name
		   are important here (client names are unnecessary).
		*/
		/* count lines in advtab */
	nadv = 0;
	while(getline(str,sizeof(str),atab) != 0) 
		nadv++;
	if((advlst = (struct advlst *)malloc(nadv * sizeof(struct advlst))) 
									== 0) {
		fprintf(stderr,"fusage: cannot get memory for advtab\n");
		exit(1);
	}
	freopen(ADVTAB,"r",atab);	/* rewind */
		/* load advlst from advtab */
	i = 0;
	while((getline(str,sizeof(str),atab) != 0) && (i < nadv)) {
		loadadvl(str, &advlst[i++]);
	}
	if(nlload()) 
		exit(1);
		/* for each entry in the /etc/mnttab,
		   if the mount is a local file system, find advertised
		   resources that are within it.  Collect and print data 
		   on each remote, then print the info for the file system.

		   Even though we may be requested to print a subset of the
		   mounted resources, we must always execute all of the loop 
		   in order to get data for items that are printed.
		*/
noadv:
	for(i = 0; i < nmount; i++) {
		if(mnttab[i].mt_ro_flg & REMOTE)
			continue;
		if(wantprint(argc,argv,mnttab[i].mt_dev,mnttab[i].mt_filsys)) {
			printf("\n\t%-15s      %s\n",mnttab[i].mt_dev, 
						mnttab[i].mt_filsys);
			fswant++;
		} else
			fswant = 0;
		advsum = 0;
		if(statfs(mnttab[i].mt_filsys,&fsi,sizeof(struct statfs),0)<0){
			fsi.f_bsize = 1000;	/* force report in blocks */
		}
		for(j = 0; j < nadv; j++) {
			if(isinfs(i, advlst[j].dir)) {
				prtflg = wantprint(argc, argv, advlst[j].resrc, 
								advlst[j].dir);
				if(prtflg)
					printf("\n\t%15s", advlst[j].resrc);

					/* get client list */
				switch(getnodes(advlst[j].resrc, 0)) {
				case 1:
					if(prtflg)
						printf(" (%s) not in kernel advertise table\n",
							advlst[j].dir);
					continue;
					break;
				case 2:
					if(prtflg)
						printf(
						" (%s) ...bad data\n",
							advlst[j].dir);
					continue;
					break;
				case 3:
					if(prtflg)
						printf(
						" (%s) ...no clients\n",
							advlst[j].dir);
					continue;
				}
				if(prtflg)
					printf("      %s\n", advlst[j].dir);
				clntsum = 0;
				for(k = 0; client[k].flags != EMPTY; k++) {
					if(prtflg)
						prdat(client[k].node, 
							client[k].bcount,
							fsi.f_bsize);
					clntsum += client[k].bcount;
				}
				if(prtflg)
					prdat("Sub Total",clntsum,fsi.f_bsize);
				advsum += clntsum;
			}
		}
		if(fswant){
			printf("\n\t%15s      %s\n", "",  mnttab[i].mt_filsys);
			if((local = getcount(mnttab[i].mt_filsys)) != -1)
				prdat(myname.nodename, local, fsi.f_bsize);
		}
		if(!wantprint(argc,argv,mnttab[i].mt_dev,mnttab[i].mt_filsys))
			continue;
		if(advsum > 0) {
			prdat("Clients", advsum, fsi.f_bsize);
			prdat("TOTAL", local + advsum, fsi.f_bsize);
		}
	}
	for(i = 1; i < argc; i++)
		if(argv[i][0] != '\0')
			printf("'%s' not found\n", argv[i]);
	exit(0);
}
#define EQ(X,Y) !strcmp(X,Y)
wantprint(argc, argv, r1, r2)
int argc;
char *argv[], *r1, *r2;
{				/* if user specified resources to print,
				   find out if either r1 or r2 were requested */
	int	found, i;

	found = 0;
	if(argc == 1)
		return(1);	/* the default is "print everything" */
	for(i = 0; i < argc; i++)
		if(EQ(r1,argv[i]) || EQ(r2,argv[i])) {
			argv[i][0] = '\0';	/* done with this arg */
			found++;		/* continue scan to find */
		}				/* duplicate requests */
	return(found);
}

prdat(s,n, bsize)
char *s;
short bsize;
{
	char *tag;

	if(bsize == 1000)
		tag = "Blocks";
	else
		tag = "Kb";
	printf("\t\t\t%15s %10d %s\n", s, n * bsize / 1000, tag);
}

isinfs(n,advdir)
char *advdir;
{			/* is 'advdir' part of mountpoint n? */
			   
	struct stat mpstat, advstat;

	stat(mnttab[n].mt_filsys, &mpstat);
	stat(advdir, &advstat);
	if(advstat.st_dev == mpstat.st_dev)
		return(1);
	return(0);
}

getline(str,len,fp)
char *str;
int len;
FILE *fp;
{
			/* function to read up to 'len' characters from
			   the file (/etc/advtab) and toss remaining
			   characters up to a newline.  If the last line
			   is not terminated in a '\n', the funcion returns
			   failure even if some data was read.  Since
			   the adv(1M) command always terminates the
			   lines with a '\n', this is not a problem. */
	int i, c;
	char *s;

	s = str; 
	i=1;
	for(;;) {
		c = getc(fp);
		switch (c) {
		case EOF:
			*s = '\0';
			return(0);
		case '\n':
			*s = '\0';
			return(1);
		default:
			if(i < len)
				*s++ = c;
			i++;
		}
	}
}

loadadvl(s,advx)
char *s;
struct advlst *advx;
{
	int i;

	i = 0;
	while(isspace(*s)) s++;
	while(!isspace(*s) && (i < SZ_RES)) advx->resrc[i++] = *s++;
	advx->resrc[i] = '\0';

	i = 0;
	while(isspace(*s)) s++;
	while(!isspace(*s) && (i < SZ_MACH+SZ_PATH+1)) advx->dir[i++] = *s++;
	advx->dir[i] = '\0';
}


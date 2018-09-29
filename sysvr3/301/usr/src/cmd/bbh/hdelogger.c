/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bbh:hdelogger.c	1.3"

#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/vtoc.h"
#include "sys/hdelog.h"
#include "sys/hdeioctl.h"
#include "sys/stat.h"
#include "hdecmds.h"
#include "edio.h"
#include <stdio.h>
#include <time.h>
#include "sys/signal.h"

extern int errno;
char * cmdname;
int prtmode;
#define PRSMMRY	0
#define PRFULL	1
dev_t devarg = -1;
int initflg;
int notdemon = 1;

main(argc, argv)
char *argv[];
{
	register int rval;

	if (argc < 1) cmdname = "hdeadd";
	else cmdname = argv[0];
	edinit();
	if (getppid() == 1) dodemon();
	if ( argc < 2) doalled();
	argc--;
	argv++;
	while(argc > 0) {
		if (argv[0][0] != '-') badusage();
		switch(argv[0][1]) {
			case 'i':
				initflg = 1;
				argc--;
				argv++;
				break;
			case 's':
				prtmode = PRSMMRY;
				argc--;
				argv++;
				break;
			case 'f':
				prtmode = PRFULL;
				argc--;
				argv++;
				break;
			case 'D':
				rval = getdev(argc, argv);
				argc -= rval;
				argv += rval;
				break;
			default:
				badusage();
		}
	}
	if (initflg) {
		if (devarg == -1)
			doallinit();
		else if (do1init())
			exit (ERREXIT);
		exit(NORMEXIT);
	}
	if (devarg == -1) doalled();
	if (do1ed()) exit(ERREXIT);
	exit(NORMEXIT);
}

cmdusage()
{
	fprintf(stderr,
		"usage:\t%s [-s] [-f] [-D maj min]\n",
		cmdname);
}

doallinit()
{
	int cnt;
	int i;
	int rval;

	edinit();
	cnt = 0;
	rval = 0;
	for (i = 0; i < edcnt; i++) {
		devarg = edtable[i];
		rval |= do1init();
	}
	if (notdemon && rval) exit(ERREXIT);
}

do1init()
{
	if (edopen(devarg)) return (-1);
	if (edgetel()) {
		edclose();
		return(-1);
	}
	if (edhdelp->l_valid == HDEDLVAL) {
		edclose();
		return(0);
	}
	if (edvalel(0)) {
		edclose();
		return(-1);
	}
	edclose();
	return(0);
}

doalled()
{
	int cnt;
	int i;
	int rval;

	edinit();
	cnt = 0;
	rval = 0;
	for (i = 0; i < edcnt; i++) {
		devarg = edtable[i];
		rval |= do1ed();
	}
	if (rval) exit(ERREXIT);
	exit(NORMEXIT);
}

char *
fixdate(tm)
time_t tm;
{
	char *cp;

	cp = ctime(&tm);
	cp[24] = 0;
	cp += 4;
	return(cp);
}

do1ed()
{
	register int i;
	register struct hderecord *rp;
	register ecnt;

	if (prtmode == PRFULL) {
		printf("\nDisk Error Log: Full Report for maj=%d min=%d\n",
			major(devarg), minor(devarg));
	}
	if (edopen(devarg)) return(-1);
	if (edgetel()) {
		edclose();
		return(-1);
	}
	if (edhdelp->l_valid != HDEDLVAL) {
		printf("%s: Disk does not contain a valid error log\n", cmdname);
		printf("\tdisk maj=%d min=%d\n", major(devarg), minor(devarg));
		edclose();
		return(-1);
	}
	if (prtmode == PRFULL) {
		printf("\tlog created:  %s", ctime(&edhdelp->createdt));
		printf("\tlast changed: %s", ctime(&edhdelp->lastchg));
		printf("\tentry count: %d\n", edhdelp->logentct);
		if (!edhdelp->logentct) {
			printf("\tno errors logged\n");
			edclose();
			return(0);
		}
	printf("    phys blkno  cnt     first occurrence       last occurrence\n");
		for (ecnt = 0, i = 0, rp = edhdelp->errlist;
		     i < edhdelp->logentct; rp++, i++) {
			ecnt += rp->reportct;
			printf("%2d: %10d %4d", i, rp->blkaddr, rp->reportct);
			printf("   %s", fixdate(rp->firsttime));
			printf("   %s\n", fixdate(rp->lasttime));
		}
		printf("TOTAL: %d errors logged\n", ecnt);
	} else {
		printf("Disk maj=%d min=%d: ", major(devarg), minor(devarg));
		if (edhdelp->logentct) {
			for (ecnt = 0, i = 0, rp = edhdelp->errlist;
			     i < edhdelp->logentct; rp++, i++)
				ecnt += rp->reportct;
			printf("%d errors logged\n", ecnt);
		} else printf("no errors logged\n");
	}
	edclose();
	return(0);
}

badusage()
{
	fprintf(stderr, "bad command usage\n");
	cmdusage();
	exit(INVEXIT);
}


getdev(ac, av)
char *av[];
{
	int maj, min;
	int rcnt;
	long maxmaj, maxmin;
	dev_t maxdev = -1;

	rcnt = 2;
	if (ac < 2) badusage();
	if (av[0][0] == '-') {
		if (av[0][1] != 'D' || ac < 3) badusage();
		rcnt++;
		av++;
	}
	if (sscanf(av[0], "%d", &maj) != 1
		|| sscanf(av[1], "%d", &min) != 1) badusage();
	maxmaj = major(maxdev);
	maxmin = minor(maxdev);
	if (maj < 0 || maj > maxmaj) {
		fprintf(stderr,
			"major device number %d out of range (0 to %d)\n",
			maj, maxmaj);
		exit(INVEXIT);
	}
	if (min < 0 || min > maxmin) {
		fprintf(stderr,
			"minor device number %d out of range (0 to %d)\n",
			min, maxmin);
		exit(INVEXIT);
	}
	devarg = makedev(maj, min);
	return(rcnt);
}

/* DEMON CODE from here on down */

int drepflg;

setdrep()
{
	signal(SIGTERM, SIG_IGN);
	drepflg = 1;
}

dodemon()
{

	notdemon = 0;
	cmdname = "Disk Error Daemon";
	setpgrp();
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, setdrep);
	doallinit();
	demonrep();
	for(;;) {
		if (drepflg) demonrep();
		while (edrptslp() == -2)
			if (drepflg) demonrep();
		addreps();
	}
}

demonrep()
{
	register int i, j;
	register struct hderecord *rp;
	register ecnt;

	for (i = 0; i < edcnt; i++) {
		if (edopen(edtable[i])) {
			printf("%s: Disk maj=%d min=%d: cannot access it\n",
				cmdname, major(edtable[i]), minor(edtable[i]));
			continue;
		}
		if (edgetel()) {
			printf("%s: Disk maj=%d min=%d: cannot access error log\n",
				cmdname, major(edtable[i]), minor(edtable[i]));
			edvalel(1);
			edclose();
			continue;
		}
		if (edhdelp->l_valid != HDEDLVAL) {
			printf("%s: Disk does not contain a valid error log\n",
				cmdname);
			printf("\tdisk maj=%d min=%d\n",
				major(edtable[i]), minor(edtable[i]));
			edvalel(1);
			edclose();
		}
		if (edhdelp->logentct) {
			printf("%s: Disk maj=%d min=%d: ",
				cmdname, major(edtable[i]), minor(edtable[i]));
			for (ecnt = 0, j = 0, rp = edhdelp->errlist;
			     j < edhdelp->logentct; rp++, j++)
				ecnt += rp->reportct;
			printf("%d errors logged\n", ecnt);
		}
		edclose();
	}
	drepflg = 0;
	signal(SIGTERM, setdrep);
}

struct hdedata newrep;

addreps()
{
	register int rval;

	/* this is simple-minded for now (1 at a time)
	 * but the code is easier that way and that
	 * should be the dominant case, except when
	 * something more basic is wrong.
	 */
	for (;;) {
		if ((rval = edgetrpts(&newrep, 1)) != 1) {
			if (!rval)	/* done with new reports */
				return(0);
			if (rval == -2)	/* between the devil and deep blue sea */
				continue;	/* log the new ones first */
			printf("%s: exiting to let \"init\" rerun me\n",
				cmdname);
			exit(ERREXIT);
		}
		if (eddevck(newrep.diskdev)) {
			printf("%s: unknown disk: maj=%d min=%d\n",
				cmdname,
				major(newrep.diskdev), minor(newrep.diskdev));
			printf("\tits error report being discarded\n");
			goto discard;
		}
		if (newrep.blkaddr == edgetpdsno(newrep.diskdev)) {
			printf("%s: CRITICAL ERROR: physical description sector %d\n",
				cmdname, newrep.blkaddr);
			printf("\tis bad on Disk maj=%d min=%d\n",
				major(newrep.diskdev), minor(newrep.diskdev));
			printf("\tcontact your service representative\n");
			goto discard;
		}
		if (edopen(newrep.diskdev)) {
			printf("%s: cannot access Disk maj=%d, min=%d\n",
				cmdname, major(newrep.diskdev),
				minor(newrep.diskdev));
discmsg:
			printf("\tdiscarding error report for block %d\n",
				newrep.blkaddr);
discard:
			edclose();
			if (edclrrpts(&newrep, 1) < 0) {
				printf("%s: cannot clear error report\n",
					cmdname);
				printf("%s: exiting to let \"init\" rerun me\n",
					cmdname);
				exit(ERREXIT);
			}
			continue;
		}
		if (edpdck())
			goto discmsg;
		if (memcmp(newrep.dskserno, edpdp->pdinfo.serial, 12)) {
			printf("%s: media serial number of disk maj=%d min=%d\n",
				cmdname, major(newrep.diskdev), minor(newrep.diskdev));
			printf("\tdisagrees with serial number in error report\n");
			goto discmsg;
		}
		if (newrep.blkaddr >= edpdp->pdinfo.defectst &&
		    newrep.blkaddr < edpdp->pdinfo.defectst
					+ (edpdp->pdinfo.defectsz+edsecsz-1)/edsecsz) {
			printf("%s: CRITICAL ERROR: defect mapping table\n",
				cmdname);
			printf("\tis bad on Disk maj=%d min=%d\n",
				major(newrep.diskdev), minor(newrep.diskdev));
			printf("\tcontact your service representative\n");
			goto discmsg;
		}
		if (newrep.blkaddr >= edpdp->pdinfo.errlogst &&
			newrep.blkaddr < edpdp->pdinfo.errlogst
					+ (edpdp->pdinfo.errlogsz +edsecsz-1)/edsecsz) {
			printf("%s: CRITICAL ERROR: error log\n", cmdname);
			printf("\tis bad on Disk maj=%d min=%d\n",
				major(newrep.diskdev), minor(newrep.diskdev));
			printf("\tcontact your service representative\n");
			goto discmsg;
		}
		if (edgetel()) {
			printf("%s: cannot access error log for disk maj=%d min=%d\n",
				cmdname, major(newrep.diskdev),
				minor(newrep.diskdev));
			goto discmsg;
		}
		if (edhdelp->l_valid != HDEDLVAL) {
			if (edvalel(1))
				goto discmsg;
		}
		add1rep();
		if (edputel()) {
			printf("%s: cannot write error log for disk maj=%d min=%d\n",
				cmdname, major(newrep.diskdev),
				minor(newrep.diskdev));
			goto discmsg;
		}
		printf("%s: successfully logged error for block %d on disk maj=%d min=%d\n",
			cmdname,
			newrep.blkaddr,
			major(newrep.diskdev),
			minor(newrep.diskdev));
		goto discard;
	}
}

struct hderecord nrec, zerorec;

add1rep()
{
	register struct hderecord *rp, *r2p, *r3p, *ep;
	register int i;
	register int rcnt;

	nrec.blkaddr = newrep.blkaddr;
	nrec.readtype = newrep.readtype;
	nrec.severity = newrep.severity;
	nrec.badrtcnt = newrep.badrtcnt;
	nrec.reportct = 1;
	nrec.firsttime = nrec.lasttime = newrep.timestmp;
	if (edhdelp->logentct < NUMRCRDS) {
		edhdelp->errlist[edhdelp->logentct++] = nrec;
		return(0);
	}
	printf("%s: full error log on disk maj=%d min=%d\n",
		cmdname, major(newrep.diskdev),
		minor(newrep.diskdev));
	printf("\tattempting to combine records\n");
	rp = edhdelp->errlist;
	ep = rp + NUMRCRDS-1;
	i = NUMRCRDS;
	for ( ; rp < ep; rp++) {
		for (r2p = rp+1; r2p <= ep; r2p++) {
			if (rp->blkaddr == r2p->blkaddr) {
				if (r2p->firsttime < rp->firsttime)
					rp->firsttime = r2p->firsttime;
				if (r2p->lasttime > rp->lasttime)
					rp->lasttime = r2p->lasttime;
				rcnt = rp->reportct + r2p->reportct;
				if (rcnt > MAXRPTCNT) rcnt = MAXRPTCNT;
				rp->reportct = rcnt;
				i--;
				for (r3p = r2p; r3p < ep; r3p++)
					r3p[0] = r3p[1];
				*ep = zerorec;
				--r2p;
				--ep;
			}
		}
	}
	for (rp = edhdelp->errlist; rp <= ep; rp++) {
		if (rp->blkaddr == nrec.blkaddr) {
			if (nrec.firsttime < rp->firsttime)
				rp->firsttime = nrec.firsttime;
			if (nrec.lasttime > rp->lasttime)
				rp->lasttime = nrec.lasttime;
			if (rp->reportct < MAXRPTCNT) rp->reportct++;
			printf("recovered %d error record slots by combining entries\n",
				NUMRCRDS-i+1);
			edhdelp->logentct = i;
			return(0);
		}
	}
	if (i == NUMRCRDS) {
		printf("must throw away an entry to make room\n");
		r2p = edhdelp->errlist;
		i = r2p->reportct;
		for (rp = r2p+1; rp <= ep; rp++) {
			if (rp->reportct <= i && rp->firsttime < r2p->firsttime) {
				r2p = rp;
				i = rp->reportct;
			}
		}
		printf("%s: throwing away record for block %d on disk maj=%d min=%d\n",
			cmdname, r2p->blkaddr, major(newrep.diskdev),
			minor(newrep.diskdev));
		for ( ; r2p <= ep; r2p++) r2p[0] = r2p[1];
		*ep = zerorec;
	} else {
		 printf("recovered %d error record slots by combining entries\n",
		NUMRCRDS-i);
		edhdelp->logentct = i+1;
		ep++;
	}
	*ep = nrec;
	return(0);
}

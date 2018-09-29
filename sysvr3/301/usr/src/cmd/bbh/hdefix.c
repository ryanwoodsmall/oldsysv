/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bbh:hdefix.c	1.5"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/vtoc.h"
#include "sys/hdelog.h"
#include "sys/hdeioctl.h"
#include "sys/stat.h"
#include "sys/fs/s5macros.h"
#include "hdecmds.h"
#include "edio.h"
#include "sys/signal.h"
#include "sys/filsys.h"
#include <stdio.h>
#include <fcntl.h>
#include <utmp.h>
#include "sys/uadmin.h"

extern char *malloc(), *realloc();
char * cmdname;
dev_t devarg = -1;
struct eddata blist;
int blistcnt;
#define blistp ((daddr_t *) blist.badr)

struct eddata zerobuf;


main(argc, argv)
char *argv[];
{

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	if (argc < 1) cmdname = "hdefix";
	else cmdname = argv[0];
	if (argc < 2) {
		cmdusage();
		exit(INVEXIT);
	}
	if (argv[1][0] != '-') badusage();
	edinit();
	switch(argv[1][1]) {
		case 'a':
			doadd(argc-2, argv+2);
			break;
		case 'F':
			doforce(argc-2, argv+2);
			break;
		case 'p':
			dodmprnt(argc-2, argv+2);
			break;
		case 'r':
			dorestore(argc-2, argv+2);
			break;
		case 's':
			dosave(argc-2, argv+2);
			break;
		default:
			badusage();
	}
	exit(NORMEXIT);
}

cmdusage()
{
	fprintf(stderr,
		"usage:\t%s -a [[-D] maj min [{ -b blockno | -B cyl trk sec } ... ]]\n",
		cmdname);
	fprintf(stderr,
		"\t%s -F [[-D] maj min [{ -b blockno | -B cyl trk sec } ... ]]\n",
		cmdname);
	fprintf(stderr,
		"\t%s -p [[-D] maj min]\n",
		cmdname);
	fprintf(stderr,
		"\t%s -r [-D] maj min filename\n",
		cmdname);
	fprintf(stderr,
		"\t%s -s [-D] maj min filename\n",
		cmdname);
}

dodmprnt(ac, av)
char **av;
{
	register int i;

	if (ac > 0) {
		getdev(ac, av);
		prnt1dm(devarg);
		exit(NORMEXIT);
	}
	for (i = 0; i < edcnt; i++) {
		prnt1dm(edtable[i]);
	}
	exit(NORMEXIT);
}

prnt1dm(dev)
dev_t dev;
{
	register struct hddm *mp, *emp;
	int sz;

	if (edopen(dev)) {
		printf("%s: can't access disk maj=%d min=%d\n",
			cmdname, major(dev), minor(dev));
		return(-1);
	}
	if (edpdck()) {
		edclose();
		return(-1);
	}
	if (edgetdm()) {
		edclose();
		return(-1);
	}
	sz = edpdp->pdinfo.defectsz/sizeof(struct hddmap);
	mp = eddmp;
	emp = mp + sz;
	printf("\nBasic physical description of disk maj=%d min=%d:\n",
		major(dev), minor(dev));
	printf("sector size=%d, sectors per track=%d tracks per cylinder=%d\n",
		edpdp->pdinfo.bytes, edpdp->pdinfo.sectors, edpdp->pdinfo.tracks);
	printf("number of cylinders=%d, block number range: 0 thru %d\n",
		edpdp->pdinfo.cyls, edenddad-1);
	printf("defect map has %d slots\n", sz);
	printf("its active slots are:\nslot#   from blk#     to blk#\n");
	for (sz = 0; mp < emp; sz++, mp++) {
		if (mp->frmblk == -1) continue;
		printf("%5d %11d %11d\n", sz, mp->frmblk, mp->toblk);
	}
	printf("its surrogate region description is:\n");
	printf("\tstart blk#: %d\n", edpdp->pdinfo.relst);
	printf("\tsize (in blks): %d\n", edpdp->pdinfo.relsz);
	printf("\tnext blk#: %d\n", edpdp->pdinfo.relnext);
	printf("physical description is at blk# %d\n", edpdsno);
	printf("error log is at blk# %d\n", edpdp->pdinfo.errlogst);
	printf("logical start is at blk# %d\n", edpdp->pdinfo.logicalst);
	edclose();
	return(0);
}

char *blkbp;

dosave(ac, av)
char **av;
{
	register int sfdes;
	register int rval;
	register struct hddm *mp, *emp;
	int sz;

	rval = getdev(ac, av);
	ac -= rval;
	av += rval;
	if (ac < 1) badusage();
	if (edopen(devarg)) {
		printf("%s: can't access disk maj=%d min=%d\n",
			cmdname, major(devarg), minor(devarg));
		exit(ERREXIT);
	}
	if (edpdck()) {
		edclose();
		exit(ERREXIT);
	}
	if (edgetdm()) {
		edclose();
		exit(ERREXIT);
	}
	sz = edpdp->pdinfo.defectsz/sizeof(struct hddmap);
	mp = eddmp;
	emp = mp + sz;
	if ((sfdes = creat(av[0], 0644)) < 0) {
		fprintf(stderr, "unable to create save file: \"%s\"\n", av[0]);
		exit(ERREXIT);
	}
	if ((rval = write(sfdes, edpdsec.badr, edpdsec.csz)) != edpdsec.csz) {
		if (rval < 0)
			fprintf(stderr, "write of save file failed\n");
		else fprintf(stderr,
			"wrote %d bytes (instead of %d) in save file\n",
			rval, edpdsec.csz);
		edclose();
		close(sfdes);
		exit(ERREXIT);
	}
	if ((rval = write(sfdes, eddm.badr, eddm.csz)) != eddm.csz) {
		if (rval < 0)
			fprintf(stderr, "write of save file failed\n");
		else fprintf(stderr,
			"wrote %d bytes (instead of %d) in save file\n",
			rval, eddm.csz);
		fprintf(stderr, "WARNING: save file incomplete\n");
		edclose();
		close(sfdes);
		exit(ERREXIT);
	}
	if (!(blkbp = malloc(edsecsz))) {
		fprintf(stderr,"%s: malloc for block buffer failed\n",
			cmdname);
		fprintf(stderr, "WARNING: save file incomplete\n");
		exit(ERREXIT);
	}
	for (sz = 0; mp < emp; sz++, mp++) {
		if (mp->frmblk == -1) continue;
		if (mp->toblk == -1) {
			fprintf(stderr,
				"%s: WARNING: slot %d of defect map is bad\n",
				cmdname, sz);
			fprintf(stderr, "from block %d has no to block\n",
				mp->frmblk);
			continue;
		}
		if (edread(mp->toblk, 1, blkbp)) {
			fprintf(stderr,
				"%s: read of to block %d for defect map slot %d failed\n",
				cmdname, mp->toblk, sz);
			fprintf(stderr, "WARNING: save file incomplete\n");
			exit(ERREXIT);
		}
		if ((rval = write(sfdes, blkbp, edsecsz)) != edsecsz) {
			if (rval < 0)
				fprintf(stderr, "write of save file failed\n");
			else fprintf(stderr,
				"wrote %d bytes (instead of %d) in save file\n",
				rval, edsecsz);
			fprintf(stderr, "WARNING: save file incomplete\n");
			edclose();
			close(sfdes);
			exit(ERREXIT);
		}
	}
	fprintf(stderr, "save successful\n");
	edclose();
	close(sfdes);
	exit(NORMEXIT);
}

dorestore(ac, av)
char **av;
{
	register int sfdes;
	register int rval;
	register struct hddm *mp, *emp;
	int sz;
	int scnt;
	struct stat sbuf;

	rval = getdev(ac, av);
	ac -= rval;
	av += rval;
	if (ac < 1) badusage();
	chkstate();
	if (edopen(devarg)) {
		printf("%s: can't access disk maj=%d min=%d\n",
			cmdname, major(devarg), minor(devarg));
		exit(ERREXIT);
	}
	if ((sfdes = open(av[0], O_RDONLY)) < 0) {
		fprintf(stderr, "open of save file \"%s\" failed\n", av[0]);
		edclose();
		exit(ERREXIT);
	}
	if ((rval = read(sfdes, edpdsec.badr, edsecsz)) != edsecsz) {
		if (rval < 0)
			fprintf(stderr, "read of save file failed\n");
		else fprintf(stderr,
			"read %d bytes (instead of %d) in save file\n",
			rval, edsecsz);
		fprintf(stderr, "restore aborted\n");
		edclose();
		close(sfdes);
		exit(ERREXIT);
	}
	if (edpdck()) {
		edclose();
		exit(ERREXIT);
	}
	edenddad = edpdp->pdinfo.cyls * edpdp->pdinfo.tracks * edpdp->pdinfo.sectors;
	edallocdm();
	if ((rval = read(sfdes, eddm.badr, eddm.csz)) != eddm.csz) {
		if (rval < 0)
			fprintf(stderr, "read of save file failed\n");
		else fprintf(stderr,
			"read %d bytes (instead of %d) in save file\n",
			rval, eddm.csz);
		fprintf(stderr, "restore aborted\n");
		edclose();
		close(sfdes);
		exit(ERREXIT);
	}
	eddm.valid = 1;
	eddmap.valid = 1;
	sz = edpdp->pdinfo.defectsz/sizeof(struct hddmap);
	mp = eddmp;
	emp = mp + sz;
	if (!(blkbp = malloc(edsecsz))) {
		fprintf(stderr,"%s: malloc for block buffer failed\n",
			cmdname);
		fprintf(stderr, "restore aborted\n");
		exit(ERREXIT);
	}
	for (scnt = 0, sz = 0; mp < emp; sz++, mp++) {
		if (mp->frmblk == -1) continue;
		if (mp->toblk == -1) {
			continue;
		}
		scnt++;
	}
	rval = edsecsz + eddm.csz + scnt*edsecsz;
	if (fstat(sfdes, &sbuf) < 0) {
		fprintf(stderr, "fstat of save file \"%s\" failed\n", av[0]);
		edclose();
		close(sfdes);
		exit(ERREXIT);
	}
	if (sbuf.st_size != rval) {
		if (sbuf.st_size < rval)
			fprintf(stderr, "save file too small (%d < %d)\n",
				sbuf.st_size, rval);
		else fprintf(stderr, "save file too large (%d > %d)\n",
				sbuf.st_size, rval);
		exit(ERREXIT);
	}
	if (edputpd()) {
		fprintf(stderr, "%s: write of pd sector failed\n", cmdname);
		exit(ERREXIT);
	}
	if (edputdm()) {
		fprintf(stderr, "%s: write of defect map failed\n", cmdname);
		fprintf(stderr, "disk maj=%d, min=%d in inconsistent state\n",
			major(devarg), minor(devarg));
		fprintf(stderr, "restore aborted\n");
		exit(ERREXIT);
	}
	mp = eddmp;
	for (sz = 0; mp < emp; sz++, mp++) {
		if (mp->frmblk == -1) continue;
		if (mp->toblk == -1) {
			fprintf(stderr,
				"%s: WARNING: slot %d of defect map is bad\n",
				cmdname, sz);
			fprintf(stderr, "from block %d has no to block\n",
				mp->frmblk);
			continue;
		}
		if ((rval = read(sfdes, blkbp, edsecsz)) != edsecsz) {
			if (rval < 0)
				fprintf(stderr, "read of save file failed\n");
			else fprintf(stderr,
				"read %d bytes (instead of %d) in save file\n",
				rval, edsecsz);
			edclose();
			fprintf(stderr, "disk maj=%d, min=%d in inconsistent state\n",
				major(devarg), minor(devarg));
			fprintf(stderr, "restore aborted\n");
			close(sfdes);
			exit(ERREXIT);
		}
		if (edwrite(mp->toblk, 1, blkbp)) {
			fprintf(stderr,
				"%s: write of to block %d for defect map slot %d failed\n",
				cmdname, mp->toblk, sz);
			fprintf(stderr, "disk maj=%d, min=%d in inconsistent state\n",
				major(devarg), minor(devarg));
			fprintf(stderr, "restore aborted\n");
			exit(ERREXIT);
		}
	}
	fprintf(stderr, "restore successful\n");
	edclose();
	close(sfdes);
	exit(NORMEXIT);
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

doforce(ac, av)
char **av;
{
	register int rval;
	register int sz;
	register struct hddm *mp, *m2p, *emp;

	chkstate();
	rval = getdev(ac, av);
	ac -= rval;
	av += rval;
	if (edopen(devarg)) {
		exit(ERREXIT);
	}
	if (edpdck()) {
		exit(ERREXIT);
	}
	if (edgetdm()) {
		exit(ERREXIT);
	}
	sz = edpdp->pdinfo.defectsz/sizeof(struct hddmap);
	mp = eddmp;
	emp = mp + sz;
	for ( ; mp < emp; mp ++)
		if (mp->frmblk == -1) break;
	emp = --mp;
	if (mp < eddmp) {
		fprintf(stderr, "there are no defects to remove\n");
		exit(ERREXIT);
	}
	if (ac < 1) {
		printf("%d\n", mp->frmblk);
		emp->frmblk = -1;
		emp->toblk = -1;
		if (edputdm()) {
			exit(ERREXIT);
		}
		edclose();
		exit(NORMEXIT);
	}
	getablist(ac, av);
	for (sz = 0; sz < blistcnt; sz++) {
		for(mp = eddmp; mp <= emp; mp++) {
			if (blistp[sz] == mp->frmblk) {
				mp->frmblk = -1;
				mp->toblk = -1;
				goto gotit;
			}
		}
		fprintf(stderr, "%s: block# %d not a from block in defect map\n",
			cmdname, blistp[sz]);
		exit(ERREXIT);
gotit:		;
	}
	for (mp = m2p = eddmp; mp <= emp; mp++) {
		if (mp->frmblk == -1) continue;
		if (mp != m2p) *m2p = *mp;
		m2p++;
	}
	while (m2p <= emp) {
		m2p->frmblk = -1;
		m2p->toblk = -1;
		m2p++;
	}
	if (edputdm()) {
		exit(ERREXIT);
	}
	edclose();
	printf("changed defect table\n");
	exit(NORMEXIT);
}

chkstate()
{
	register struct utmp *up;
	int c;
	extern struct utmp *getutid();
	struct utmp runlvl;

	runlvl.ut_type = RUN_LVL;
	if ((up = getutid(&runlvl)) == NULL) {
		fprintf(stderr, "%s: can't determine run state\n", cmdname);
		exit(ERREXIT);
	}
	c = up->ut_exit.e_termination;
	if (c != 'S' && c != 's') {
		fprintf(stderr, "%s: not in single user state\n", cmdname);
		exit(INVEXIT);
	}
}

getablist(ac, av)
char **av;
{
	register int sz;
	register int rval;

	sz = (ac/2) * sizeof(daddr_t);	/* possible over estimate, so what */
	if (!(blist.badr = malloc(sz))) {
		fprintf(stderr, "%s: malloc of blist failed\n", cmdname);
		exit(ERREXIT);
	}
	blist.bsz = blist.csz = sz;
	blistcnt = 0;
	sz = ac/2;
	while(ac > 0) {
		rval = getbno(ac, av);
		ac -= rval;
		av += rval;
	}
}

getbno(ac, av)
char **av;
{
	daddr_t bno;
	int cyl, trk, sec;

	if (ac < 2) badusage();
	if (av[0][0] != '-')
		badusage();
	if (av[0][1] == 'b') {
		if (sscanf(av[1], "%d", &bno) != 1)
			badusage();
		if (dadcheck(bno)) badusage();
		blistp[blistcnt++] = bno;
		return(2);
	}
	if (av[0][1] == 'B') {
		if (ac < 4) badusage();
		if (sscanf(av[1], "%d", &cyl) != 1) badusage();
		if (sscanf(av[2], "%d", &trk) != 1) badusage();
		if (sscanf(av[3], "%d", &sec) != 1) badusage();
		if ((bno = ctstodad(cyl, trk, sec)) == -1) badusage();
		blistp[blistcnt++] = bno;
		return(4);
	}
	badusage();
}

int rebootflg;
struct hdedata hdequeue[HDEQSIZE];
int hdeqcnt;

gethdeq()
{

	if (hdeqcnt = edgetrct()) {
		if (hdeqcnt < 0) {
			fprintf(stderr,
				"%s: attempt to get error report queue cnt failed\n",
				cmdname);
			hdeqcnt = 0;
		} else {
			if (hdeqcnt > HDEQSIZE) {
			  /* just in case somebody increases HDEQSIZE
			   * and fails to recompile/install hdefix
			  */
				hdeqcnt = HDEQSIZE;
			}
			if ((hdeqcnt = edgetrpts(hdequeue, hdeqcnt)) < 0) {
				fprintf(stderr,
					"%s: attempt to get queued error reports failed\n",
					cmdname);
				hdeqcnt = 0;
			}
			printf("there are %d unlogged error reports\n",
				hdeqcnt);
			edclrrpts(hdequeue, hdeqcnt);
			doclear();
		}
	}
}

doclear()
{
	struct hdedata junkit;

	while (edgetrct() > 0) {
		if (edgetrpts(&junkit, 1) != 1) break;
		if (edclrrpts(&junkit, 1) != 1) break;
	}
}
doadd(ac, av)
char **av;
{
	register int i;

	chkstate();
	sync();
	printf("%s: once fixing bad blocks is started, signals are ignored\n", cmdname);
	sleep(10);
	printf("starting\n");
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	fixsetup();
	gethdeq();
	if (ac < 1) {
		for (i = 0; i < edcnt; i++) {
			devarg = edtable[i];
			do1add(ac, av);
		}
	} else {
		i = getdev(ac, av);
		ac -= i;
		av += i;
		do1add(ac, av);
	}
	doclear();
	if (rebootflg) {
		printf("%s: causing system reboot\n", cmdname);
		doreboot();
	}
	printf("%s: you may now resume normal operation\n", cmdname);
	exit(NORMEXIT);
}

fixsetup()
{

	edfixlk();
}

daddr_t curbno, effbno;
long lgclsz;

do1add(ac, av)
char **av;
{
	register int i, sz;
	daddr_t bigsur;
	int clrlogflg;
	int zaplogflg;
	register struct hddm *mp, *m2p, *emp, newmp;

	if (edopen(devarg)) {
		fprintf(stderr, "%s: unable to access disk maj=%d min=%d\n",
			cmdname, major(devarg), minor(devarg));
		return(-1);
	}
	setzbuf();
	if (edgetdm()) {
		fprintf(stderr,
			"%s: unable to access defect map on disk maj=%d min=%d\n",
			cmdname, major(devarg), minor(devarg));
		edclose();
		return(-1);
	}
	zaplogflg = 0;
	if (ac > 0) {
		if (hdeqcnt > 0) {
			printf("unlogged error reports are being ignored\n");
			hdeqcnt = 0;
		}
		getablist(ac, av);
		clrlogflg = 0;
	} else {
		clrlogflg = 1;
		getloglist();
	}
	if (blistcnt == 0) {
		edclose();
		printf("%s: no bad blocks on disk maj=%d min=%d\n",
			cmdname, major(devarg), minor(devarg));
		return(0);
	}
	printf("%s: processing %d bad blocks on disk maj=%d min=%d\n",
		cmdname, blistcnt, major(devarg), minor(devarg));
	sz = edpdp->pdinfo.defectsz/sizeof(struct hddmap);
	m2p = eddmp;
	emp = m2p + sz;
	for ( ; m2p < emp; m2p++)
		if (m2p->frmblk == -1) break;
	for (bigsur = -1, mp = eddmp; mp < m2p; mp++)
		if (mp->toblk > bigsur) bigsur = mp->toblk;
	if (bigsur >= ((daddr_t)edpdp->pdinfo.relst+edpdp->pdinfo.relsz))
		printf("defect map corrupted, it has a to-block out of surrogate region\n");
	else if (bigsur > edpdp->pdinfo.relnext) {
		edpdp->pdinfo.relnext = bigsur;
		printf("correcting next block value of surrogate region accounting\n");
		edputpd();
	}
	for (i = 0; i < blistcnt; i++) {
		effbno = curbno = blistp[i];
		printf("processing block %d:\n", curbno);
		for (mp = eddmp; mp < m2p; mp++) {
			if (mp->frmblk == curbno) {
				printf("    block already mapped\n");
				if (clrlogflg) {
					printf("    clearing block from log\n");
					clrlog(curbno);
				}
				goto nextone;
			}
		}
		printf("    new bad block (e.g., not already mapped)\n");
		if (edpdp->pdinfo.relnext >= edpdp->pdinfo.relst + edpdp->pdinfo.relsz) {
runout:
			printf("CRITICAL PROBLEM: you have run out of surrogate image space on this disk\n");
abortit:
			printf("unable to map bad block\n");
			printf("processing being aborted for disk maj=%d min=%d\n",
				major(devarg), minor(devarg));
			edclose();
			return(-1);
		}
		if (curbno == edpdsno) {
			printf("CRITICAL PROBLEM: physical description sector is bad\n");
			goto abortit;
		}
		if (curbno >= edpdp->pdinfo.defectst &&
		    curbno < edpdp->pdinfo.defectst + (edpdp->pdinfo.defectsz+edsecsz-1)/edsecsz) {
			printf("CRITICAL PROBLEM: a defect map block is bad\n");
			goto abortit;
		}
		for (mp = eddmp; mp < m2p; mp++) {
			if (mp->toblk == curbno) {
				printf("    block is surrogate image of block %d\n",
					mp->frmblk);
				effbno = mp->frmblk;
				goto classify;
			}
		}
		if (curbno >= edpdp->pdinfo.relst && curbno < edpdp->pdinfo.relst + edpdp->pdinfo.relsz) {
			printf("    block is unused surrogate image block\n");
			printf("    doing nothing about it now\n");
			goto nextone;
		}
classify:
		if (effbno >=  edpdp->pdinfo.errlogst &&
		    effbno < edpdp->pdinfo.errlogst + edpdp->pdinfo.errlogsz) {
			printf("    block in disk error log\n");
			clrlogflg = 0;	/* no touchie, its bad */
			zaplogflg = 1;
			goto fixit;
		}
		if (edpdp->pdinfo.logicalst < edpdsno) {
			lgclsz = edpdsno - edpdp->pdinfo.logicalst;
			if (effbno >= edpdsno) {
				printf("    non-critical block in device specific data\n");
				goto fixit;
			}
		} else {
			lgclsz = edenddad - edpdp->pdinfo.logicalst;
			if (effbno < edpdp->pdinfo.logicalst) {
				printf("    non-critical block in device specific data\n");
				goto fixit;
			}
		}
		rebootflg = 1;		/* the conservative strategy */
		printf("    block in partitioned portion of disk\n");
		partit();
fixit:
		printf("    assigning new surrogate image block for it\n");
		newmp.toblk = edpdp->pdinfo.relnext++;
		newmp.frmblk = effbno;
		if (effbno != curbno) {	/* use same slot */
			for (mp = eddmp; mp < m2p; mp++) {
				if (mp->frmblk == effbno)
					goto gotit;
			}
			/* not possible */
			printf("this program is sick! sick! sick!\n");
		}
		for (mp = m2p-1; mp >= eddmp; --mp) {
			if (mp->frmblk < effbno) break;
			mp[1] = mp[0];
		}
		++mp;
gotit:
		*mp = newmp;
		m2p++;
		edputpd();
		edputdm();
		doclear();
		if (zaplogflg) {
			zaplogflg = 0;
			edvalel(1);
		} else edwrite (effbno, 1, zerobuf.badr);
		if (edgetrct() > 0) {
			doclear();
			printf("got an error while zeroing surrogate\n");
			if (edpdp->pdinfo.relnext >=
			    edpdp->pdinfo.relst + edpdp->pdinfo.relsz) {
				goto runout;
			}
			goto fixit;
		}
nextone:
		if (clrlogflg) clrlog(curbno);
	}
	edclose();
	printf("finished with this disk\n");
	return(0);
}

setzbuf()
{
	if (zerobuf.csz >= edsecsz) {
		zerobuf.csz = edsecsz;
		return(0);
	}
	if (zerobuf.bsz >= edsecsz) {
		memset(zerobuf.badr, 0, edsecsz);
		zerobuf.csz = edsecsz;
		return(0);
	}
	doalloc(&zerobuf, edsecsz, "zerobuf");
	memset(zerobuf.badr, 0, edsecsz);
	return(0);
}

struct eddata sblkbuf;
#define sbp	((struct filsys *) sblkbuf.badr)
struct eddata inobuf;
#define ip	((struct dinode *) inobuf.badr)

partit()
{
	daddr_t lbno, vtocst, vtocend, maxp, pend, pbno, sbbno, inobno;
	register int sz, i, inpart, fspart, j;
	char *cp1, *cp2;
	register struct partition *pp;
	long fssize, rootino;

	lbno = effbno - edpdp->pdinfo.logicalst;
	sz = edsecsz;
	if (sz < 512) sz = ((512 + edsecsz - 1)/edsecsz) * edsecsz;
	vtocst = sz/edsecsz;
	vtocend = 2 * vtocst;
	if (lbno < vtocst) {
		printf("CRITICAL PROBLEM: mboot sector is bad\n");
		return(0);
	}
	if (lbno >= vtocst && lbno < vtocend) {
		printf("CRITICAL PROBLEM: vtoc sector is bad\n");
		return(0);
	}
	if (edgetvt()) {
		fprintf(stderr, "%s: no vtoc, cannot classify bad block\n",
			cmdname);
		return(-1);
	}
	if (edvtocp->v_sanity != VTOC_SANE) {
		fprintf (stderr,
			"%s: invalid vtoc, cannot classify bad block\n",
			cmdname);
		return(-1);
	}
	pp = edvtocp->v_part;
	maxp = 0;
	for (inpart = fspart = i = 0; i < V_NUMPAR; i++, pp++) {
		if (pp->p_size <= 0) continue;
		pend = pp->p_start + pp->p_size;
		if (pend > maxp) maxp = pend;
		if (pp->p_start > lbno || pend <= lbno)
			continue;
		inpart |= 1 << i;
		printf("    block in partition %d\n", i);
		pbno = lbno - pp->p_start;
		printf("\tit is block %d in the partition\n", pbno);
		if (pp->p_tag == V_SWAP) {
			printf("\tblock is in a swap partition\n");
		}
		if (pp->p_flag & V_UNMNT) continue;
		if ( pbno == sz/edsecsz) {
			printf("\tif partition %d contained a file system,\n");
			printf("\tthat file system's superblock was lost\n");
			continue;
		}
		if (pbno == 0) continue;
		doalloc(&sblkbuf, sz, "sblkbuf");
		sbbno = edpdp->pdinfo.logicalst + pp->p_start + sz/edsecsz;
		if (edread(sbbno, sz/edsecsz, sblkbuf.badr)) {
			printf("\tcannot read potential superblock sector of partition\n");
			continue;
		}
		if (sbp->s_magic != FsMAGIC) continue;
		fssize = sbp->s_fsize *(sz/edsecsz);
		rootino = 2*(sz/edsecsz);
		if (sbp->s_type == Fs1b || sbp->s_type == Fs2b) {
			if (sbp->s_type == Fs2b) {
				fssize *= FsBSIZE(Fs2BLK)/512;
				rootino *= FsBSIZE(Fs2BLK)/512;
			}
			if (pbno >= fssize) {
				printf("\tblock was past end of file system\n");
				continue;
			}
			printf ("\tpartition has a file system containing the bad block\n");
		} else printf("\tcan't calculate end of this type of file system\n");
		sbp->s_state = FsBADBLK;
		fspart++;
		printf("\tmarking file system dirtied by bad block handling\n");
		if (edwrite(sbbno, sz/edsecsz, sblkbuf.badr)) {
			printf("WARNING: write of superblock failed\n");
			continue;
		}
		if (sbp->s_type != Fs1b && sbp->s_type != Fs2b) {
			printf("\tfile system is type %d and I ",
				sbp->s_type);
			printf("don't know how to do any more\n");
			continue;
		}
		if (sz != 512) {
			printf("\tstrange block size of %d and I ",
				edsecsz);
			printf("don't know how to do any more\n");
			continue;
		}
		fssize = (2 + sbp->s_isize)*(sz/edsecsz);
		if (sbp->s_type == Fs2b)
			fssize *= FsBSIZE(Fs2BLK)/512;
		if (pbno < fssize) {
			printf("\tthe bad block was an inode block\n");
			if (pbno == rootino) {
				printf("\tthe root inode of the file system was lost\n");
			}
		} else printf("\tthe bad block was a file block\n");
	}
	if (fspart > 1) {
		printf("WARNING: %d file systems claimed to contain block\n");
		printf("all but one claim should be false, but I marked all\n");
	}
	return(0);
}

doreboot()
{
	printf("system being rebooted\n");
	sleep (10);
	uadmin(A_REBOOT, AD_BOOT, 0);
	printf("REBOOT FAILED\n");
	exit(BADBEXIT);
}

getloglist()
{
	register int i, j;
	register struct hderecord *rp;

	if (blist.bsz == 0) {
		i = blist.csz = blist.bsz = (NUMRCRDS+HDEQSIZE) * sizeof(daddr_t);
		if (!(blist.badr = malloc(i))) {
			fprintf(stderr, "%s: malloc of blist failed\n", cmdname);
			exit(ERREXIT);
		}
	}
	blistcnt = 0;
	if (edgetel()) return(-1);
	for (i = 0, rp = edhdelp->errlist; i < edhdelp->logentct; i++, rp++) {
		for (j = 0; j < blistcnt; j++) {
			if (blistp[j] == rp->blkaddr) goto contin;
		}
		blistp[blistcnt++] = rp->blkaddr;
contin:		;
	}
	for (i = 0; i < hdeqcnt; i++) {
		if (hdequeue[i].diskdev != devarg) continue;
		for (j = 0; j < blistcnt; j++) {
			if (blistp[j] == hdequeue[i].blkaddr) goto contin2;
		}
		blistp[blistcnt++] = hdequeue[i].blkaddr;
contin2:	;
	}
	return(0);
}

struct hderecord zder;

clrlog(bno)
daddr_t bno;
{
	register struct hderecord *rp, *r2p, *erp;
	if (edgetel()) return(-1);
	for (rp = r2p = edhdelp->errlist, erp = rp + edhdelp->logentct;
	     rp < erp; rp++) {
		if (rp->blkaddr == bno) continue;
		if (rp != r2p) *r2p = *rp;
		r2p++;
	}
	edhdelp->logentct = r2p - edhdelp->errlist;
	while (r2p < erp) *r2p++ = zder;
	if (edputel()) return(-1);
	return(0);
}

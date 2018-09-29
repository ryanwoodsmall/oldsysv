/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bbh:hdeadd.c	1.3"

#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/vtoc.h"
#include "sys/hdelog.h"
#include "sys/hdeioctl.h"
#include "sys/stat.h"
#include "hdecmds.h"
#include "edio.h"
#include <stdio.h>
#include <fcntl.h>
#include "dconv.h"

char * cmdname;
dev_t devarg = -1;

main(argc, argv)
char *argv[];
{

	if (argc < 1) cmdname = "hdeadd";
	else cmdname = argv[0];
	if (argc < 2) {
		cmdusage();
		exit(INVEXIT);
	}
	if (argv[1][0] != '-') badusage();
	switch(argv[1][1]) {
		case 'a':
			doadd(argc-2, argv+2);
			break;
		case 'd':
			dodelete(argc-2, argv+2);
			break;
		case 'e':
			doeqpdsk(argc-2, argv+2);
			break;
		case 'f':
			dofilead(argc-2, argv+2);
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
		"usage:\t%s -a -D maj min { -b blockno | -B cyl trk sec } [-t mmddhhmm[yy]]\n",
		cmdname);
	fprintf(stderr,
		"\t%s -d -D maj min { -b blockno | -B cyl trk sec } [-t mmddhhmm[yy]]\n",
		cmdname);
	fprintf(stderr,
		"\t%s -d [-D] maj min [-F mmddhhmm[yy]] [-T mmddhhmm[yy]]\n",
		cmdname);
	fprintf(stderr,
		"\t%s -e [[-D] maj min]\n",
		cmdname);
	fprintf(stderr,
		"\t%s -f filename\n",
		cmdname);
	fprintf(stderr,
		"\t%s -r [[-D] maj min] filename\n",
		cmdname);
	fprintf(stderr,
		"\t%s -s [[-D] maj min] filename\n",
		cmdname);
}

doeqpdsk(ac, av)
char **av;
{
	int i;

	edinit();
	if (ac > 0) {
		getdev(ac, av);
		printf("maj=%d min=%d is%s an equipped disk\n",
			major(devarg), minor(devarg),
			(i = eddevck(devarg)) ? " not" : "");
		exit(i ? BADBEXIT : NORMEXIT);
	}
	printf("The %d equipped disks for bad block handling are:\n",
		edcnt);
	printf(" MAJOR MINOR\n");
	for (i = 0; i < edcnt; i++)
		printf("%5d %5d\n", major(edtable[i]), minor(edtable[i]));
	exit(NORMEXIT);
}

dosave(ac, av)
char **av;
{
	register int sfdes;
	register int rval;

	rval = getdev(ac, av);
	ac -= rval;
	av += rval;
	if (ac < 1) badusage();
	if (edopen(devarg)) exit(ERREXIT);
	if (edgetel()) {
		edclose();
		exit(ERREXIT);
	}
	if (edhdelp->l_valid != HDEDLVAL) {
		fprintf(stderr,
			"%s: WARNING: disk log does not contain a valid log\n",
			cmdname);
		fprintf(stderr, "\tdisk maj=%d min=%d\n",
			major(devarg), minor(devarg));
	}
	if ((sfdes = creat(av[0], 0644)) < 0) {
		fprintf(stderr, "unable to create save file: \"%s\"\n", av[0]);
		exit(ERREXIT);
	}
	if ((rval = write(sfdes, edhdel.badr, edhdel.csz)) != edhdel.csz) {
		if (rval < 0)
			fprintf(stderr, "write of save file failed\n");
		else fprintf(stderr,
			"wrote %d bytes (instead of %d) in save file\n",
			rval, edhdel.csz);
		edclose();
		close(sfdes);
		exit(ERREXIT);
	}
	fprintf(stderr, "save successful\n");
	edclose();
	close(sfdes);
	exit(NORMEXIT);
}

dorestore(ac, av)
char **av;
{
	register int rval;
	register int sfdes;
	register int sz;
	struct stat sbuf;

	rval = getdev(ac, av);
	ac -= rval;
	av += rval;
	if (ac < 1) badusage();
	if (edopen(devarg)) exit(ERREXIT);
	if (edpdck()) {
		edclose();
		exit(ERREXIT);
	}
	sz = edallocel();
	if ((sfdes = open(av[0], O_RDONLY)) < 0) {
		fprintf(stderr, "open of save file \"%s\" failed\n", av[0]);
		edclose();
		exit(ERREXIT);
	}
	if (fstat(sfdes, &sbuf) < 0) {
		fprintf(stderr, "fstat of save file \"%s\" failed\n", av[0]);
		edclose();
		close(sfdes);
		exit(ERREXIT);
	}
	if (sbuf.st_size != edhdel.csz) {
		if (sbuf.st_size < edhdel.csz)
			fprintf(stderr, "save file too small (%d < %d)\n",
				sbuf.st_size, edhdel.csz);
		else fprintf(stderr, "save file too large (%d > %d)\n",
				sbuf.st_size, edhdel.csz);
		exit(ERREXIT);
	}
	if ((rval = read(sfdes, edhdel.badr, edhdel.csz)) != edhdel.csz) {
		fprintf(stderr, "bad read of save file\"%s\"\n", av[0]);
		edclose();
		close(sfdes);
		exit(ERREXIT);
	}
	close(sfdes);
	edhdel.valid = 1;
	if (edhdelp->l_valid != HDEDLVAL) {
		fprintf(stderr,
			"WARNING: save file does not contain a valid log\n");
		sleep(10);
	}
	if(edputel()) {
		edclose();
		exit(ERREXIT);
	}
	edclose();
	fprintf(stderr, "disk error log restored from file \"%s\"\n",
		av[0]);
	exit(NORMEXIT);
}

#define LBSIZE 512
#define MAXLAC 32
char linebuf[LBSIZE];
int linecnt;
char *lav[MAXLAC];
int lac;

dofilead(ac, av)
char **av;
{
	register FILE *infile;
	register char *cp;
	register int c;

	if (ac < 1) badusage();
	if ((infile = fopen(av[0], "r")) == NULL) {
		fprintf(stderr, "open of input file \"%s\" failed\n", av[0]);
		exit(ERREXIT);
	}
	for(;;) {
		linecnt++;
		lac = 0;
		cp = linebuf;
		c = fgetc(infile);
white:
		while(c == ' ' || c == '\t') c = fgetc(infile);
		if (c == EOF) exit(NORMEXIT);
		if (c == '\n') {
			do1a_or_d(lac, lav);
			continue;
		}
		if (lac >= MAXLAC) {
			fprintf(stderr, "too many arguments in line %d\n",
				linecnt);
			exit (INVEXIT);
		}
		lav[lac++] = cp;
		while(c != EOF && c != ' ' && c != '\t' && c != '\n') {
			if (cp >= &linebuf[LBSIZE-2]) {
				fprintf(stderr, "line %d is too long\n",
					linecnt);
				exit(INVEXIT);
			}
			*cp++ = c;
			c = fgetc(infile);
		}
		*cp++ = 0;
		goto white;
	}
}

badusage()
{
	if (linecnt) badline();
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

badline()
{

	if(!linecnt) badusage();
	fprintf(stderr, "bad usage on line %d\n", linecnt);
	exit(INVEXIT);
}

time_t fromtm;
time_t totm;
time_t reptm;

daddr_t bnoarg;
int cylarg, trkarg, secarg;

int didflgs;
#define DIDB	1
#define DIDb	2
#define DIDD	4
#define DIDF	8
#define DIDT	16
#define DIDt	32

do1a_or_d(ac, av)
char **av;
{

	if (av[0][0] != '-') badline();
	if (av[0][1] == 'a') doadd(ac-1, av+1);
	else if (av[0][1] == 'd') dodelete(ac-1, av+1);
	else badline();
}

doadd(ac, av)
char **av;
{
	struct hdedata logrep;
	register int i;

	reptm = time(0);
	didflgs = 0;
	while (ac > 0) {
		if (av[0][0] != '-') badline();
		switch(av[0][1]) {
			case 'D':
				if (DIDD & didflgs) badline();
				didflgs |= DIDD;
				i = getdev(ac, av);
				ac -= i;
				av += i;
				break;

			case 'b':
				if ((DIDb|DIDB) & didflgs) badline();
				didflgs |= DIDb;
				if (ac < 2) badline();
				if (sscanf(av[1], "%d", &bnoarg) != 1) badline();
				/* check it later when equipped disk is open */
				ac -= 2;
				av += 2;
				break;

			case 'B':
				if ((DIDb|DIDB) & didflgs) badline();
				didflgs |= DIDB;
				if (ac < 4) badline();
				if (sscanf(av[1], "%d", &cylarg) != 1) badline();
				if (sscanf(av[2], "%d", &trkarg) != 1) badline();
				if (sscanf(av[3], "%d", &secarg) != 1) badline();
				/* check it later when equipped disk is open */
				ac -= 4;
				av += 4;
				break;

			case 't':
				if (DIDt & didflgs) badline();
				didflgs |= DIDt;
				if (ac < 2) badline();
				reptm = tconvs(av[1]);
				if (reptm == -1) badline();
				ac -= 2;
				av += 2;
				break;

			default:
				badline();
		}
	}
	if (!(didflgs & DIDD) || !(didflgs & (DIDb|DIDB))) badline();
	if (edopen(devarg)) exit(ERREXIT);
	if (didflgs & DIDb) {
		if (dadcheck(bnoarg)) exit(ERREXIT);
	} else if ((bnoarg = ctstodad(cylarg, trkarg, secarg)) == -1)
		exit(ERREXIT);
	logrep.diskdev = devarg;
	for (i = 0; i < 12; i++) logrep.dskserno[i] = edpdp->pdinfo.serial[i];
	logrep.blkaddr = bnoarg;
	logrep.readtype = HDECRC;
	logrep.severity = HDEUNRD;
	logrep.badrtcnt = 0;
	logrep.bitwidth = 0;
	logrep.timestmp = reptm;
	printf("%s: logging following error report:\n", cmdname);
	printf("\tdisk maj=%d min=%d\n",
		major(logrep.diskdev),
		minor(logrep.diskdev));
	printf("\tblkaddr=%d, timestamp=%s",
		logrep.blkaddr,
		ctime(&logrep.timestmp));
	printf("\treadtype=%d, severity=%d, badrtcnt=%d, bitwidth=%d\n",
		logrep.readtype,
		logrep.severity,
		logrep.badrtcnt,
		logrep.bitwidth);
	sleep(3);
	edmlogr(&logrep, 1);
	edclose();
}

struct hderecord zerorec;

dodelete(ac, av)
char **av;
{
	register int i;
	register struct hderecord *r1p, *r2p, *ep;
	daddr_t minblk, maxblk;

	fromtm = 0;
	totm = 0x7FFFFFFF;
	didflgs = 0;
	didflgs = 0;
	while (ac > 0) {
		if (av[0][0] != '-') badline();
		switch(av[0][1]) {
			case 'D':
				if (DIDD & didflgs) badline();
				didflgs |= DIDD;
				i = getdev(ac, av);
				ac -= i;
				av += i;
				break;

			case 'b':
				if ((DIDb|DIDB) & didflgs) badline();
				didflgs |= DIDb;
				if (ac < 2) badline();
				if (sscanf(av[1], "%d", &bnoarg) != 1) badline();
				/* check it later when equipped disk is open */
				ac -= 2;
				av += 2;
				break;

			case 'B':
				if ((DIDb|DIDB) & didflgs) badline();
				didflgs |= DIDB;
				if (ac < 4) badline();
				if (sscanf(av[1], "%d", &cylarg) != 1) badline();
				if (sscanf(av[2], "%d", &trkarg) != 1) badline();
				if (sscanf(av[3], "%d", &secarg) != 1) badline();
				/* check it later when equipped disk is open */
				ac -= 4;
				av += 4;
				break;

			case 't':
				if (DIDt & didflgs) badline();
				didflgs |= DIDt;
				if (ac < 2) badline();
				reptm = tconvs(av[1]);
				if (reptm == -1) badline();
				ac -= 2;
				av += 2;
				break;

			case 'F':
				if (DIDF & didflgs) badline();
				didflgs |= DIDF;
				if (ac < 2) badline();
				fromtm = tconvs(av[1]);
				if (fromtm == -1) badline();
				ac -= 2;
				av += 2;
				break;

			case 'T':
				if (DIDT & didflgs) badline();
				didflgs |= DIDT;
				if (ac < 2) badline();
				totm = tconvs(av[1]);
				if (totm == -1) badline();
				ac -= 2;
				av += 2;
				break;

			default:
				badline();
		}
	}
	if (!(didflgs & DIDD)) badline();
	if ((didflgs & (DIDb|DIDB)) && (didflgs & (DIDF|DIDT)))
		badline();
	if (edopen(devarg)) exit(ERREXIT);
	if (edgetel()) exit(ERREXIT);
	minblk = 0;
	maxblk = edenddad;
	if (didflgs & (DIDb|DIDB)) {
		if (didflgs & DIDb) {
			if (dadcheck(bnoarg)) exit(ERREXIT);
		} else if ((bnoarg = ctstodad(cylarg, trkarg, secarg)) == -1)
			exit(ERREXIT);
		minblk = maxblk = bnoarg;
		if (didflgs & DIDt) {
			fromtm = totm = reptm;
			fromtm--;
			totm += 60;
		}
	}
	r1p = r2p = edhdelp->errlist;
	ep = r1p + edhdelp->logentct;
	printf("attempting to delete error reports for disk: maj=%d min=%d\n",
		major(devarg), minor(devarg));
/*DB*/printf("minblk=%d, maxblk=%d, fromtm=%d, totm=%d\n", minblk, maxblk, fromtm, totm);
	for (i = 0; r1p < ep; r1p++) {
		if (!(r1p->blkaddr < minblk || r1p->blkaddr > maxblk ||
		      r1p->lasttime < fromtm || r1p->firsttime > totm)) {
			i++;
			printf("removing report for block %d\n",
				r1p->blkaddr);
		} else {
			if (r1p != r2p) *r2p = *r1p;
			r2p++;
		}
	}
	while (r2p < ep) *r2p++ = zerorec;
	printf("%d reports remain in the log\n",
		edhdelp->logentct -= i);
	if (i) {
		edputel();
	} else printf("no reports met the criteria\n");
	edclose();
}

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bbh:edio.h	1.3"

struct eddata {
	char	*badr;
	int	bsz;
	int	csz;
	int	valid;
};

extern int edcnt;
extern dev_t *edtable;
extern struct hdeedd *ededdp;

extern struct eddata edpdsec;
extern struct eddata eddmap;		/* machine form of defect map */
extern struct eddata eddm;		/* standard form of defect map */
extern struct eddata edhdel;
extern struct eddata edvtoc;
extern long edsecsz;
extern daddr_t edenddad;
extern daddr_t edpdsno;

struct dskaddr	{
	short	cylno;
	char	trkno;
	char	secno;
};

union dskaval {
	struct	dskaddr cts;
	long	lval;
};

struct hddmap {
	union dskaval frmadr;
	union dskaval toadr;
};

struct hddm {
	daddr_t	frmblk;
	daddr_t	toblk;
};
#define edpdp	((struct pdsector *)edpdsec.badr)
#define eddmapp	((struct hddmap *)eddmap.badr)
#define eddmp	((struct hddm *)eddm.badr)
#define edhdelp	((struct hdedl *)edhdel.badr)
#define edvtocp	((struct vtoc *)edvtoc.badr)

#define eddmcnt	(edpdp->pdinfo.defectsz/sizeof(struct hddmap))

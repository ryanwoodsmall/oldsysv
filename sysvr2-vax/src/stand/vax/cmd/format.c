/*	@(#)format.c	1.6	*/
	/* RP06 or RM05 or RM03 formatter */
	/*
	 *	REVISIONS:
	 *
	 * bl81-05801 combine 3 formatters into one.
	 */
#define	RP06_0
#include	"stdio.h"
#include	"sys/mtpr.h"
#include	"sys/param.h"
#include	"sys/types.h"
#include	"sys/buf.h"
#include	"sys/signal.h"
#include	"sys/sysmacros.h"
#include	"sys/io.h"


struct dlimits {
	int dtype, nsect, ntrak, ncyl;
	char *dname;
} d_lims[] = {
	{ 0x2012, 22, 19, 815, "RP06"},
	{ 0x2014, 32,  5, 823, "RM03"},
	{ 0x2017, 32, 19, 823, "RM05"},
	{ 0, 0, 0, 0, "???"}	/* leave last in case of funny type */
};
int dr_type;
#define	RP06	0
#define	RM03	1
#define	RM05	2
#define	NTYPES sizeof(d_lims)/sizeof(struct dlimits)
	/* the following must agree with data in the d_lims[] table */
#define	MAXTRK 19	/* highest # of tracks on any device */
#define	MAXSEC 32	/* highest # of sectors on any device */

#define	Rx_FMT 0xd000	/* format bit and good sector mark */
#define	RxCYL d_lims[dr_type].ncyl	/* no. cylinders/pack */
#define	RxTRK d_lims[dr_type].ntrak	/* no. tracks/cyl */
#define	RxSEC d_lims[dr_type].nsect	/* no. sectors/track */

/* tracks and heads are directly related as follows:
	1. the top and bottom platters on the disk are only covers
	2. starting on top of the second platter and continueing
	   downwards, the track assignment sequence is (RP06,RM05)
	   0,2,1,4,3,6,5,7,8,10,19,9,12,14,11,13,16,18,15,17.
	3. track 19 is the servo track.
	4. track assignments on HONEWELL disks are the same,
	   and we expect they both follow IBM's lead.
*/
#define	RxST	(RxSEC*RxTRK)	/* no. sectors/cyl */
#define	MBAinit	01	/* MBA init bit */
#define	MB_ERR	0x3080	/* expected error bits in MBA status reg */
#define	Rx_GO	1		/* go bit */
#define	Rx_OFF	014	/* head offset */
#define	Rx_RTC	016	/* Return to Centerline */
#define	Rx_RED	070	/* Read */
#define	Rx_RHD	072	/* Rx06 read sector header and date */
#define	Rx_WHD	062	/* Rx06 write sector header and data */
#define	Rx_WCH	052	/* Write Check Header and Data */
#define	Rx_DC	010	/* drive clear */
#define	Rx_RIP	0x10	/* Read-in Preset Rx06 */
#define	Rx_DRY	0x80	/* drive ready, status reg */
#define	Rx_ERR	0x4000	/* composite error, status reg */
#define	Rx_MOL	0x1000	/* medium-online bit in status reg */
#define	Rx_WRL	0x800	/* write lock status */
#define	Rx_ECH	0x40	/* ECC Hard Error */
#define	Rx_DCK	0x8000	/* ECC */
#define	Rx_HCRC	0x100	/* header crc err */
#define	Rx_HCE	0x80	/* header compare err */
#define	Rx_FER	0x10	/* format err */
#define	Rx_PAR	0x8	/* parity err */
#define	Rx_HCI	0x400	/* Header Compare Inhibit, offset reg */
#define	Rx_ECI	0x800	/* ECC Inhibit, offset reg	*/
#define	M_WCKU	0x400	/* Write Check Upper error status - MBA */
#define	M_WCKL	0x200	/* Write Check Lower error status - MBA */
#define	BLKSIZ	512
#define	MAXUNI	8	/* max. no. units on MBA */
#define	MAXERR	50
#define	PTRN2	0xeb6d	/* DEC defined */
#define	PTRN1	0xb6db	/* worst case pattern */
#define	OBOFF	(((int)obuf) & 0x1ff)
#define	IBOFF	(((int)ibuf) & 0x1ff)
#define	IBMAP0	(128 << 9)
#define	EQ(x, y)	(strcmp(x, y) == 0)
int Report = 8;
int Rxhead;	/* writable bytes in header */
int Rp;		/* rp=1 if device=RP06; rp=0 otherwise */
int Bufsz;	/* ((Rxhead + BLKSIZ) * RxSEC) */
union MBREGS {
	struct MBAR {
		int 	M_csr, M_cr, M_sr,
			M_var, M_bc;
	} Mbar;
	struct {
		char fill1[0x400];
		struct RxR {
			int	Rx_cr, Rx_ds, Rx_er1, Rx_mr;
			int	Rx_as, Rx_da, Rx_dt, Rx_la;
			int	Rx_sn, Rx_off, Rx_cyl, Rx_cc;
			int	Rx_err2, Rx_err3, Rx_Epos, Rx_Epat;
			int	fill2[16];
		} Rxreg[8];
	} Unit;
	struct MAP {
		long fill3[0x200], M_map[256];
	} Map;
} *mptr;
struct RPsect {
	short s_cyl[1];
	char  s_sec;
	char  s_trk;
	short s_kw1, s_kw2;	/* keywords for RP only */
	short s_data[256];
}; 
struct RMsect {
	short s_cyl[1];
	char  s_sec;
	char  s_trk;
	short s_data[256];
};
int track, cylndr, secnum, mnum, dunit, ecount, daterr, wcerr;
int cputype;
char *M_BASE;	/* Pointer to base of MBA blocks */
int defMBA, minMBA, maxMBA;	/* used as guides only */
char wcrflg;
char NEXnam[] = "NEXUS";
char MBAnam[] = "MBA";
char *mname;	/* points to either "NEXUS" or "MBA" */
char *ibuf, *obuf;	/* buffer pointers */
short fmtflg;
int Offset[]	/* Centerline offsets */
	= {0, 0x08, 0x88, 0x10, 0x90, 0x18, 0x98, -1
}; 
struct EL {
	int e_typ, e_cyl, e_trk, e_err;
} errlst[MAXERR];
struct RxR *Rxptr,Test;
extern char *malloc();
extern char *errtyp();
	/*
	* Stand-alone program to format Rx06 disk.
	* User specifies disk unit.
	*/
main() 
{
	int i, j;
	char c;
#ifndef TEST
	if (ckcpu()) return;
	/* set up MBA base address */
	switch (cputype) {
	case 780:
		M_BASE = (char *)0x20010000;	/* NEXUS 8 */
		defMBA = 8;	/* NEXUS number */
		minMBA = 8;	/* lo NEXUS for MBA */
		maxMBA = 11;	/* hi NEXUS for MBA */
		mname = NEXnam;
		break;
	case 750:
		M_BASE = (char *)0xf28000;
		defMBA = 0;	/* MBA number */
		minMBA = 0;	/* lo MBA */
		maxMBA = 3;	/* hi MBA */
		mname = MBAnam;
		break;
	default:
		printf("Can only be used on VAX-11/780 and 11/750\n");
	}
	if(ckcomp()) return;
	printf("\n\nVAX-11/%d Multi-type pack formatter\n", cputype);
#endif
	obuf = malloc((sizeof (struct RPsect)) * 32);
	ibuf = malloc((sizeof (struct RPsect)) * 32);
	dunit = 0;
	mnum = defMBA;
#ifndef TEST
	obinit(mnum);	/* setup map registers and output buffer */
#endif
	ungetc('v',stdin);	/* print vocabulary */
	while((c = getc(stdin)) != 'q'){
		switch ( c ) {
	case 'm':
		if ((getnum(&mnum))  == 0 || mnum < minMBA || mnum > maxMBA){
			printf("need %s number from %d to %d\n",
				mname, minMBA, maxMBA);
			tossin(1);
			mnum = defMBA;	/* reset to default */
		}
#ifndef TEST
		obinit(mnum);	/* setup map registers and output buffer */
#endif
		break;
	case 'd':
		if ((getnum(&dunit)) == 0 || dunit <= 0 || dunit >= MAXUNI){
			printf("need unit number from 1 to %d\n",MAXUNI - 1);
			tossin(1);
			dunit = 0;	/* reset */
		}
		break;
	case 'f':
		if(drchk()) break;
		printf("\n *** The %s on drive %d, %s %d",
			d_lims[dr_type].dname,dunit,mname,mnum);
		printf(" will be over-written during formatting ***\n");
		tossin(0);
		printf("\nDO YOU WISH TO CONTINUE? (y or n) ");
		if ((c = getc(stdin)) != 'y'){
			printf("*** FORMAT CANCELLED ***");
			if(c == '\n') ungetc(c,stdin);
			break;
		}
		tossin(1);
		fmtflg++;
		if (pkinit()) {
			printf("init error\n");
			break;
		}
	
		Rxptr->Rx_off = Rx_FMT | Rx_ECI; /* format and ECC inhibit */
		printf("\n\n *** FORMATTING PACK ***\n\n");
		ecnt0();
		for (cylndr = 0; cylndr < RxCYL; cylndr++) {
			for (track = 0; track < RxTRK; track++) {
				if (format(OBOFF,Bufsz,0)) break;
				if (wcheck(OBOFF,Bufsz,0)) break;
				trkinc();
			}
			cylinc();
			trkclr();
			if(Report && ((cylndr % 50) == 0))
				printf("\n---cyl  %d---\n",cylndr);
		}
	
		esum();
		if(Report) edump();
		printf("\n\n*** FORMAT COMPLETE *** \n\n");
		printf("\n\n");
	case 'c':
		if(drchk()) break;
		printf("\n*** checking %s on drive %d, %s %d ***\n",
			d_lims[dr_type].dname,dunit,mname,mnum);
		tossin(1);
		ecnt0();
		fmtflg = 0;
		if(pkinit()) {
			printf("init error\n");
			break;
		}
		Rxptr->Rx_off = Rxptr->Rx_off | Rx_FMT;
		for (cylndr = 0; cylndr < RxCYL; cylndr++) {
			for (track = 0; track < RxTRK; track++) {
				if (dread(IBOFF,BLKSIZ * RxSEC,0)) break;
				trkinc();
			}
			cylinc();
			trkclr();
			if(Report && ((cylndr % 50) == 0))
				printf("\n---cyl  %d---\n",cylndr);
		}
		esum();
		if(Report) edump();
		printf("\n\n*** CHECK COMPLETE *** \n\n");
		break;
	case 'X':
		whoa (9);
		break;
	case 'R':
	case 'T':
		if(getnum(&Report) == 0) {
			printf("\ninvalid Report level; ");
			Report = 0;
		}
		printf("\nReport level = %d\n",Report);
		break;
	case 'v':
		printf("\nCommands:\n\tm n - %s, ", mname);
		printf("%d<=n<=%d (defaults to %d)",minMBA,maxMBA,minMBA);
		printf("\n\td n - Drive number, ");
		printf("1<=n<=%d (must pick one)",MAXUNI - 1);
		printf("\n\tf   - format pack");
		printf("\n\tc   - check pack format");
		printf("\n\tRn  - set Report level to n");
		printf("\n\tX   - to execute the report trap ");
		printf("\n\tq   - quit");
		printf("\n\tv   - print vocabulary");
	case '\n':
		printf("\n? ");
	case ' ':
		break;
	case 'Z':
		Rxptr = &Test;
		ecount = 0;
		printf("\n number of errors ? ");
		getnum(&j);
		for(i = 0; i < j; i++) {
			Rxptr->Rx_cyl = 10 * i;
			Rxptr->Rx_da = (i + (256 * i)) & 0x1f1f;
			if(logerr(i%10 + 1,0x12345)) break;
		}
		esum();
		edump();
		break;
	default:
		ungetc(c,stdin);
		printf("\n dont know how to '");
		while((c = getc(stdin)) != '\n') printf("%c",c);
		printf("'.\n? ");
		}
	}
}
getnum(num)
int *num;
{
	char c;
	while((c =getc(stdin)) == ' ' || c == '\n');
	ungetc(c,stdin);
	if ((c = getc(stdin)) != '0') {
		ungetc(c,stdin);
		if (scanf("%d",num) == 0) return(0);
		return(1);
	}
	if ((c = getc(stdin)) != 'x') {
		ungetc(c,stdin);	/* may be a number, may not. */
		ungetc('0',stdin); /* leading 0 be only # */
		if (scanf("%o",num) == 0) return(0);
	}
	else {
		if (scanf("%x",num) == 0) return(0);
	}
	return(1);
}
tossin(n)
int n;
{
	while(getc(stdin) != '\n');
	if (n) ungetc('\n',stdin);
	return;
}
	/*
	*	reset error counters
	*/
ecnt0()
{
	ecount = daterr = wcerr = 0;
}
	/*
	 * check that a drive has been specified and
	 * that it is a known type
	*/
drchk()
{
	if (dunit == 0){
		printf("\nmust give a drive number.\n");
		return(-1);
	}
	mptr->Mbar.M_cr = MBAinit;	/* MBA init */
	Rxptr = &mptr->Unit.Rxreg[dunit];

	for(dr_type = 0; dr_type < NTYPES; dr_type++)
		if((Rxptr->Rx_dt & 070777) == d_lims[dr_type].dtype) break;
	if(d_lims[dr_type].nsect == 0){
		printf("*** Cannot format this device *** ");
		return(-1);
	}
	if (EQ("RP06", d_lims[dr_type].dname)) {
		Rxhead = sizeof (struct RPsect) - 512;
		Rp = 1;
	} else {
		Rxhead = sizeof (struct RMsect) - 512;
		Rp = 0;
	}
	Bufsz = (Rxhead + BLKSIZ) * RxSEC;
	return(0);
}

 
	/*
	* Initialize MBA (disk) .
	* Set up MBA map register to map a 
	* output buffer uses 1st 128 map reg's -
	* input buffer uses last 128 map reg's 
	*/
obinit(mnum)
int mnum;
{
	register int page, *mp0, i;
	mptr = (union MBREGS *)(M_BASE + ((mnum-minMBA)*0x2000));
	page = (int)((int)obuf>>9) & 0x1fffff;
	mp0 = mptr->Map.M_map;
	for (i = 0; i < 128; i++)
		(*mp0++) = 0x80000000 | page++;
 
	page = (int)((int)ibuf>>9) & 0x1fffff;
	for (i = 0; i<128; i++ )
		(*mp0++) = 0x80000000 | page++;
	return;
}
	/*
	* Pack initialization.
	* transfer of 'Bufsz' bytes.
	*/
pkinit() 
{
	register int stat, i, j;

	if (((stat = (Rxptr->Rx_ds)) & Rx_MOL) == 0) {
		printf("unit OFF-LINE\n");
		return(-1);
	}
	if(fmtflg){
		if(stat & Rx_WRL){
			printf("unit WRITE-PROTECTED\n");
			return(1);
		}
		/*
		*  initialize output buffer with sector header and data:
		* 	sector header words:
		* 		cylinder
		* 		track/sector
		* 		key word 1 (RP06 only)
		* 		key word 2 (RP06 only)
		* 	256 data words
		*/

		switch (dr_type) {
		case RP06: {
			register struct RPsect *RPsecp;
			RPsecp = (struct RPsect *)obuf;
			for (i = 0; i < RxSEC; i++) {
				RPsecp->s_cyl[0] = Rx_FMT;
				RPsecp->s_kw1 = RPsecp->s_kw2 = 0;
				RPsecp->s_sec = i;
				RPsecp->s_trk = 0;
				for (j = 0; j < 256;) {
					RPsecp->s_data[j++] = PTRN1;
					RPsecp->s_data[j++] = PTRN2;
				}
				RPsecp++;
			}
			break;
		}
		case RM03:
		case RM05: {
			register struct RMsect *RMsecp;
			RMsecp = (struct RMsect *)obuf;
			for (i = 0; i < RxSEC; i++) {
				RMsecp->s_cyl[0] = Rx_FMT;
				RMsecp->s_sec = i;
				RMsecp->s_trk = 0;
				for (j = 0; j < 256;) {
					RMsecp->s_data[j++] = PTRN1;
					RMsecp->s_data[j++] = PTRN2;
				}
				RMsecp++;
			}
			break;
		}
		default:
			printf("Invalid drive type\n");
			return (1);
		}
	}
	Rxptr->Rx_cr = Rx_RIP | Rx_GO; /* drive preset - sets vv */
	dwait(Rxptr);
	ecount = daterr = wcerr = 0;
	return(0);
}
	/*
	* write 'nb' bytes worth of sector and data info
	* from buffer whose map/offset is 'baddr' to disk.
	*/
format(baddr,nb,s)
int baddr, nb, s;
{
	register int i;
	register struct RxR *R;
	register struct MBAR *M;
	R = Rxptr;
	M = &mptr->Mbar;
	R->Rx_cr = Rx_DC | Rx_GO;
	dwait(R);
	setadr(baddr,nb,s);
	R->Rx_cr = Rx_WHD | Rx_GO; /* write sector header and data */
	dwait(R);	
	if (i = derror(R)) {
		cursec(nb,s);
		logerr(3,i);
		if (wcrflg) return(1);	/* stop if 'wrt check' rec. */
		if (ecount > MAXERR) return(-1);
	}
	if (i = mbaerr(M)) {
		cursec(nb,s);
		if(logerr(1,i) || (wcrflg == 0 )) return(-1);
	}
	return(0);	
}
	/*
	* With ECC enabled,
	* read 'nb' bytes into buffer pointed to
	* by map 'baddr' - start at sector 's'.
	*/
dread(baddr,nb,s)
int baddr, nb, s;
{
	register int i;
	register struct MBAR *M;
	register struct RxR *R;
	R = Rxptr;
	M = &mptr->Mbar;
	setadr(baddr,nb,s);
rini:
	R->Rx_cr = Rx_DC | Rx_GO;	
	dwait(R);
	dwait(R);
	R->Rx_cr = Rx_RED | Rx_GO;	
	dwait(R);	
	cursec(nb,s);
	if (i = derror(R)) {	
		if (wcrflg) return(-1);
		if (i & (Rx_HCE|Rx_HCRC|Rx_FER|Rx_PAR)){
			logerr(14,i);
			return(1);
		}
		if (i & Rx_DCK) {	
			daterr++;
			if (i & Rx_ECH) {	/* ECC Hard Error */
				if(logerr(5,R->Rx_Epat)
					|| wcrflg) return(-1);
			}
			else {		/* ECC recoverable */
				logerr(8,R->Rx_Epat & 0xfff);
			}
			if (M->M_bc) {	/* more i-o to complete */
				goto rini;
				/* status reg cleared by Drive Clear */
			}
		}
		else {
			if( i & Rx_FER)
				logerr(9,i);
			else
				logerr(4,i);
		}
		if (wcrflg) return(0);
	}
	if (i = mbaerr(M)) {
		logerr(1,i);
		return(-1);
	}
	return(0);
}
mbaerr(mba)
register struct MBAR *mba;
{
	register int i;
	if ((i = mba->M_sr) == 0x2000 || i == MB_ERR)
		return(0); 
	mba->M_sr = (-1);
	if(Rxptr->Rx_as != (1 << dunit)) return(0);
	Rxptr->Rx_as = Rxptr->Rx_as;
	return(i);
}
	/*
	* write check 'nb' bytes of sector and data info
	* from buffer whose map reg is 'baddr' - start at sector 's'.
	*/
wcheck(baddr,nb,s)
int baddr, nb, s;
{
	register int i, j;
	static int osec;
	register struct RxR *R;
	register struct MBAR *M;
	R = Rxptr;
	M = &mptr->Mbar;
	R->Rx_cr = Rx_DC | Rx_GO;
	dwait(Rxptr);
	setadr(baddr,nb,s);
	R->Rx_cr = Rx_WCH | Rx_GO; 
	dwait(Rxptr); 
	j = cursec(nb,s);
	if (i = derror(R))  {
		if(R->Rx_er1 & Rx_DCK) {
			logerr(7,i);
			R->Rx_cr = Rx_RIP | Rx_GO;	/* resets ECC inhibit */
			R->Rx_off = R->Rx_off | Rx_FMT;
			if((j = dread(IBOFF + IBMAP0,Rxhead + BLKSIZ,j)) == 0)
				logerr(8,R->Rx_Epat & 0xfff);
			R->Rx_off = R->Rx_off | Rx_ECI | Rx_FMT;
			return(i);
		}
	}
	else if (i = mbaerr(M)) {
		if (i & (M_WCKU | M_WCKL)) { 
			wcerr++;
			if(wcrflg) {
				if(osec != j)
					logerr(1,i);
				osec = j;
				return(1);
			}
			osec = -1;
			wckrcv(j);
		}
	}
	if (ecount > MAXERR) return(-1);
	return(0);
}
	/*
	* Try to recover from a 'write check' error during a
	* 'Write Check Header and Data' function.
	* 'Rxptr' is ptr to Rx register set.
	* 'mptr' is ptr to MBA reg set.
	* MBA byte count reg has neg. no. bytes remaining - transfer 
	* stops on error, even in middle of a sector.
	* Loop with a 'Write Sector Header and Data' followed by a 
	* 'Read Sector Header and Data' - each loop iteration uses 
	* a different head centerline offset.
	* If all fails, report irrecoverable error and finish off the
	* 'write check data and header' on the rest of the track.
	*/
wckrcv(j)
int j;
{
	register int i, k, l, m;
	wcrflg++;
	m = j * (BLKSIZ+Rxhead);	/* data offset in buffer */
	/* See if a simple reread (3) will fix this problem */
	for(l = 1; l <= 16; l++){
		if(wcheck(m + OBOFF,BLKSIZ + Rxhead,j)) continue;
		logerr(12,l);
		goto wckfin;
	}
	/*
	* Try to recover using 
	* different centerline offset - 1st offset is 0.
	*/
	for (i = 0; (l = Offset[i]) >= 0; i++) {
		k = Rxptr->Rx_off & 0xff00;
		Rxptr->Rx_off = k | l;
		Rxptr->Rx_cr = Rx_DC | Rx_GO;
		dwait(Rxptr);
		Rxptr->Rx_cr = Rx_OFF | Rx_GO;
		dwait(Rxptr);
		if (format(m + OBOFF,BLKSIZ + Rxhead,j)) continue;
		if (rshd(IBMAP0 + IBOFF,BLKSIZ + Rxhead,j) < 0) continue;
		if (cksec(ibuf,obuf+m)) continue;
		secnum = j;
		logerr(6,(i << 12) | l);
		goto wckfin;
	}
	secnum = j;
	logerr(10,Rxptr->Rx_da);
wckfin :
	Rxptr->Rx_cr = Rx_RTC | Rx_GO; /* return to centerline */
	dwait(Rxptr);
	/*
	* Continue write check where
	* the original WCHD left off.
	*/
	if(++j >= RxSEC)  {
		wcrflg = 0;
		return;
	}
	i = j * (BLKSIZ + Rxhead);
	if(wcheck(i + OBOFF,(RxSEC - j) * (BLKSIZ + Rxhead),j))
				logerr(13,i);
	wcrflg = 0;
	return;
}
	/*
	* read 'nb' bytes into buffer pointed to
	* by map and offset in 'baddr' - start at sector 's'
	*/
rshd(baddr,nb,s)
int baddr, nb, s;
{
	register int i, j;
	register struct MBAR *M;
	register struct RxR *R;
	R = Rxptr;
	M = &mptr->Mbar;
	setadr(baddr,nb,s);
	R->Rx_cr = Rx_DC | Rx_GO; 
	dwait(R);
	R->Rx_cr = Rx_RHD | Rx_GO; /* read sector header and data */
	dwait(R);	/* wait for i/o to finish */
	if (i = derror(R)) {	
		cursec(nb,s);
		if(i & (Rx_HCRC|Rx_HCE|Rx_FER)){
			logerr(14,i);
			return(-1); 
		}
		if(R->Rx_er1 & Rx_DCK) {
			R->Rx_cr = Rx_RIP | Rx_GO;	/* resets ECC inhibit */
			R->Rx_off = R->Rx_off | Rx_FMT;
			if((j = dread(IBOFF + IBMAP0,Rxhead + BLKSIZ,s)) == 0)
				logerr(8,R->Rx_Epat & 0xfff);
			R->Rx_off = R->Rx_off | Rx_ECI | Rx_FMT;
			return(i);
		}
		else logerr(2,i);
		return(i);
	}
	if (i = mbaerr(M)) {
		return(-1);
	}
	return(i);
}
	/* setup data transfer addresses */
setadr(baddr,nb,s)
int baddr, nb, s;
{
	Rxptr->Rx_cyl = cylndr;	
	Rxptr->Rx_da = (track<<8) | s;	
	mptr->Mbar.M_bc = (-nb);
	mptr->Mbar.M_var = baddr; /* virt addr reg = map no. + byte off */
}
	/*
	* Compare sector header and data info on disk against that
	* which was written from output buffer
	*/
cksec(insec,ousec)
register struct RPsect *insec, *ousec;
{
	register int j;

	for (j = 0; j < ((Rxhead)>>1); j++)
		if (insec->s_cyl[j] != ousec->s_cyl[j])
			return(1);

	switch (dr_type) {
	case RP06:
		for (j = 0; j < 256; j++)
			if (insec->s_data[j] != ousec->s_data[j])
				return(1);
		break;
	case RM03:
	case RM05: {
		register struct RMsect *inp, *oup;
		inp = (struct RMsect *) insec;
		oup = (struct RMsect *) ousec;
		for (j = 0; j < 256; j++)
			if (inp->s_data[j] != oup->s_data[j])
				return(1);
		break;
	}
	default:
		printf("(cksec): Invalid drive type\n");
		return 1;
	}
	return(0);
}
	/*
	* wait Rx06 disc unit to be ready.
	*/
dwait(rptr)
struct RxR *rptr;
{
	while ((rptr->Rx_ds & Rx_DRY) == 0);
}
	/*
	* check for Rx06 error.
	*/
derror(rptr)
struct RxR *rptr;
{
	if (rptr->Rx_ds & Rx_ERR) 
		return(rptr->Rx_er1 & 0xffff);
	return(0);
}
	/*
	* given sector and count for last transfer, determine
	* the last partial sector transferred.
	*/
cursec(nb,s)
int nb,s;
{
	register int i, j;
	if((i = mptr->Mbar.M_bc >> 16) == 0){
		secnum = (Rxptr->Rx_da & 0x1f) -1;
	}
	else {
		j = nb + i;	/* bytes transferred */
		secnum = ((j - 1) /(Rxhead + BLKSIZ)) + s;
	}
	if(secnum >= RxSEC) {
		printf("\n*** sector conversion error");
		printf("\n*** M_bc %x, bytes %x, sector %x ***\n",
			i,j,secnum);
		whoa(2);
	}
	if(secnum < 0 ) secnum =  RxSEC - 1;
	return(secnum);
}
	/*
	* modify track/sector no. in output buffer sector headers.
	*/
trkinc()
{
	register struct RPsect *RPsp;
	register struct RMsect *RMsp;
	register int i;

	if (Rp)
		for (i = 0,RPsp = (struct RPsect *)obuf; i < RxSEC; i++,RPsp++)
			RPsp->s_trk++;
	else
		for (i = 0,RMsp = (struct RMsect *)obuf; i < RxSEC; i++,RMsp++)
			RMsp->s_trk++;
}
cylinc() 
{
	register struct RPsect *RPsp;
	register struct RMsect *RMsp;
	register int i;

	if (Rp)
		for (i = 0,RPsp = (struct RPsect *)obuf; i < RxSEC; i++,RPsp++)
			RPsp->s_cyl[0]++;
	else
		for (i = 0,RMsp = (struct RMsect *)obuf; i < RxSEC; i++,RMsp++)
			RMsp->s_cyl[0]++;
}
trkclr()
{
	register struct RPsect *RPsp;
	register struct RMsect *RMsp;
	register int i;

	if (Rp)
		for (i = 0,RPsp = (struct RPsect *)obuf; i < RxSEC; i++,RPsp++)
			RPsp->s_trk = 0;
	else
		for (i = 0,RMsp = (struct RMsect *)obuf; i < RxSEC; i++,RMsp++)
			RMsp->s_trk = 0;
}
	/*
	*	check we were assembled standalone for the VAX
	*/
ckcomp()
{
#ifndef vax
	printf("available only on the VAX-11/780 and 11/750\n");
#endif
#ifndef STANDALONE
	printf("available STAND-ALONE only \n");
	return(1);
#endif
#ifndef vax
	return(1);
#else
	return(0);
#endif
}

	/* return contents of privileged register */
mfpr(regno)
{
        asm("   mfpr    4(ap),r0");
}

	/* check the System ID Register for valid cpu types */
ckcpu()
{
	long sid;

        sid = mfpr(SID);	/* get system id */
	switch (sid & 0xff000000) {
	case 0x1000000:
		cputype = 780;
		return(0);
	case 0x2000000:
		cputype = 750;
		return(0);
	case 0x3000000:
		cputype = 730;
		return(0);
	default:
		break;
	}
	return(1);
}
	/*
	*	log errors in erlist
	*/
logerr(code,stat)
int code, stat;
{
	errlst[ecount].e_typ = code;
	errlst[ecount].e_cyl = Rxptr->Rx_cyl & 0xffff;
	errlst[ecount].e_trk = Rxptr->Rx_da;
	errlst[ecount].e_err = stat;
	if(secnum == (RxSEC -1)){
		errlst[ecount].e_trk += ((-1) << 8);
	}
	prloc();
	printf(":  %s, %8x\n",errtyp(code),stat);
	if(++ecount > (MAXERR - 5)) {
		Report = 9;
		printf("\nerror count is %d of %d MAX\n",
			ecount,MAXERR);
	}
	if(Report != 8) whoa(Report);
	if(ecount >= MAXERR) return(-1);
	return(0);
}
	/*
	*	summarize errors by type and location
	*/
esum()
{
	int n, h, bw, badpk,badecc;
	register int slice;
	register struct EL *elptr;
	int ernum, endcyl, ocyl, otrk, osec;
	int *hitcyl, hittrk[MAXTRK], hitsec[MAXSEC],
		hcyl[8], htrk[8], hsec[8], hard[8], soft[8], xsec[8],
		headr[8], dta[8], read[8], write[8],
		minb[8],maxb[8];
	if(ecount == 0){
		printf("\nNO ERRORS\n");
		return;
	}
	printf("\n Total errors: %d",ecount);
	hitcyl = (int *)ibuf; /* scratch space */
	for(n = 0; n < RxCYL; n++) *(hitcyl + n) = 0;
	for(n = 0; n < RxTRK; n++) hittrk[n] = 0;
	for(n = 0; n < RxSEC; n++) hitsec[n] = 0;
	for(n = 0;n < 8; n++){
		hcyl[n] = htrk[n] = hsec[n] = 0;
		hard[n] = soft[n] = xsec[n] = 0;
		headr[n] = dta[n] = read[n] = write[n] = 0;
		minb[n] = 13;	/* 12 is max field width */
		maxb[n] = 0;
	}
	ernum = badecc = badpk = 0;
	elptr = errlst;
	ocyl = otrk = osec = -1;
	for(slice = 0; slice < 8; slice++) {
		if((endcyl = hp_sizes[slice + 1].cyloff) == 0)
			endcyl = RxCYL;
		while((ernum < ecount) && (elptr->e_cyl < endcyl)) {
			if((n = elptr->e_cyl) < RxCYL){
				(*(hitcyl + n))++;
				if(n != ocyl){
					hcyl[slice]++;
					ocyl = n;
					otrk = osec = -1;
				}
			}
			else printf("\nbad cylinder data %x\n",
				elptr->e_cyl);
			if((n = (elptr->e_trk >> 8) & 0x1f) < RxTRK){
				hittrk[n]++;
				if(n != otrk){
					htrk[slice]++;
					otrk = n;
					osec = -1;
				}
			}
			else printf("\nbad track data %x\n",
				elptr->e_trk);
			if((n = (elptr->e_trk & 0x1f)) < RxSEC){
				(*(hitsec + n))++;
				if(n != osec){
					hsec[slice]++;
					osec = n;
				}
			}
			else printf("\nbad sector data %x\n",
				elptr->e_trk);
			switch (elptr->e_typ) {
			case 2:
			case 6:
				read[slice]++;
				headr[slice]++;
				break;
			case 3:
				write[slice]++;
				headr[slice]++;
				break;
			case 4:
				read[slice]++;
				dta[slice]++;
				break;
			case 5:
				hard[slice]++;
				badpk++;
				break;
			case 7:
				write[slice]++;
				dta[slice]++;
				break;
			case 8:
				soft[slice]++;
				if((bw = brstw(&elptr->e_err)) > maxb[slice])
					maxb[slice] = bw;
				if((bw < minb[slice]) && bw)
					minb[slice] = bw;
				break;
			case 9:
				break;
			case 10:
			case 13:
				hard[slice]++;
				xsec[slice]++;
				break;
			case 12:
				soft[slice]++;
				break;
			}
			ernum++;
			elptr++;
		}
		if(hcyl[slice])
			printf("\nDisk slice %d error summary",slice);
		if(hard[slice]){
			printf("\n\n\t%d\tHard errors",hard[slice]);
		}
		if(soft[slice]){
			printf("\n\t%d\tRecoverable errors",soft[slice]);
			if(!fmtflg)
			printf("\n\t\tmin burst : %d, max burst : %d",
				minb[slice] % 13, maxb[slice]);
			if(maxb[slice] > 8) badecc++;
		}
		if(hcyl[slice]){
			printf("\n\t%d\tBad cylinders",hcyl[slice]);
		}
		if(htrk[slice]){
			printf("\n\t%d\tBad tracks",htrk[slice]);
		}
		if(hsec[slice]){
			printf("\n\t%d\tBad sectors",hsec[slice]);
		}
		if(write[slice]){
			printf("\n\t%d\tWrite errors",write[slice]);
		}
		if(read[slice]){
			printf("\n\t%d\tRead errors",read[slice]);
		}
		if(headr[slice]){
			printf("\n\t%d\tHeader errors",headr[slice]);
		}
		if(dta[slice]){
			printf("\n\t%d\tData errors",dta[slice]);
		}
	
	}
	if(badecc){
		printf("\n\n*** This pack has %d ECC error",badecc);
		if(badecc > 1) printf("s");
		printf(" That FAIL");
		if(badecc < 2) printf("S");
		printf(" new pack requirements ***\n");
		printf("*** If this is a new pack, ");
		printf("return it to the vendor ***\n\n");
	}
	if(badpk && !fmtflg){
		printf("\n\n*** This pack has %d HARD error",badpk);
		if(badpk > 1) printf("s");
		printf(". It is UNUSABLE as a system pack ***\n\n");
		printf("*** This pack must be refurbished ***\n\n");
	}
	printf("\n\nPack summary\n");
	printf("\n\nCylinder hits\n");
	for(n = 0;n < RxCYL; n++){
		h = *(hitcyl + n);
		if(h){
			printf("\n %d:\t%d hit",n,h);
			if(h > 1) printf("s");
		}
	}
	printf("\n\nTrack hits\n");
	for(n = 0;n < RxTRK; n++)
		if(hittrk[n]){
			printf("\n %d:\t%d hit",n,hittrk[n]);
			if(hittrk[n] > 1) printf("s");
		}
	printf("\n\nSector hits\n");
	for(n = 0;n < RxSEC; n++)
		if(hitsec[n]){
			printf("\n %d:\t%d hit",n,hitsec[n]);
			if(hitsec[n] > 1) printf("s");
		}
}
brstw(eptr)
int *eptr;
{
	int i, j, b1, b2;
	b1 = b2 = 0;
	j = *eptr;
	for(i = 1;i < 13; i++){
		if(j % 2) {
			if( b1 == 0) b1 = i;
			b2 = i + 1;
		}
		j = j / 2;
	}
	return(b2 - b1);
}
	/*
	*	dump the error list
	*/
edump()
{
	int i;
	if(ecount == 0)	return;
	printf("\n\nerror file dump");
	printf("\n\nCYL\tTRK\tSECT\t  STATUS\t ERROR TYPE\n");
	for(i = 0; i < ecount; i++)
		printf("\n%3d\t%3d\t%3d\t%8x\t%s",
		errlst[i].e_cyl,
		(errlst[i].e_trk >> 8) & 0x1f,
		errlst[i].e_trk & 0x1f,
		errlst[i].e_err,
		errtyp(errlst[i].e_typ));
	printf("\n");
}
	/*
	*	print cyl-trk-sect address
	*/
prloc()
{
	printf("\ncyl %d, trk %d, sec %d",
		cylndr,track,secnum);
}
	/*
	*	decode error type given the code number
	*/
char *
errtyp(n)
int n;
{
	switch(n) {
	case 1: return("MBA error");
	case 2: return("Read SHD");
	case 3: return("Write SHD");
	case 4: return("Read error");
	case 5: return("ECC; Non-recoverable");
	case 6: return("Write Check; Recoverable with head offset");
	case 7: return("Data check");
	case 8: return("ECC; Recoverable");
	case 9: return("Format error");
	case 10: return("Bad sector");
	case 11: return("Wckrcv sect. compare");
	case 12: return("Write Check; Recoverable by re-read");
	case 13: return("Multiple wrt-cks on track");
	case 14: return("Header Compare error");
	default: return("undefined");
	}
}
	/* debugging trap */
whoa(n)
int n;
{
	int i, j, *mp;
	static int passn = 1;
	char c;
	switch(n){
case 0: return;
case 1:
	printf("\n\nMBA Regs:\
		\n csr  %9x\n cr   %9x\n sr   %9x\
		\n var  %9x\n bc   %9x\n",
		mptr->Mbar.M_csr,
		mptr->Mbar.M_cr,
		mptr->Mbar.M_sr,
		mptr->Mbar.M_var,
		mptr->Mbar.M_bc);
	break;
case 2:
case 7:
	printf("\n\nMBA Regs:\
		\n csr  %9x\n cr   %9x\n sr   %9x\
		\n var  %9x\n bc   %9x\n",
		mptr->Mbar.M_csr,
		mptr->Mbar.M_cr,
		mptr->Mbar.M_sr,
		mptr->Mbar.M_var,
		mptr->Mbar.M_bc);
case 3:
	printf("\nDrive Regs:\
		\n cr   %5x\tds   %5x\ter1  %5x\tmr   %5x\
		\n as   %5x\tda   %5x\tdt   %5x\tla   %5x\
		\n sn   %5x\toff  %5x\tcyl  %5x\tcc   %5x\
		\n err2 %5x\terr3 %5x\tepos %5x\tepat %5x\n",
		Rxptr->Rx_cr & 0xffff,
		Rxptr->Rx_ds & 0xffff,
		Rxptr->Rx_er1 & 0xffff,
		Rxptr->Rx_mr & 0xffff,
		Rxptr->Rx_as & 0xffff,
		Rxptr->Rx_da & 0x1f1f,
		Rxptr->Rx_dt & 0xffff,
		Rxptr->Rx_la & 0xffff,
		Rxptr->Rx_sn & 0xffff,
		Rxptr->Rx_off & 0xffff,
		Rxptr->Rx_cyl & 0xffff,
		Rxptr->Rx_cc & 0xffff,
		Rxptr->Rx_err2 & 0xffff,
		Rxptr->Rx_err3 & 0xffff,
		Rxptr->Rx_Epos & 0xffff,
		Rxptr->Rx_Epat & 0xffff);
		prloc();
		break;
case 8:
	prloc();
	return;
case 9:
	printf("\n***");
	}
	while(c != 'R'){
		if(passn < 2){
			printf("\nType 'c' fo continue,");
			printf("\n     'Rn' to change this trap to level n.");
			printf("\n and '<ctl> p' and 'H' to halt the system..");
			printf("\n\nReport levels:");
			printf("\n 9 - trap, no printout");
			printf("\n 8 - pack address info, NO trap");
			printf("\n 3 - drive registers");
			printf("\n 2 - things above and MBA regs");
			printf("\n 1 - MBA regs only");
			printf("\n 0 - no trap, re-enable with 'Rn' command");
		}
		printf("\nContinue?...");
		passn++;
		tossin(0);
		if((c = getc(stdin)) == 'c') return;
	}
	if(getnum(&Report) == 0) {
		printf("\ninvalid level");
		Report = 0;
	}
	printf("\nReport level %d\n",Report);
	return;
}

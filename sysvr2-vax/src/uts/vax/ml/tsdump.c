/* @(#)tsdump.c	6.1 */
/*
 * Dump core to magtape
 * Assumes memory mapping has been disabled
 * and IPL is set high
 */

typedef	unsigned short ushort;

struct	device {
	ushort	tsdb;
	ushort	tssr;
};

/* status message */
struct	sts {
	ushort	s_sts;
	ushort	len;
	ushort	rbpcr;
	ushort	xs0;
	ushort	xs1;
	ushort	xs2;
	ushort	xs3;
};

/* Error codes in stat 0 */
#define	TMK	0100000
#define	RLS	040000
#define	ONL	0100
#define	WLE	04000

/* command message */
struct cmd {
	ushort	c_cmd;
	ushort	c_loba;
	ushort	c_hiba;
	ushort	c_size;
};

#define	ACK	0100000
#define	CVC	040000
#define	IE	0200
#define	READ	01
#define	REREAD	01001

#define	SETCHR	04

#define	WRITE	05
#define	REWRITE	01005

#define	SFORW	010
#define	SREV	0410
#define	REW	02010

#define	WTM	011

#define	GSTAT	017

/* characteristics data */
struct charac {
	ushort	char_loba;
	ushort	char_hiba;
	ushort	char_size;
	ushort	char_mode;
};

/* Bits in (unibus) status register */
#define	SC	0100000
#define	SSR	0200
#define	OFL	0100
#define	NBA	02000

struct tsmesg {
	struct	cmd ts_cmd;
	struct	sts ts_sts;
	struct	charac ts_char;
	int	align;
};

static struct tsmesg dts;
static dtsinfo;

extern dump_addr;
static *ubmap;

#define	NBLK	16

tsdump()
{
	extern physmem;
	extern short cputype;

	printf("\nTape Dump\n");
	printf("%d memory blocks\n%d blocks per record\n", physmem, NBLK);
	printf("1600 bpi\n");
	if (cputype == 780) {
		dump_addr += 0x20130000;
		ubmap = (int *)0x20006800;
	} else {
		dump_addr += 0xff0000;
		ubmap = (int *)0xf30800;
	}
	twall((char *)0, physmem);	/* write out memory */
	tcom(WTM);
	tcom(WTM);
	tcom(REW);
}

static
twall(start, num)
register char *start;
register num;
{
	tinit();
	while (num) {
		twrite(start);
		start += 512*NBLK;
		num -= NBLK;
	}
}

static
tinit()
{
	register struct device *tsaddr = (struct device *) dump_addr;
	register struct tsmesg *tsm;
	register int *ubap = ubmap;
	register i;

	tsaddr->tssr = 0;
	while ((tsaddr->tssr&SSR)==0)
		;
	i = (int)&dts;
	i &= 0xfffff;
	dtsinfo = ((i&0777)|NBLK<<9);
	tsm = (struct tsmesg *)dtsinfo;
	i >>= 9;
	i |= 0x80000000;
	*(ubap+NBLK) = i;
	*(ubap+NBLK+1) = i+1;
	dts.ts_cmd.c_cmd = ACK | SETCHR;
	dts.ts_cmd.c_loba = (ushort) &tsm->ts_char;
	dts.ts_cmd.c_hiba = 0;
	dts.ts_cmd.c_size = sizeof(dts.ts_char);
	dts.ts_char.char_loba = (ushort) &tsm->ts_sts;
	dts.ts_char.char_hiba = 0;
	dts.ts_char.char_size = sizeof(dts.ts_sts);
	dts.ts_char.char_mode = 0;
	tsaddr->tsdb = dtsinfo;
	twait();
}

static
twrite(buf)
register char *buf;
{
	register struct device *tsaddr = (struct device *) dump_addr;
	register i, pte, *ubap = ubmap;
	int	err;

	pte = (((int)buf)>>9) | 0x80000000;
	for (i=0; i<NBLK; i++)
		*(ubap+i) = pte+i;
	dts.ts_cmd.c_loba = 0;
	dts.ts_cmd.c_hiba = 0;
	dts.ts_cmd.c_size = 512*NBLK;
	dts.ts_cmd.c_cmd = ACK|CVC|WRITE;
	tsaddr->tsdb = dtsinfo;
	if (twait()) printf("Error writing buffer at %x(16).\n", buf);

}

static
tcom(com)
{
	register struct device *tsaddr = (struct device *) dump_addr;

	dts.ts_cmd.c_cmd = ACK|CVC|com;
	dts.ts_cmd.c_loba = 1;
	tsaddr->tsdb = dtsinfo;
	twait();
}

static
twait()
{
	register struct device *tsaddr = (struct device *) dump_addr;
	register i;

	while ((tsaddr->tssr&SSR)==0)
		;
	i = tsaddr->tssr;
	if (i&SC) {
		printf("Tape error, tssr %x(16).\n", i);
		return(-1);
	}
	return(0);
}

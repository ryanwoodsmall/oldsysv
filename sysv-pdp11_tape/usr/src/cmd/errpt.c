static char sccsid[] = "@(#)errpt.c	1.3";
/* Format and interpret the error log file */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#ifdef vax
#include <sys/mba.h>
#endif
#include <sys/map.h>
#include <sys/elog.h>
#include <sys/err.h>
#include <sys/erec.h>
#include <time.h>

#ifdef vax
#define   PNEED   25      /* Space needed for each block device detail report */
#else
#define   PNEED   24
#endif
#define dysize(x)	(((x)%4)? 365: 366)
#define writout()	((mode==PRINT) && (page<=limit))
#define major(x)	(int)((unsigned)(x>>8)&0377)
#define minor(x)	((int)x&0377)
#define araysz(x)	((sizeof(x)/sizeof(x[0]))-2)
#define readrec(x)	(fread((char*)&recrd,\
			(e_hdr.e_len - sizeof(struct errhdr)), 1, x) )
#define recrd		ercd.ebb.block
#define erp		ercd.parity
#define erm		ercd.memory

#define GDRP04	020
#define GDRP05	021
#define GDRP06	022
#define GDRM03	024
#define GDRM80	026
#define GDRM05	027
#define GDRP07	042
#define GTTM03	050		/* (tu16) */
#define GTTM02	010
#define GTTM78	0100

#ifdef vax
#define FORM "%.8X"
#define FORM2 "\t%.8X\n"
#define DRIVE_REG 6
#else
#define FORM "%.6o"
#define FORM2 "%16lo\n"
#define DRIVE_REG 11
#endif
#define DRIVE_TYPE	ercd.ebb.reginf[DRIVE_REG] & 0777
char Nl [1];
#define NUL4	Nl,Nl,Nl,Nl
#define NUL8	Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl
#define NULS	Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl,Nl

#define INAUG "0301080077"	/* start of error logging [mmddhhmmyy] */
#define WDLEN 16
#define MINREC 8
#define MAXREC 74
#define DEVADR ((physadr)(0160000))
#define MAXLEN 66
#define MAXSTR 40
#define NMAJOR 12
#define NMINOR 8
#define PGLEN 60
#define MBAREG 5
#define INT  15

#define MEM  14
#define YES 1
#define NO 0
#define PRINT 1
#define NOPRINT 0
#define DSEC 3	/* 2**DSEC is the number of logical partitions on RP03/4/5/6 */
#define USAGE1 "errpt [-a] [-d devlist] [-s date]"
#define USAGE2 "\n [-e date] [-p n] [-f] [files]"

/* NMAJOR devices of NMINOR possible logical units */
struct sums {
	long	soft;
	long	hard;
	long	totalio;
	long	misc;
	long	missing;
#ifdef vax
	int	contr;
	int	slave;
#endif
	char	*drvname;
} sums[NMAJOR][NMINOR];
struct tb_sums {
	long	totalio;
	long	misc;
	long	missing;
};

/* structure for maintaining totals across multiple error files: */
struct tb_sums tot_sums[NMAJOR][NMINOR];
struct tb_sums base_sums[NMAJOR][NMINOR];

union ercd {
	struct  estart start;
	struct	eend end;
	struct	etimchg timchg;
	struct	econfchg confchg;
	struct	estray stray;
	struct	eparity parity;
	struct	ememory memory;
	struct  eb {
		struct	eblock block;
		unsigned short reginf [30];
	} ebb;
} ercd;

struct errhdr e_hdr;

int dmsize[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int maj;
int min;
int page=1;
int print;
int mode = NOPRINT;
int line;
int n = 0;
int aflg;
int dflg;
int fflg;
int Unix = 1;
short cputype;
int parsum;
int straysum;
int limit = 10000;		/* page limit */
int optdev = 0;			/* bit map for device-specific reports */
long readerr = 0;		/* input record error count */
FILE	*file;
time_t	atime;
time_t	xtime = 0L;
time_t	etime = 017777777777L;
time_t	fftime = 017777777777L;
time_t	ltime = 0L;
char interp [MAXSTR];
char choice [MAXSTR];
char *strcpy();
char *ctime();
long tloc;
long time();
#ifdef vax
struct vaxreg {
	char *regname;
	char *bitcode [WDLEN*2];
};
int mb_dev;	/* True if device is MBA connected */
#endif
struct regs {
	char *regname;
	char *bitcode [WDLEN];
};

struct regs *regs_ptr;
char htime[20];
char *header = "SYSTEM ERROR REPORT";
char *hd1 = "System Error Report - Selected Items";
char *hd2 = "Summary Error Report";

char *dev[] = {
	"RK05","RP03","RF11","TU10",
	"TC11","RP04/5/6","TU16","RS03/4",
	"RL02","TM78","RP05","RP06",
	0
};
/*
 * Register names and bit mnemonics for each
 * register and bit to be interpreted on detail report.
 */
struct regs rkregs [] = {
	"DS",NUL4,Nl,"WPS",Nl,Nl,Nl,"SIN","DRU",NUL4,Nl,
	"ER","WCE","CSE","","","","NXS","NXC","NXD","TE","DLT",
		"NXM","PGE","SKE","WLO","OVR","DRE",
	"CS",NULS,
	"WC",NULS,
	"BA",NULS,
	"DA",NULS,
	0
};
struct regs rk2regs [] = {
	"CS1",NULS,
	"WC",NULS,
	"BA",NULS,
	"DA",NULS,
	"CS2",NULS,
	"DS",NULS,
	"ER",NULS,
	"ASOF",NULS,
	"DC",NULS,
	"Z   ",NULS,
	"DB",NULS,
	"MR1",NULS,
	"ECPS",NULS,
	"ECPT",NULS,
	"MR2",NULS,
	"MR3",NULS,
	0
};
struct regs rpregs[] = {
	"DS",NUL8,Nl,"SUFU",Nl,"SUSI","HNF",Nl,Nl,Nl,
	"ER","DSKERR","EOP","NXME","WCE","TIMEE","CSME",
		"WPE","LPE","MODE","FMTE","PROG","NXS","NXT","NXC","FUV","WPV",
	"CS",NULS,
	"WC",NULS,
	"BA",NULS,
	"CA",NULS,
	"DA",NULS,
	0
};
struct regs rfregs[] = {
	"CS",NUL8,Nl,"MXF","WLO","NED","DPE","WCE",Nl,Nl,
	"WC",NULS,
	"BA",NULS,
	"DA",NULS,
	"DAE",NUL4,Nl,"DAOVFL",Nl,"DRL",Nl,Nl,"NEM",
		"CTER","BTER","ATER","APE",Nl,
	0
};
struct regs tmregs [] = {
	"ER",Nl,Nl,"WRL",NUL4,"NXM","BTE","RLE","EOT",
		"BGL","PAE","CRE",Nl,"ILL",
	"CS",NULS,
	"BC",NULS,
	"BA",NULS,
	"Z   ",NULS,
	"Z   ",NULS,
	0
};
struct regs tcregs [] = {
	"CSR",NUL8,"NEX","DATM","BLKM","SELE","ILO","MTE","PAR",Nl,
	"CM",NULS,
	"WC",NULS,
	"BA",NULS,
	"Z   ",NULS,
	0
};
#ifdef vax
struct regs hpregs[] = {
	"CS1","GO","F0","F1","F2","F3","F4",Nl,Nl,NUL8,
	"DS",NUL4,Nl,Nl,"VV","DRY","DPR",Nl,Nl,Nl,"MOL","PIP","ERR","ATA",
	"ER1","ILF","ILR","RMR","PAR","FER","WCF","ECH","HCE",
		"HCRC","AOE","IAE","WLE","DTE","OPI","UNS","DCK",
	"MR",NULS,
	"AS","ATA0","ATA1","ATA2","ATA3","ATA4","ATA5","ATA6","ATA7",NUL8,
	"DA",NULS,
	"DT",NULS,
	"LA",NULS,
	"Z   ",NULS,
	"OF",NULS,
	"DC",NULS,
	"CC",NULS,
	"ER2",NULS,
	"ER3",NULS,
	"EC1",NULS,
	"EC2",NULS,
	0
};
struct regs hp2regs[] = {		/* RM02/3/5/80 Registers */
	"CS1","GO","F0","F1","F2","F3","F4",Nl,Nl,NUL8,
	"DS",NUL4,Nl,Nl,"VV","DRY","DPR",Nl,Nl,Nl,"MOL","PIP","ERR","ATA",
	"ER1","ILF","ILR","RMR","PAR","FER","WCF","ECH","HCE",
		"HCRC","AOE","IAE","WLE","DTE","OPI","UNS","DCK",
	"MR1",NULS,
	"AS","ATA0","ATA1","ATA2","ATA3","ATA4","ATA5","ATA6","ATA7",NUL8,
	"DA",NULS,
	"DT",NULS,
	"LA",NULS,
	"Z   ",NULS,
	"OF",NULS,
	"DC",NULS,
	"HR",NULS,
	"MR2",NULS,
	"ER2",NULS,
	"EC1",NULS,
	"EC2",NULS,
	0
};
struct regs htregs[] = {
	"CS1","GO","F0","F1","F2","F3","F4",NUL4,Nl,"DVA",NUL4,
	"DS","SLA","BOT","TM","IDB","SDWN","PES","SSC","DRY",
		"DPR",Nl,"EOT","WRL","MOL","PIP","ERR","ATA",
	"ER1","ILF","ILR","RMR","CPAR","FMT","DPAR","INC/VPE",
		"PER/LRC","NSG","FCE","CS/ITM","NEF","DTE","OPI","UNS",
		"COR/CRC",
	"MR",NULS,
	"AS","ATA0","ATA1","ATA2","ATA3","ATA4","ATA5","ATA6","ATA7",NUL8,
	"FC",NULS,
	"DT",NUL8,Nl,Nl,"SPR",NUL4,Nl,
	"CK",NULS,
	"SN",NULS,
	"TC","SS0","SS1","SS2","EVPAR","FMT0","FMT1","FMT2",
		"FMT3","DEN0","DEN1","DEN2",Nl,"EAODTE","TCW","FCS","ACCL",
	0
};

struct regs ht2regs[] = {	/* TM78 registers: */
	"CSR","GO",NUL8,Nl,Nl,"DVA",NUL4,
	"DTC",NUL8,"DPR",NUL4,Nl,Nl,Nl,
	"FM",NUL8,NUL4,Nl,Nl,Nl,"SER",
	"MR1",NULS,
	"AS",NULS,
	"FC",NULS,
	"DT",NUL8,Nl,"WCS",Nl,"DMB",Nl,Nl,"TAP","NSA",
	"DS",NUL4,"DSE",Nl,"SHR","AVL","FPT","EOT","BOT","PE","REW",
		"ONL","PRES","RDY",
	"SN",NULS,
	"MR2",NULS,
	"MR3",NULS,
	"NDC",NULS,
	"NDT0",NULS,
	"NDT1",NULS,
	"NDT2",NULS,
	"NDT3",NULS,
	0
};
struct vaxreg mbareg[] = {
	"MBACSR",NULS,NUL4,Nl,"OT","PU","PD",Nl,Nl,
		"XMTFLT","MT",Nl,"URD","WS","SBIPE",
	"MBACR","INIT","ABRT","IE","MM",NUL8,NUL4,NULS,
	"MBASR","RDTO","ISTO","RDS","ERCON","INVMAP","MAPPE","MDPE",
		"MBEXC","MXF","WCLE","WCUE","DLT","DTABT","DTCMP",Nl,
		Nl,"ATTN","MCPE","NFD","PGE",NUL8,Nl,"CRD","NRC","DTB",
	"MBAVAR",NULS,NULS,
	"MBABCR",NULS,NULS,
	0
};

struct vaxreg mem780[] = {
	"SBIER", Nl, "NBSY", "MLTERR", "IBERR", "IBTO0", "IBTO1", "IBTOS",
		"IBRDS", "CPERR", Nl, "CPTOS0", "CPTOS1", "CPTO", "RDS",
		"CRD", "RDSEN", NULS,
	"MCRC", NULS, Nl, Nl, Nl, Nl, Nl, Nl, Nl, Nl, Nl, Nl, Nl, Nl,
		"ELOGRQ", "HERR", "INHCRD", Nl,
	0
};
struct vaxreg mem750[] = {
	"CSR0", NULS, Nl, Nl, Nl, Nl, Nl, Nl, Nl, Nl, Nl, Nl, Nl, Nl, Nl,
		"CRD", "INL", "UERRF",
	0
};
struct regs tsregs[] = {	/* ts11 tape drive */
	"Z   ",NULS,
	"Z   ",NULS,
	"RBPCR",NULS,
	"XS0","EOT","BOT","WLK",Nl,Nl,"IE","ONL",Nl,"ILA",
		"ILC",Nl,"WLE","RLL","LET","RLS","TMK",
	"XS1","MTE","UNC",NUL4,Nl,Nl,Nl,"SCK",Nl,"TIG",Nl,Nl,Nl,"DLT",
	"XS2",NUL8,"DTP",Nl,"WCF",Nl,Nl,"BPE","SIP","OPM",
	"XS3","RIB",NUL4,"REV","OPI",Nl,NUL8,
	"TSSR",NUL4,"FC0","FC1",NUL8,Nl,"SC",
	0
};
#else
struct regs hpregs[] = {
	"CS1",NUL8,NUL4,Nl,"MCPE","TRE",Nl,
	"WC",NULS,
	"BA",NULS,
	"DA",NULS,
	"CS2",NUL8,"MDPE","MXF","PGE","NEM","NED","PE","WCE","DLT",
	"DS",NUL8,Nl,Nl,Nl,"WRL","MOL",Nl,Nl,Nl,
	"ER1","ILF","ILR","RMR","PAR","FER","WCF","ECH","HCE",
		"HCRC","AOE","IAE","WLE","DTE","OPI","UNS","DCK",
	"AS",NULS,
	"Z   ",NULS,
	"DB",NULS,
	"MR",NULS,
	"DT",NULS,
	"Z   ",NULS,
	"OF",NULS,
	"DC",NULS,
	"CC",NULS,
	"ER2",NULS,
	"ER3",NULS,
	"EC1",NULS,
	"EC2",NULS,
	"BAE",NULS,
	"CS3",NUL8,NUL4,Nl,Nl,Nl,"APE",
	0
};
struct regs hp2regs[] = {		/* RM02/3/5/80 Registers */
	"CS1",NUL8,NUL4,Nl,"MCPE","TRE",Nl,
	"WC",NULS,
	"BA",NULS,
	"DA",NULS,
	"CS2",NUL8,"MDPE","MXF","PGE","NEM","NED","PE","WCE","DLT",
	"DS",NUL8,Nl,Nl,Nl,"WRL","MOL",Nl,Nl,Nl,
	"ER1","ILF","ILR","RMR","PAR","FER","WCF","ECH","HCE",
		"HCRC","AOE","IAE","WLE","DTE","OPI","UNS","DCK",
	"AS",NULS,
	"Z   ",NULS,
	"DB",NULS,
	"MR1",NULS,
	"DT",NULS,
	"Z   ",NULS,
	"OF",NULS,
	"DC",NULS,
	"HR",NULS,
	"MR2",NULS,
	"ER2",NULS,
	"EC1",NULS,
	"EC2",NULS,
	"BAE",NULS,
	"CS3",NUL8,NUL4,Nl,Nl,Nl,"APE",
	0
};
struct regs htregs [] = {
	"CS1",NUL8,NUL4,Nl,"MCPE","TRE",Nl,
	"WC",NULS,
	"BA",NULS,
	"FC",NULS,
	"CS2",NUL8,"MDPE","MXF","PGE","NEM","NED","PE","WCE","DLT",
	"DS",Nl,"BOT",NUL8,"EOT","WRL","MOL",Nl,Nl,Nl,
	"ER","ILF","ILR","RMR","CPAR","FMT","DPAR",
		"INC/VPE","PEF/LRC","NSG","FCE","CS/ITM","NEF",
		"DTE","OPI","UNS","COR/CRC",
	"AS",NULS,
	"CK",NULS,
	"DB",NULS,
	"MR",NULS,
	"DT",NULS,
	"Z   ",NULS,
	"TC",NUL8,Nl,"NRZ","PE",NUL4,Nl,
	"BAE",NULS,
	"CS3",NUL8,NUL4,Nl,Nl,Nl,"APE",
	0
};
struct regs ht2regs[] = {
	"CS1","GO",NUL4,Nl,"IE","RDY",Nl,Nl,"PSEL","DVA",Nl,"MCPE","TRE","SC",
	"WC",NULS,
	"BA",NULS,
	"BCR",NULS,
	"CS2",Nl,Nl,Nl,"BAI","PAT","CLR","IR","OR","MDPE","MXF","PGE","NEM",
		"NED","UPE","WCE","DLT",
	"DTC",NUL8,"DPR",NUL4,Nl,Nl,Nl,
	"FM",NULS,
	"AS",NULS,
	"DS",NUL4,"DSE",Nl,"SHR","AVL","FPT","EOT","BOT",Nl,"REW","PRES","RDY",
	"DB",NULS,
	"MR1",NULS,
	"DT",NUL8,Nl,"WCS",Nl,"DMB",Nl,Nl,"TAP","NSA",
	"SN",NULS,
	"MR2",NULS,
	"MR3",NULS,
	"NDC",NULS,
	"NDT0",NULS,
	"NDT1",NULS,
	"NDT2",NULS,
	"NDT3",NULS,
	0
};
#endif
struct regs hsregs[] = {
	"CS1",NUL8,NUL4,Nl,"MCPE","TRE","SC",
	"WC",NULS,
	"BA",NULS,
	"DA",NULS,
	"CS2",NUL8,"MDPE","MXF","PGE","NEM","NED","PE","WCE","DLT",
	"DS",NUL8,Nl,Nl,Nl,"WRL","MOL",Nl,Nl,Nl,
	"ER","ILF","ILR","RMR","PAR",Nl,Nl,Nl,Nl,Nl,"AO",
		"IAE","WLE","DTE","OPI","UNS","DCK",
	"AS",NULS,
	"Z   ",NULS,
	"DB",NULS,
	"MR",NULS,
	"Z   ",NULS,
	"BAE",NULS,
	"CS3",NUL8,NUL4,Nl,Nl,Nl,"APE",
	0
};
struct regs rlregs[] = {
	"CS",NUL8,Nl,Nl,"OPI","CRC","DLT/HNF","NXM","DE",Nl,
	"BA",NULS,
	"DA",NULS,
	"MP",Nl,Nl,Nl,"BH","HO","CO","HS",Nl,"DSE","VC","WGE","SPE",
		"SKTO","WL","CHE","WDE",
	0
};
int rkblk();
int rpblk();
int rfblk();
int tmblk();
int tcblk();
int gdblk();
int hsblk();
int rlblk();

/* Device specification functions */
/* Must remain in same order as true major device numbers */
int (*func[]) () = {
	rkblk,
	rpblk,
	rfblk,
	tmblk,
	tcblk,
	gdblk,
	gdblk,
	hsblk,
	rlblk,
	gdblk,
	gdblk,
	gdblk,
	0
};
struct regs  *devregs[] = {
	rkregs,
	rpregs,
	rfregs,
	tmregs,
	tcregs,
	hpregs,
	htregs,
	hsregs,
	rlregs,
	hpregs,
	hpregs,
	hpregs,
	0
};
struct pos {
	unsigned flg;
	unsigned unit;
#ifdef vax
	unsigned controller;	/* massbus controller number */
	unsigned slave;		/* for multiple tape drives on formater */
#endif
	unsigned cyl;
	unsigned trk;
	unsigned sector;
} pos;
/* Array order directed by MMR3 */
char *msg[] = {
	"User D Space Enabled",
	"Supervisor D Space Enabled",
	"Kernel D Space Enabled",
	Nl,
	"22 bit mapping Enabled",
	"UNIBUS MAP relocation Enabled",
	0
};

char *lines [] = {
/* 0*/	"\n",
/* 1*/	"%s\tError Logged On   %s\n",
/* 2*/	"\tPhysical Device\t\t\t%u\n",
/* 3*/	"\tLogical Device\t\t\t%d (%2.2o)\n",
/* 4*/	"\tDevice Address\t\t\t",
/* 5*/	"\tRetry Count\t\t\t%u\n",
/* 6*/	"\tError Diagnosis\t\t\t%s\n",
/* 7*/	"\tSimultaneous Bus Activity\t",
/* 8*/	"\tRegisters at Error time\n",
/* 9*/	"\t\t%s\t",
/*10*/	"\tPhysical Buffer Start Address\t",
/*11*/	"\tTransfer Size in Bytes\t\t%16u\n",
/*12*/	"\tType of Transfer\t\t\t%8s\n",
/*13*/	"\tBlock No. in Logical File System\t%8ld\n",
/*14*/	"\tI/O Type\t\t\t\t%8s\n",
/*15*/	"\tCylinder\t\t\t\t%8u\n",
/*16*/	"\tTrack\t\t\t\t\t%8u\n",
/*17*/	"\tSector\t\t\t\t\t%8u\n",
/*18*/	"\tStatistics on Device to date:\n",
/*19*/	"\t\tR/W Operations\t\t%16ld\n",
/*20*/	"\t\tOther Operations\t%16ld\n",
/*21*/	"\t\tUnrecorded Errors\t%16u\n",
/*9b*/	"%o      ",
/*23*/	"\tSector Requested\t\t%u\n",
/*24*/  "\tUnibus Map Utilization?\t\t\t%3.3s\n",
#ifdef vax
/*2a*/	"\tController\t\t\t%d\n",
/*2b*/	"\tPhysical Device\t\t\t%u\t\tSlave\t%u\n",
#endif
	0
};

char *xlines [] = {
/* 0*/  "\n\nDEVICE CONFIGURATION CHANGE   - %s\n",
/* 1*/	"\tDEVICE: %s - %s\n",
/* 2*/	"\n\nSTRAY INTERRUPT on %s\n",
/* 3*/  "\tFor Controller at - ",
/* 4*/	"\tAt Location\t",
#ifdef pdp11
/* 5*/	"\n\nMEMORY PARITY ERROR at %s\n",
/* 6*/	"\tMemory Address of Error - %lo (cycle %d)\n\n",
/* 7*/	"\t\tMSER\t%.6o",
/* 8*/	"\t\tMSCR\t%.6o",
#else
/* 5*/	"\n\nMEMORY ECC on %s\n",
/* 6*/	"\tError Address (%s)\t%.5X\n",
/* 7*/	"\tError Syndrome\t\t\t%.2X\n",
/* 8*/	0,
#endif
/* 9*/	"\n\nTIME CHANGE ***** FROM %s",
/*10*/  "\t\t   TO  %s \n\n\n\n",
/*11*/  "\nERROR LOGGING SYSTEM SHUTDOWN - %s\n\n\n",
/*12*/	"\nERROR LOGGING SYSTEM STARTED - %s \n",
/*13*/	"\n\n\tSystem Profile:\n\n",
/*14*/	"\t     11/%d  Processor\n",
/*15*/	"\t     System Memory Size - %ld Bytes\n",
/*16*/	Nl,
/*17*/	"\t     UNIX/%s  Operating System (%s)\n",
/*18*/  "\t     %s \n",
	0
};

char *sumlines [] = {
/* 0*/	"\n\n",
#ifdef vax
/* 1*/	"%s\tUnit  %d\t Controller %d\n",
#else
/* 1*/	"%s\tUnit  %d\n",
#endif
/* 2*/	"\tHard Errors\t\t- %10ld\n",
/* 3*/	"\tSoft Errors\t\t- %10ld\n",
/* 4*/	"\tTotal I/O Operations    - %10ld\n",
/* 5*/	"\tTotal Misc. Operations  - %10ld\n",
/* 6*/	"\tErrors Missed\t\t- %10ld\n",
/* 7*/	"\tTotal Read Errors\t\t-   %ld\n",
/* 8*/	"\tTotal Memory Parity Errors\t-   %d\n",
/* 9*/	"\tTotal Stray Interrupts\t\t-   %d\n",
/*10*/	"\tDate of Earliest Entry: %s",
/*11*/	"\tDate of Latest   Entry: %s",
/*12*/  "\tError Types: %s\n",
/*13*/  "\tLimitations: ",
/*14*/  "\t\t",
#ifdef vax
/*15*/	"%s\tUnit  %d\t Slave  %d\t Controller %d\n",
#endif
0
};


/* Correspondence of input requests to major device defines */
struct tab {
	char *devname ;
	int devnum;
};
struct tab dtab[] = {
	"rk",RK0,	"rk05",RK0,	"tc",TC0,	"rk07",RK0,	"rm80",HP0,
	"rp",RP0,	"rp03",RP0,	"rf",RF0,	"rf11",RF0,
	"tm",TM0,	"tu",TM0,	"mt",TM0,	"tu10",TM0,	"ts11",TM0,
	"tc11",TC0,	"hp",HP0,	"rp04",HP0,	"rp05",HP0,
	"rp07",HP0,	"rm05",HP0,	"rp06",HP0,	"ht",HT0,	"te16",HT0,
	"tm78",HT0,	"tu78",HT0,	"tu16",HT0,	"hs",HS0,
	"rs03",HS0,	"rs04",HS0,	"rs",HS0,	"rl",RL0,
	"rl01",RL0,	"mem",MEM,	"int",INT,	0,0
};

main (argc,argv)
char *argv[];
int argc;
{
	register i,j;
	print = NO;
	while (--argc>0 && **++argv =='-') {
		switch (*++*argv) {
		case 's':	/* starting at a specified time */
			header = hd1;
			if((--argc <=0) || (**++argv == '-'))
			error("Date required for -s option",(char *)NULL);
			if(gtime(&xtime,*argv))
				error("Invalid Start time",*argv);
			break;
		case 'e':	/* ending at a specified time */
			header = hd1;
			if((--argc<=0) || (**++argv =='-'))
				error("Date required for -e option\n",(char *)NULL);
			if(gtime(&etime,*argv))
				error("Invalid End time.",(char *)NULL);
			break;
		case 'a':	/* print all devices*/
			aflg++;
			mode = PRINT;
			break;
		case 'p':	/* limit total no. of pages */
			if((--argc<=0) || (**++argv == '-'))
				error("Page limit not supplied.\n",(char *)NULL);
			limit = atoi(*argv);
			break;
		case 'f':	/* fatal errors */
			header = hd1;
			fflg++;
			break;

		default:
			if(j=encode(*argv)) {
				optdev = (optdev |= j);
				dflg++;
				header = hd1;
				mode = PRINT;
				if(strlen(choice)) concat(",",choice);
				concat(*argv,choice);
				}
			else
			(void) fprintf(stderr,"%s?\n",argv);
		}
	}
	for(i = 0;i < NMAJOR;i++) {
		for(j = 0; j < NMINOR; j++) {
			sums[i][j].hard = 0;
			sums[i][j].soft = 0;
			sums[i][j].totalio = 0;
			sums[i][j].misc = 0;
			sums[i][j].missing = 0;
#ifdef vax
			sums[i][j].contr = 0;
			sums[i][j].slave = 0;
#endif
			tot_sums[i][j].totalio = 0;
			tot_sums[i][j].misc = 0;
			tot_sums[i][j].missing = 0;
			base_sums[i][j].totalio = 0;
			base_sums[i][j].misc = 0;
			base_sums[i][j].missing = 0;
		}
	}

	parsum=0;
	straysum=0;
	if(gtime(&atime,INAUG)) error("Invalid INAUG time",INAUG);
	if (argc ==0)
		report("/usr/adm/errfile");
	else while(argc--) 
		report(*argv++);
	printsum();
	putft();
	exit(0);
}

/* Associate typed name with a specific bit in "optdev" */
encode(p)
char  *p;
{
	register struct tab *q;
	int lower();

	lower (p); /* convert device name to lower case */
	for(q=dtab;q->devname;q++) {
		if (!strcmp(q->devname,p)) {
			if (q->devnum != HP0)
				return(1<<(q->devnum));
			else
				return (1<<HP0 | 1<<HP1 | 1<<HP2 | 1<<HP3);
		}
	}
	return(0);
}

report(fp)
char *fp;
{
	register int i, j;

	if ((file = fopen(fp, "r")) == NULL)
		error("cannot open", fp);
	inithdng();
	if (writout())
		puthead(header);
	putdata();
	if (writout())
		putft();
	for(i = 0; i < NMAJOR; i++)
		for(j = 0; j < NMINOR; j++)
			if(sums[i][j].totalio)
				adjustsum(i, j);
}
putdata()
{
	while(fread((char*)&e_hdr.e_type, sizeof(struct errhdr), 1, file)) {
newtry:
		switch(e_hdr.e_type) {
		
		case E_GOTS:
			setime();
			up();
			break;

		case E_GORT:
			setime();
			up();
			break;

		case E_STOP:
			setime();
			down();
			break;

		case E_TCHG:
			setime();
			timecg();
			break;

		case E_BLK:
			setime();
			blk();
			break;
		
		case E_STRAY:
			setime();
			stray();
			break;

		case E_PRTY:
			setime();
			party();
			break;

		case E_CCHG:
			cconfig();
			setime();
			break;
		default:
			fprintf(stderr, "%d\n", e_hdr.e_len);
			fprintf(stderr, "%d\n", e_hdr.e_type);
			readerr++;
			if (recov())
				goto newtry;
			fprintf (stderr, "Unrecovered read error.\n");
		}
	}

}
/* System Startup Record */

#define est	ercd.start
up()
{
register int i;

	if (!readrec(file)) {
		fprintf(stderr, "at up = %o\n", est.e_cpu);
		fprintf(stderr, "%o\n", est.e_mmr3);
		fprintf(stderr, "%ld\n", est.e_name.release);
		fprintf(stderr,
			"%s %s\n", est.e_name.release, est.e_name.sysname);
		readerr++;
		return;
	}
	cputype = est.e_cpu;
	if (writout()) {
		need(13+araysz(msg));
		printf(xlines[12], ctime(&e_hdr.e_time));
		printf(xlines[13]);
		printf(xlines[17], est.e_name.release, est.e_name.sysname);
		printf(xlines[14], est.e_cpu);
		if (est.e_syssize)
			printf(xlines[15], est.e_syssize);
		else
			line--;
		for(i = 0; i <= araysz(msg) + 1; i++)
			if (est.e_mmr3 & (1 << i))
				printf(xlines[18], msg[i]);
			else
				line--;
		printf(lines[0]);
	}
}
/* System Shutdown Record */

down()
{
	if (writout()) {
		need(5);
		printf(xlines[11], ctime(&e_hdr.e_time));
	}
}
/* Time Change Record */

timecg()
{
	if (!readrec(file)) {
		readerr++;
		return;
	}
	if (writout()) {
		need(8);
		printf(xlines[9], ctime(&e_hdr.e_time));
		printf(xlines[10], ctime(&ercd.timchg.e_ntime));
	}
}
/* Handle a MERT configuration change */
cconfig() 
{

	if (!readrec(file)) {
		readerr++;
		return;
	}
	if (writout()) {
		need(7);
		printf(xlines[0], ctime(&e_hdr.e_time));
		printf(lines[0]);
		printf(xlines[1], dev[ercd.confchg.e_trudev],
			ercd.confchg.e_cflag ? "Attached" : "Detached");
		printf(lines[0]);
	}
}

/* Stray Interrupt Record */

#define estr	ercd.stray
stray()
{
	if (!readrec(file)) {
		readerr++;
		return;
	}
	if (!wanted())
		return;
	if (print == YES) {
		need(6);
		if (page <= limit) {
			printf(xlines[2], ctime(&e_hdr.e_time));
			if (estr.e_saddr < DEVADR)
				printf(xlines[4]);
			else
				printf(xlines[3]);
			printf(FORM, estr.e_saddr);
			printf(lines[0]);
			printf(lines[7]);
			if (estr.e_sbacty == 0)
				printf("None\n");
			else
				afix(araysz(dev)+1,
					(unsigned) estr.e_sbacty, dev);
		}
	}
	straysum++;
}

/* Memory Parity Record */
party()
{
	register struct vaxreg *q;
	char *ctime();

	if (!readrec(file)) {
		readerr++;
		return;
	}
	if (!wanted())
		return;
	if (print == YES) {
#ifdef pdp11
		need(9);
#else
		need(11);
#endif
		if (page > limit)
			return;
		printf(xlines[5], ctime(&e_hdr.e_time));
#ifdef pdp11
		printf(xlines[6],(((long)(erp.e_parreg[1]&077))<<WDLEN) +
			((long)((unsigned) erp.e_parreg[0])),
			(erp.e_parreg[1]>>14)&03);
		printf(lines[0]);
		printf(lines[8]);
		printf(xlines[7], erp.e_parreg[2]);
		printf(lines[0]);
		printf(xlines[8], erp.e_parreg[3]);
#else
		printf(lines[0]);
		if (cputype == 780) {
			printf(xlines[6], "Longword",
				(erm.e_memcad & 0X0fffff00) >> 6);
			printf(xlines[7], erm.e_memcad & 0x000000ff);
			printf(lines[0]);
			printf(lines[8]);
			q = mem780;
			printf("\t  %-4.4s\t  ", q->regname);
			printf(FORM, erm.e_sbier);
			afix(WDLEN*2, erm.e_sbier, q->bitcode);
			printf("\t  %-4.4s\t  ", (++q)->regname);
			printf(FORM, erm.e_memcad);
			afix(WDLEN*2, erm.e_memcad, q->bitcode);
		} else {
			--line;
			printf(xlines[6], "within page", erm.e_memcad);
			printf(xlines[7], erm.e_sbier & 0x0000007f);
			printf(lines[0]);
			printf(lines[8]);
			q = mem750;
			printf("\t  %-4.4s\t  ", q->regname);
			printf(FORM, erm.e_sbier);
			afix(WDLEN*2, erm.e_sbier, q->bitcode);
		}
#endif
	}
	parsum++;
}

/* Device Error Record */

blk()
{
	register union ercd *z;
	register int i;
	register struct sums *p;
	register struct eblock *r;
	int *mbar;
	struct vaxreg *q;
	int ldev;

#ifdef vax
	pos.controller = pos.slave = pos.flg = 0;
#else
	pos.flg = 0;
#endif
	if (!readrec(file)) {
		readerr++;
		return;
	}
	z = &ercd;
	maj = major(recrd.e_dev);
	ldev = min = minor(recrd.e_dev);
	if ((maj > araysz(func)) | maj < 0)
		return;
	if (!wanted())
		return;
	regs_ptr = devregs[maj];

	(*func[maj])();

	/* Increment summary totals */
	
	p = &sums[maj][min];
	r = &recrd;
	if (r->e_bflags & E_ERROR)
		p->hard++;
	else
		p->soft++;
	if (r->e_stats.io_ops < p->totalio)
		adjustsum(maj, min);

	if (!(p->drvname))
		p->drvname = dev[maj];
#ifdef vax
	p->contr = pos.controller;
	p->slave = pos.slave;
#endif
	p->totalio = r->e_stats.io_ops;
	p->misc = r->e_stats.io_misc;
	p->missing = r->e_stats.io_unlog;

	if (print == NO)
		return;

#ifdef vax
	mb_dev = (maj == HP0 || maj == HT0 || maj >= HP1);
	need(PNEED + r->e_nreg + mb_dev * 5 + pos.flg * 5);
#else
	need(PNEED + r->e_nreg + pos.flg * 5);
#endif
	if (page > limit)
		return;
	printf(lines[0]);
	printf(lines[1], p->drvname, ctime(&e_hdr.e_time));
#ifdef pdp11
	printf(lines[2], pos.unit);
#else
	if (pos.slave)
		printf(lines[26], pos.unit, pos.slave);
	else
		printf(lines[2], pos.unit);
	printf(lines[25], p->contr);
#endif
	printf(lines[3], ldev, ldev);
	printf(lines[4]);
	printf(FORM, r->e_regloc);
	printf(lines[0]);
	printf(lines[5], r->e_rtry);
	printf(lines[6], r->e_bflags & E_ERROR ?
		"Unrecovered" : "Recovered");
	printf(lines[7]);
	if (r->e_bacty == 0)
		printf("None\n");
	else
		afix(WDLEN, (unsigned) r->e_bacty, dev);
	printf(lines[0]);
	printf(lines[8]);
#ifdef vax
	if (mb_dev) {
		mbar = ((int *) &z->ebb.block.e_mba);
		q = mbareg;
		for(i = 0; i < MBAREG; i++, mbar++, q++) {
			printf("\t  %-6.6s   %.8X", q->regname, *mbar);
			afix(WDLEN*2, *mbar, q->bitcode);
		}
	}
#endif
	for(i = 0; i < r->e_nreg; i++, regs_ptr++) {
		if (*regs_ptr->regname != 'Z') {
			printf("\t  %-4.4s\t   ", regs_ptr->regname);
			printf("%8.6o", ercd.ebb.reginf[i]);
			afix(WDLEN, ercd.ebb.reginf[i], regs_ptr->bitcode);
		} else
			line--;	/* adjust page-position counter */
	}
	printf(lines[0]);
	printf(lines[10]);
	printf(FORM2, r->e_memadd);
	printf(lines[11], r->e_bytes);
	i = r->e_bflags;
	printf(lines[12],
		(i & E_NOIO) ? "No-op" : ((i & E_READ) ? "Read" : "Write"));
	printf(lines[13], r->e_bnum);
	if (Unix)
		printf(lines[14], i & E_PHYS ? "Physical" : "Buffered");
	else
		line--;
	printf(lines[0]);
	if (pos.flg) {
		printf(lines[0]);
		if (maj != HS0)
			printf(lines[15], pos.cyl);
		else line--;
		printf(lines[16], pos.trk);
		if (maj == HS0)
			printf(lines[23], pos.sector, pos.sector+n);
		else
			printf(lines[17], pos.sector);
		printf(lines[0]);
	}
	printf(lines[0]);
	printf(lines[18]);
	printf(lines[19], tot_sums[maj][min].totalio+r->e_stats.io_ops);
	printf(lines[20], tot_sums[maj][min].misc+r->e_stats.io_misc);
	printf(lines[21], tot_sums[maj][min].missing+r->e_stats.io_unlog);
	printf(lines[0]);
}
rkblk()
{
	register int m;
	register daddr_t d;
	register int track, sect;

	pos.flg = 1;
	m = min - 7;
	if (m <= 0) {
		d = recrd.e_bnum;
		pos.unit = min;
	}
	else {
		d = recrd.e_bnum / m;
		pos.unit =  recrd.e_bnum % m;
	}
	if (recrd.e_nreg > 8) {
		sums[maj][pos.unit].drvname = (ercd.ebb.reginf[0] & 01000) ?
			"RK06" : "RK07";
		regs_ptr = rk2regs;
		track = 3; sect = 22;
	} else {
		track = 2; sect = 12;
	}
	min = pos.unit;
	pos.cyl = d / (track * sect);
	pos.trk = d % (track * sect) / sect;
	pos.sector = d % (track * sect) % sect;
}
rpblk()
{

	pos.flg = 1;
	min = pos.unit = min>>DSEC;
	pos.cyl = recrd.e_cyloff;
	pos.cyl += recrd.e_bnum / (20 * 10);
	pos.trk = (recrd.e_bnum % (20 * 10)) / 10;
	pos.sector = (recrd.e_bnum % (20 * 10)) % 10;
}

rfblk()
{
	pos.flg = 1;
	min = pos.unit = recrd.e_bnum >> 10;
	pos.trk = (recrd.e_bnum % 1024) >> 3;
	pos.cyl = 0;
	pos.sector = (recrd.e_bnum % 1024) % 8;
}
tmblk()
{
	min = pos.unit = min & 03;
#ifdef vax
	if (recrd.e_nreg > 6) {
		regs_ptr = tsregs;
		sums[maj][pos.unit].drvname = "TS11";
	}
	else
		sums[maj][pos.unit].drvname = "TU10";
#endif
}
tcblk()
{
	min = pos.unit = min & 07;
}
gdblk()
{
	register int track, sect, dt;
	register struct eblock *r;
	char *findname();

	dt = DRIVE_TYPE;
	if ( (dt & 0770) == GTTM03 || (dt  & 0770) == GTTM78 || maj == HT0) {
		pos.unit = (min >> 1) & 01;
#ifdef vax
		pos.slave = min & 01;
#endif
		min &= 03;
		regs_ptr = ((dt & 0770) == GTTM78) ? ht2regs : htregs;
	} else {
		switch (dt) {
		case GDRM03:
			sect = 32;
			track = 19;
			regs_ptr = hp2regs;
			break;
		case GDRM05:
			sect = 32;
			track = 19;
			regs_ptr = hp2regs;
			break;
		case GDRM80:
			sect = 32;
			track = 7;
			regs_ptr = hp2regs;
			break;
		case GDRP07:
			sect = 50;
			track = 32;
			break;
		default:	/* RP04/5/6 */
			sect = 22;
			track = 19;
			break;
		}
		pos.flg = 1;
		min = pos.unit = (min & 070) >> DSEC;
		r = &recrd;
		pos.cyl = r->e_cyloff + r->e_bnum / (track * sect);
		pos.trk = (r->e_bnum % (track * sect)) / sect;
		pos.sector = (r->e_bnum % (track * sect)) % sect;
	}
#ifdef vax
	pos.controller = (min >> 6) & 03;
#endif
	sums[maj][min].drvname = findname();
}
hsblk()
{
	register div;

	if (min & 010) {
		div = 32L;
		n = 1;
	}
	else {
		div = 16L;
		n = 3;
	}
	pos.flg = 1;
	pos.cyl = 0;
	min = pos.unit = min & 07;
	pos.trk = recrd.e_bnum / div;
	pos.sector = (recrd.e_bnum % div) * (n + 1);
}
rlblk()
{
	pos.flg = 1;
	pos.unit = min;
	pos.trk = recrd.e_bnum / (20 * 256);
	pos.cyl = ((recrd.e_bnum) % (20 * 256)) / 20;
	pos.sector = ((recrd.e_bnum) % (20 * 256)) % 20;
	pos.sector *= 2;
}
cleanse(p, q)
	register char *p;
	register int q;
{
	while(q--)
		*p++ = '\0';
}

afix(a, b, c)
int a;
unsigned b;
char **c;
{
	register i;
	cleanse(interp,MAXSTR);
	for(i = 0; i < a; i++)  {
		if ((b & (1<<i)) && (*c[i])) {
			if ((strlen(c[i]) + strlen(interp)) >= MAXSTR) {
				concat(",", interp);
				printf("\t%s\n\t\t\t", interp);
				line++;
				cleanse(interp,MAXSTR);
			}
			else {
				if (*interp)
					concat(",", interp);
			}
			concat(c[i], interp);
		}
	}
	if (*interp)
		printf("\t%s\n", interp);
	else
		putchar('\n');
}
puthead(h)
char *h;
{
	printf("\n\n   %s   Prepared on %s     Page  %d\n\n\n\n",
		h, htime, page);
	line = 6;
}
inithdng()
{

	char *cbuf;
	time(&tloc);
	cbuf = ctime(&tloc);
	cbuf[16] = '\0';
	strcpy(htime, cbuf + 4);
}
putft()
{
	while (line++ < MAXLEN)
		putchar('\n');
	page++;
}
trnpg()
{
	if ( line >= MAXLEN)
		page++;
	else
		putft();
	if (page <= limit)
		puthead(header);
}
need(a)			/* acts like ".ne" command of nroff */
int a;
{
	if ( line > (PGLEN - a))
		trnpg();
	line += a;
}
gtime(tptr, pt)
char *pt;
time_t	*tptr;
{
	register int i;
	register int y, t;
	int d, h, m;
	long nt;

	t = gpair(pt++);
	if (t < 1 || t > 12)
		return(1);
	pt++;
	d = gpair(pt++);
	if (d < 1 || d > 31)
		return (1);
	pt++;
	h = gpair(pt++);
	if (h == 24) {
		h = 0;
		d++;
	}
	pt++;
	m = gpair(pt++);
	if (m < 0 || m > 59)
		return (1);
	pt++;
	y = gpair(pt++);
	if (y < 0) {
		time(&nt);
		y = localtime(&nt)->tm_year;
	}
	*tptr = 0;
	y += 1900;
	for(i = 1970; i < y; i++)
		*tptr += dysize(i);
	/* Leap year */
	if (dysize(y) == 366 && t >= 3)
		*tptr += 1;
	while(--t)
		*tptr += dmsize[t - 1];
	*tptr += d - 1;
	*tptr = (*tptr * 24) + h;
	*tptr = (*tptr * 60) + m;
	*tptr *= 60;
	*tptr += timezone;
	if (localtime(tptr)->tm_isdst)
		 *tptr -= 60 * 60;
	return(0);

}
gpair(pt)
char *pt;
{
	register int c, d;
	register char *cp;

	cp = pt;
	if (*cp == 0)
		return(-1);
	c = (*cp++ - '0') * 10;
	if (c < 0 || c > 100)
		return(-1);
	if (*cp == 0)
		return(-1);
	if ((d = *cp++ - '0') < 0 || d > 9)
		return(-1);
	return (c + d);
}

wanted ()
{
	/* Starting - ending limitations? */
	if (e_hdr.e_time < xtime ) {
		if (e_hdr.e_type != E_BLK)
			return 0;
		(*func[maj])();
		base_sums[maj][min].totalio = recrd.e_stats.io_ops;
		base_sums[maj][min].misc = recrd.e_stats.io_misc;
		base_sums[maj][min].missing = recrd.e_stats.io_unlog;
		return (0);
	}
	if (e_hdr.e_time > etime)
		return (0);
	/* Only fatal error flag? */
	if ((fflg) && (e_hdr.e_type == E_BLK) &&
	    !(recrd.e_bflags & E_ERROR))
		return(0);
	/* Stray interrupts or parity errors to be considered */
	if ((aflg) || ((e_hdr.e_type == E_STRAY) && (optdev & (1 << INT))) ||
		((e_hdr.e_type == E_PRTY) && (optdev & (1 << MEM)))) {
		print = YES;
		return (1);
	}
	/* Device chosen for consideration or printing? */
	if (dflg == 0) {
		print = NO;
		return(1);
		}
	if ((1 << maj) & optdev) {
		print = YES;
		return(1);
		}
	print = NO;
	return(0);
}
error(s1, s2)
char *s1, *s2;
{
	fprintf(stderr, "errpt: %s %s \n", s1, s2);
	exit(16);
}

recov()
{
	struct errhdr *p,*q;
	int i;
	for(;;) {
		p = q = &e_hdr;
		q++;
		for(i = 0; i < ((sizeof(struct errhdr) / 2) - 1); i++)
			*p++ = *q++;
		fread(p, 2, 1, file);
		if (feof(file))
			return(0);
		if (valid())
			return (1);
	}
}
valid()
{
	switch(e_hdr.e_type) {
		default:
			return(0);
		case E_GOTS:
		case E_GORT:
		case E_STOP:
		case E_TCHG:
		case E_BLK:
		case E_STRAY:
		case E_CCHG:
		case E_PRTY:
			if ((e_hdr.e_len < MINREC) ||
				 (e_hdr.e_len > MAXREC) )
				return (0);
			if ((e_hdr.e_time < atime) ||
				 (e_hdr.e_time > tloc))
				return(0);
			return (1);
	}
}
lower(str_ptr)
char *str_ptr;
{
	for(; *str_ptr; str_ptr++)
		*str_ptr = tolower(*str_ptr);
}

char *
findname()
{
	switch (DRIVE_TYPE){
	case GDRP04:
		return "RP04";
	case GDRP05:
		return "RP05";
	case GDRP06:
		return "RP06";
	case GDRP07:
		return "RP07";
	case GDRM03:
		return "RM03";
	case GDRM05:
		return "RM05";
	case GDRM80:
		return "RM80";
	default:
		switch (DRIVE_TYPE & 0770){
		case GTTM78:
			return "TM78";
		case GTTM02:
		case GTTM03:
#ifdef pdp11
			return "TU16";
#else
			return "TE16";
#endif
		default:
			return "UNKNOWN";
		}
	}
}
concat(a, b)
	register char *a,*b;
{
	while (*b) b++;
	while (*b++ = *a++);
}
setime()
{
	if (e_hdr.e_time < fftime)
	fftime = e_hdr.e_time;
	if (e_hdr.e_time > ltime)
	ltime = e_hdr.e_time;
}

adjustsum(i, j)
register int i, j;
{
	tot_sums[i][j].totalio += sums[i][j].totalio;
	tot_sums[i][j].misc += sums[i][j].misc;
	tot_sums[i][j].missing += sums[i][j].missing;
	sums[i][j].totalio = 0;
	sums[i][j].misc = 0;
	sums[i][j].missing = 0;
	if (base_sums[i][j].totalio &&
	    base_sums[i][j].totalio < sums[i][j].totalio) {
		tot_sums[i][j].totalio -= base_sums[i][j].totalio;
		tot_sums[i][j].misc -= base_sums[i][j].misc;
		tot_sums[i][j].missing -= base_sums[i][j].missing;
		base_sums[i][j].totalio = 0;
		base_sums[i][j].misc = 0;
		base_sums[i][j].missing = 0;
	}
}
printsum()
{
	int i;
	header = hd2;
	page = 1;
	puthead(header);
	need(11);
	printf(sumlines[12], choice[0] ? choice : "All");
	printf(sumlines[13]);
	if (xtime) {
		printf("On or after %s", ctime(&xtime));
		printf(sumlines[14]);
	}
	else
		line--;
	if (etime != 017777777777L) {
		printf("On or before %s", ctime(&etime));
		printf(sumlines[14]);
	}
	else
		line--;
	if (fflg) {
		printf("Only fatal errors are printed.\n");
		printf(sumlines[14]);
	}
	else
		line--;
	if (limit != 10000)
		printf("Printing suppressed after page %d.\n", limit);
	else
		line--;
	printf(lines[0]);
	printf(sumlines[10], ctime(&fftime));
	printf(sumlines[11], ctime(&ltime));
	printf(lines[0]);
	if (readerr)
		printf(sumlines[7], readerr);
	else
		printf(lines[0]);
	printf(lines[0]);
	if ((optdev & (1 << INT)) || !(dflg) || (aflg)) {
		need(3);
		printf(lines[0]);
		printf(sumlines[9], straysum);
		printf(lines[0]);
	}
	if ((optdev & (1 << MEM)) || !(dflg) || (aflg)) {
		need(3);
		printf(lines[0]);
		printf(sumlines[8], parsum);
		printf(lines[0]);
	}
	if ((dflg == 0) || aflg)
		for(i = 0; i < NMAJOR; i++)
			prsum(i);
	else
		for(i = 0; i < NMAJOR; i++)
			if (optdev & (1<<i))
				prsum(i);
	if (line == 7)
		printf("No errors for this report\n");
}

prsum(i)
register int i;
{
	register int j;

	for(j = 0; j < NMINOR; j++) {
		if (tot_sums[i][j].totalio || sums[i][j].totalio) {
			need(10);
#ifdef vax
			if (sums[i][j].slave)
				printf(sumlines[15], sums[i][j].drvname,
					(j >> 1) & 01, sums[i][j].slave,
					sums[i][j].contr);
			else
				printf(sumlines[1], sums[i][j].drvname,
					j, sums[i][j].contr);
#else
			printf(sumlines[1], sums[i][j].drvname, j);
#endif
			printf(sumlines[0]);
			printf(sumlines[2], sums[i][j].hard);
			printf(sumlines[3], sums[i][j].soft);
			printf(sumlines[4], tot_sums[i][j].totalio);
			printf(sumlines[5], tot_sums[i][j].misc);
			printf(sumlines[6], tot_sums[i][j].missing);
			printf(sumlines[0]);
		}
	}
}

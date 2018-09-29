/*
 *  Configuration information
 */

#define	DL11_0 1
#define	RP03_0 8
#define	TU10_0 4
#define	MEMORY_0 1
#define	TTY_0 1
#define	ERRLOG_0 1

#define	NBUF	20
#define	NINODE	40
#define	NIBLOCK	(NINODE-4)
#define	NFILE	40
#define	NMOUNT	2
#define	CMAPSIZ	50
#define	SMAPSIZ	50
#define	NCALL	20
#define	NPROC	25
#define	NTEXT	15
#define	NCLIST	20
#define	NSABUF	3
#define	POWER	0
#define	MAXUP	25
#define	NHBUF	64
#define	NPBUF	4
#define	MAUS	0
#define	VPMBSZ	8192
#define	X25LINKS	1
#define	X25BUFS	256
#define	X25BYTES	(16*1024)
#define	MESG	0
#define	MSGMAP	100
#define	MSGMAX	8192
#define	MSGMNB	16384
#define	MSGMNI	10
#define	MSGSSZ	8
#define	MSGTQL	40
#define	MSGSEG	1024
#define	SEMA	0
#define	SEMMAP	10
#define	SEMMNI	10
#define	SEMMNS	60
#define	SEMMNU	30
#define	SEMMSL	25
#define	SEMOPM	10
#define	SEMUME	10
#define	SEMVMX	32767
#define	SEMAEM	16384
#define	STIBSZ	8192
#define	STOBSZ	8192
#define	STIHBUF	(ST_0*4)
#define	STOHBUF	(ST_0*4)
#define	STNPRNT	(ST_0>>2)

#include	"sys/param.h"
#include	"sys/types.h"
#include	"sys/sysmacros.h"
#include	"sys/space.h"
#include	"sys/io.h"
#include	"sys/conf.h"

extern nodev(), nulldev();
extern klopen(), klclose(), klread(), klwrite(), klioctl(); extern struct tty kl_tty[];
extern rpopen(), rpclose(), rpread(), rpwrite(), rpstrategy(), rpprint();
extern tmopen(), tmclose(), tmread(), tmwrite(), tmstrategy(), tmprint();
extern mmread(), mmwrite();
extern syopen(), syread(), sywrite(), syioctl();
extern erropen(), errclose(), errread();

struct bdevsw bdevsw[] = {
/* 0*/	rpopen,	rpclose,	rpstrategy,	rpprint,
/* 1*/	tmopen,	tmclose,	tmstrategy,	tmprint,
};

struct cdevsw cdevsw[] = {
/* 0*/	klopen,	klclose,	klread,	klwrite,	klioctl,	kl_tty,
/* 1*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	0,
/* 2*/	nulldev,	nulldev,	mmread,	mmwrite,	nodev, 	0,
/* 3*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	0,
/* 4*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	0,
/* 5*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	0,
/* 6*/	tmopen,	tmclose,	tmread,	tmwrite,	nodev, 	0,
/* 7*/	rpopen,	rpclose,	rpread,	rpwrite,	nodev, 	0,
/* 8*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	0,
/* 9*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	0,
/*10*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	0,
/*11*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	0,
/*12*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	0,
/*13*/	syopen,	nulldev,	syread,	sywrite,	syioctl,	0,
/*14*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	0,
/*15*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	0,
/*16*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	0,
/*17*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	0,
/*18*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	0,
/*19*/	nodev, 	nodev, 	nodev, 	nodev, 	nodev, 	0,
/*20*/	erropen,	errclose,	errread,	nodev, 	nodev, 	0,
};

int	bdevcnt = 2;
int	cdevcnt = 21;

dev_t	rootdev	{(0<<8)|0};
dev_t	pipedev	{(0<<8)|0};
dev_t	dumpdev	{(1<<8)|0};
dev_t	swapdev	{(0<<8)|0};
daddr_t	swplo	7000;
int	nswap	2500;


int	rp_addr[]
{
	0176710,
};
int	rp_cnt = 8;

int	tm_addr[]
{
	0172520,
};
int	tm_cnt = 4;

int	kl_addr[] = {
	0177560,
};
int	kl_cnt = 1;
struct	tty	kl_tty[1];




int	(*pwr_clr[])() = 
{
	(int (*)())0
};

int	(*dev_init[])() = 
{
	(int (*)())0
};

/*
 *  Configuration information
 */


#include	"config.h"
#include	"sys/param.h"
#include	"sys/types.h"
#include	"sys/sysmacros.h"
#include	"sys/space.h"
#include	"sys/conf.h"
extern nodev(), nulldev();
extern dcopen(), dcclose(), dcread(), dcwrite(), dcstrategy(), dcprint();
extern tcopen(), tcclose(), tcread(), tcwrite(), tcioctl(), tcstrategy(), tcprint();
extern lpopen(), lpclose(), lpwrite(), lpioctl();
extern sioopen(), sioclose(), sioread(), siowrite(), sioioctl(); extern struct tty sio_tty[];
extern sxtopen(), sxtclose(), sxtread(), sxtwrite(), sxtioctl();
extern conopen(), conclose(), conread(), conwrite(), conioctl(); extern struct tty con_tty[];
extern mmread(), mmwrite();
extern syopen(), syread(), sywrite(), syioctl();
extern erropen(), errclose(), errread();

struct bdevsw bdevsw[] = {
/* 0*/	dcopen,	dcclose,	dcstrategy,	dcprint,
/* 1*/	tcopen,	tcclose,	tcstrategy,	tcprint,
};

struct cdevsw cdevsw[] = {
/* 0*/	conopen,	conclose,	conread,	conwrite,	conioctl,	con_tty,
/* 1*/	nulldev,	nulldev,	mmread,	mmwrite,	nodev, 	0,
/* 2*/	dcopen,	dcclose,	dcread,	dcwrite,	nodev, 	0,
/* 3*/	syopen,	nulldev,	syread,	sywrite,	syioctl,	0,
/* 4*/	sioopen,	sioclose,	sioread,	siowrite,	sioioctl,	sio_tty,
/* 5*/	erropen,	errclose,	errread,	nodev, 	nodev, 	0,
/* 6*/	tcopen,	tcclose,	tcread,	tcwrite,	tcioctl,	0,
/* 7*/	lpopen,	lpclose,	nodev, 	lpwrite,	lpioctl,	0,
/* 8*/	sxtopen,	sxtclose,	sxtread,	sxtwrite,	sxtioctl,	0,
};

int	bdevcnt = 2;
int	cdevcnt = 9;

dev_t	rootdev = makedev(0, 8);
dev_t	pipedev = makedev(0, 8);
dev_t	dumpdev = makedev(1, 0);
extern tcdump();
int	(*dump)() = tcdump;
int	dmplo = 0;
dev_t	swapdev = makedev(0, 9);
daddr_t	swplo = 0;
int	nswap = 5040;


int	dc_addr[] = {
	0x0d00000,
};

int	tc_addr[] = {
	0x0d00200,
};

struct	tty	con_tty[1];



int	sio_addr[] = {
	0x0a00000,
};
int	sio_cnt = 8;
struct	tty	sio_tty[8];


int	lp_addr[] = {
	0x0fff800,
};

int	sxt_cnt = 20;

int	(*pwr_clr[])() = 
{
	(int (*)())0
};

int	(*dev_init[])() = 
{
	(int (*)())0
};

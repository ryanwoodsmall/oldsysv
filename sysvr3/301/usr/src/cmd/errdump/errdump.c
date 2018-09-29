/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)errdump:errdump.c	1.3"
/*
 *		Copyright 1984 AT&T
 */
# include "sys/types.h"
# include "sys/param.h"
# include "sys/sys3b.h"
# include "sys/firmware.h"
# include "sys/immu.h"
# include "sys/psw.h"
# include "sys/nvram.h"
# include "sys/errno.h"
# include <stdio.h>
# include <time.h>

extern	int	errno ;

char	*prog_name ;

int
main(argc,argv)
int	argc ;
char	*argv[] ;
{
	struct	nvparams	nvparams ;
	struct	xtra_nvr	xtra_nvr ;
	void			error() ;
	
	prog_name = argv[0] ;
	errno = 0 ;

	if ( argc != 1 )
		error("usage: %s\n",prog_name) ;

	nvparams.addr = (char *)(ONVRAM+XTRA_OFSET) ;
	nvparams.data = (char *)&xtra_nvr ;
	nvparams.cnt = sizeof(xtra_nvr) ;
	sys3b(RNVR,&nvparams,0) ;
	if ( errno != 0 )
		error("not invoked by superuser\n") ;
	dumpnvram(&xtra_nvr) ;
	exit(0) ;
}

void
error(msg,arg)
char	*msg ;
int	arg ;
{
	fprintf(stderr,"%s: ",prog_name) ;
	fprintf(stderr,msg,arg) ;
	exit(1) ;
}

dumpnvram(p)
struct	xtra_nvr	*p ;
{
	int	i ;
	char	buf[80] ;
	static	char	*csr_names[] =
	{
		"i/o fault" ,
		"dma" ,
		"disk" ,
		"uart" ,
		"pir9" ,
		"pir8" ,
		"clock" ,
		"inhibit fault" ,
		"inhibit time" ,
		"unassigned" ,
		"floppy" ,
		"led" ,
		"alignment" ,
		"req reset" ,
		"parity" ,
		"bus timeout" ,
	} ;

	printf("nvram status:\t") ;
	switch(p->nvsanity)
	{
		case NVSANITY :
			printf("sane") ;
			break ;

		case ~NVSANITY :
			printf("incompletely updated") ;
			break ;

		default :
			printf("invalid") ;
			break ;
	}
	printf("\n\n") ;
	printf("csr:\t0x%04x\t",p->systate.csr) ;
	for ( i=15 ; i>=0 ; i-- )
		if ( (p->systate.csr&(1<<i)) != 0 )
			printf("(%s) ",csr_names[i]) ;
	printf("\n") ;
	printf("\n") ;
	printf("psw:\trsvd CSH_F_D QIE CSH_D OE NZVC TE IPL CM PM R I ISC TM FT\n") ;
	printf
	(
		"(hex)\t   %x       %x   %x     %x  %x    %x  %x   %x  %x  %x %x %x   %x  %x  %x\n",
		p->systate.psw.rsvd,
		p->systate.psw.CSH_F_D,
		p->systate.psw.QIE,
		p->systate.psw.CSH_D,
		p->systate.psw.OE,
		p->systate.psw.NZVC,
		p->systate.psw.TE,
		p->systate.psw.IPL,
		p->systate.psw.CM,
		p->systate.psw.PM,
		p->systate.psw.R,
		p->systate.psw.I,
		p->systate.psw.ISC,
		p->systate.psw.TM,
		p->systate.psw.FT
	) ;
	printf("\n") ;
	printf("r3:\t0x%08x\n",p->systate.r3) ;
	printf("r4:\t0x%08x\n",p->systate.r4) ;
	printf("r5:\t0x%08x\n",p->systate.r5) ;
	printf("r6:\t0x%08x\n",p->systate.r6) ;
	printf("r7:\t0x%08x\n",p->systate.r7) ;
	printf("r8:\t0x%08x\n",p->systate.r8) ;
	printf("oap:\t0x%08x\n",p->systate.oap) ;
	printf("opc:\t0x%08x\n",p->systate.opc) ;
	printf("osp:\t0x%08x\n",p->systate.osp) ;
	printf("ofp:\t0x%08x\n",p->systate.ofp) ;
	printf("isp:\t0x%08x\n",p->systate.isp) ;
	printf("pcbp:\t0x%08x\n",p->systate.pcbp) ;
	printf("\n") ;
	printf("fltar:\t0x%08x\n",p->systate.mmufltar) ;
	printf("fltcr:\treqacc\txlevel\tftype\n") ;
	printf("\t0x%x\t0x%x\t0x%x\n",p->systate.mmufltcr.reqacc,p->systate.mmufltcr.xlevel,p->systate.mmufltcr.ftype) ;
	printf("\n") ;
	printf("\tsrama\t\tsramb\n") ;
	for ( i=0 ; i<4 ; i++ )
		printf("[%d]\t0x%08x\t0x%08x\n",i,p->systate.mmusrama[i],p->systate.mmusramb[i].SDTlen) ;
	printf("\n") ;
	printf("\tPanic log\n") ;
	for ( i=0 ; i<NERRLOG ; i++ )
	{
		printf("\n") ;
		printf("[%d]\t",i) ;
		if ( sys3b(S3BKSTR,p->errlog[i].message,buf,sizeof(buf)) == -1 )
			printf("(0x%08x,0x%08x,0x%08x,0x%08x)\n",p->errlog[i].time,p->errlog[i].message,p->errlog[i].param1,p->errlog[i].param2) ;
		else
		{
			printf("%s",ctime(&p->errlog[i].time)) ;
			printf("\t") ;
			eprintf(buf,p->errlog[i].param1,p->errlog[i].param2) ;
			printf("\n") ;
		}
	}
	printf("\n") ;
}

/*
 * Scaled down version of C Library printf.
 * Only %s %u %d %o %x %D are recognized.
 */
eprintf(fmt,x1)
char		*fmt;
unsigned int	x1;
{
	int		c;
	unsigned int	*adx;
	char		*s;

	adx = &x1;
loop:
	while((c = *fmt++) != '%') {
		if(c == '\0')
			return;
		putchar(c) ;
	}
	c = *fmt++;
	switch(c)
	{
		case 'd' :
		case 'D' :
			printf("%d",(int)*adx) ;
			break ;

		case 'o' :
			printf("%o",(int)*adx) ;
			break ;

		case 'u' :
			printf("%u",(int)*adx) ;
			break ;

		case 'x' :
			printf("%x",(int)*adx) ;
			break ;

		case 's' :
		{
			char	buf[80] ;

			if ( sys3b(S3BKSTR,(char *)*adx,buf,80) == -1 )
				printf("(0x%08x)") ;
			else
			{
				s = buf ;
				while(c = *s++)
					putchar(c) ;
			}
			break ;
		}
	}
	adx++;
	goto loop;
}

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:nvram.c	1.6"
/*
 * This file contains code for the crash functions:  nvram, mmu, srams,
 * and stat.
 */

#include "sys/param.h"
#include "a.out.h"
#include "stdio.h"
#include "sys/sysmacros.h"
#include "sys/types.h"
#include "sys/fs/s5dir.h"
#include "sys/utsname.h"
#include "sys/panregs.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/nvram.h"
#include "time.h"
#include "sys/sys3b.h"
#include "crash.h"

struct fw_nvr fwnvram;		/* buffer for firmware nvram */
struct unx_nvr unxnvram;	/* buffer for unix nvram */
struct xtra_nvr xtranvram;	/* buffer for extra nvram */
extern int active;		/* active system flag */
extern long conrptr;		/* configuration register pointer */
extern char *sramapt[4];
extern SRAMB srambpt[4];
static struct syment *Sys, *Time, *Lbolt;	/* namelist symbol */
extern struct syment *Panic;				/* pointers */
extern char *ctime();
char *asctime();
struct tm *localtime();

#define FWNVR	0			/* lables for switch */
#define UNXNVR  1
#define SYSTATE 2
#define ERRLOG  3
#define	EBSZ	100			/* error buffer size */

/* get arguments for nvram function */
int
getnvram()
{
	int c;

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		if(!strcmp(args[optind],"fwnvr"))
			prnvram(FWNVR);
		else if(!strcmp(args[optind],"unxnvr"))
			prnvram(UNXNVR);
		else if(!strcmp(args[optind],"systate"))
			prnvram(SYSTATE);
		else if(!strcmp(args[optind],"errlog"))
			prnvram(ERRLOG);
		else longjmp(syn,0);
	}
	else longjmp(syn,0);
}


/* print nvram information */
int
prnvram(section)
int section;
{
	int  indx, i;

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

	/* figure out which section is desired */
	switch (section)
	{
		case FWNVR:	/* firmware */
			fprintf(fp,"FIRMWARE NVRAM\n");
			fprintf(fp,"   passwd: %s\n", fwnvram.passwd);
			fprintf(fp,"   console: %2x\n", fwnvram.cons_def);
			fprintf(fp,"   link: 0x%x\n", fwnvram.link);
			fprintf(fp,"   boot dev: %2x\n", fwnvram.b_dev);
			fprintf(fp,"   boot name: %-45s\n", fwnvram.b_name);
			fprintf(fp,"   dsk_chk: %x\n",fwnvram.dsk_chk);
			break;

		case UNXNVR:	/* unix */
			fprintf(fp,"UNIX NVRAM\n");
			fprintf(fp,"   consflg: 0x%x\n",unxnvram.consflg);
			fprintf(fp,"   month: %d\n", unxnvram.nv_month);
			fprintf(fp,"   year:  %d\n", unxnvram.nv_year);
			fprintf(fp,"   spmem: 0x%x\n",unxnvram.spmem);
			fprintf(fp,"   sys_name:  %s\n",unxnvram.sys_name);
			fprintf(fp,"   rootdev:   ");
			if (unxnvram.rootdev == 1) 
				fprintf(fp,"idisk\n");
			else	fprintf(fp,"ifloppy\n");
			fprintf(fp,"   ioslotinfo:\n");
			for( i = 0; i < 12; i++) 
				fprintf(fp,"%8x %s",unxnvram.ioslotinfo[i],
					(i%4) == 3 ? "\n" : "");
			break;

		case SYSTATE:  /* extra, system state */
			fprintf(fp,"XTRA NVRAM: SYSTEM STATE\n");
			fprintf(fp,"sanity:\t");
			switch   (xtranvram.nvsanity)
			{
				case NVSANITY:
					fprintf(fp,"sane\n");
					break;

				case ~NVSANITY:
					fprintf(fp,"incompletely updated\n");
					break;

				default:
					fprintf(fp,"invalid\n");
					break;
			}

			fprintf(fp,"csr:\t%08x\t",xtranvram.systate.csr) ;
			for ( indx=15 ; indx>=0 ; indx-- )
				if ( (xtranvram.systate.csr&(1<<indx)) != 0 )
					fprintf(fp,"%s, ",csr_names[indx]) ;
			fprintf(fp,"\n");
			fprintf(fp,"psw fields (hex):\n");
			fprintf(fp,"CSH_F_D QIE CSH_D OE NZVC TE IPL CM PM R I ISC TM FT\n");
			fprintf(fp,"   %x     %x    %x    %x   %x   %x  %x   %x  %x %x %x  %x   %x  %x\n",
				xtranvram.systate.psw.CSH_F_D,
				xtranvram.systate.psw.QIE,
				xtranvram.systate.psw.CSH_D,
				xtranvram.systate.psw.OE,
				xtranvram.systate.psw.NZVC,
				xtranvram.systate.psw.TE,
				xtranvram.systate.psw.IPL,
				xtranvram.systate.psw.CM,
				xtranvram.systate.psw.PM,
				xtranvram.systate.psw.R,
				xtranvram.systate.psw.I,
				xtranvram.systate.psw.ISC,
				xtranvram.systate.psw.TM,
				xtranvram.systate.psw.FT
			) ;
			fprintf(fp,"psw:\t%-8x\tlfp:\t%x\n",
				xtranvram.systate.psw,xtranvram.systate.lfp);
			fprintf(fp,"r3:\t%-8x\toap:\t%x\n",
				xtranvram.systate.r3,xtranvram.systate.oap);
			fprintf(fp,"r4:\t%-8x\topc:\t%x\n",
				xtranvram.systate.r4,xtranvram.systate.opc) ;
			fprintf(fp,"r5:\t%-8x\tosp:\t%x\n",
				xtranvram.systate.r5,xtranvram.systate.osp) ;
			fprintf(fp,"r6:\t%-8x\tofp:\t%x\n",
				xtranvram.systate.r6,xtranvram.systate.ofp) ;
			fprintf(fp,"r7:\t%-8x\tisp:\t%x\n",
				xtranvram.systate.r7,xtranvram.systate.isp) ;
			fprintf(fp,"r8:\t%-8x\tpcbp:\t%x\n",
				xtranvram.systate.r8,xtranvram.systate.pcbp) ;
			fprintf(fp,"mmufltcr:  %x\nmmufltar:  %x\n",
				xtranvram.systate.mmufltcr,
				xtranvram.systate.mmufltar);
			fprintf(fp,"mmusrama:");
			fprintf(fp,"  [0]: %-8x   [1]: %-8x   [2]: %-8x   [3]: %x\n",
				xtranvram.systate.mmusrama[0], 
				xtranvram.systate.mmusrama[1], 
				xtranvram.systate.mmusrama[2], 
				xtranvram.systate.mmusrama[3]);
			fprintf(fp,"mmusramb:");
			fprintf(fp,"  [0]: %-8x   [1]: %-8x   [2]: %-8x   [3]: %x\n",
				xtranvram.systate.mmusramb[0], 
				xtranvram.systate.mmusramb[1], 
				xtranvram.systate.mmusramb[2], 
				xtranvram.systate.mmusramb[3]);
			break;

		case ERRLOG:	/* extra, error log */
			fprintf(fp,"XTRA NVRAM: ERROR LOG\n");
			for  ( indx=0; indx < NERRLOG; indx++)
			{
				fprintf(fp,"\nerrlog[%d]:\n", indx);
				errpr(xtranvram.errlog[indx].message,
					xtranvram.errlog[indx].param1,
					xtranvram.errlog[indx].param2,
					xtranvram.errlog[indx].param3,
					xtranvram.errlog[indx].param4,
					xtranvram.errlog[indx].param5,
					xtranvram.errlog[indx].time,0) ;
			}
			break;
	}  /* switch */
}  /* prnvram */


/* error printing */
int
errpr(format,param1,param2,param3,param4,param5,time,stat)
char	*format;
int	param1,
	param2,
	param3,
	param4,
	param5,
	stat;
long	time;
	
{
	int	i ,
		j ,
		paramcnt = 1 ,
		param ;
		
	char	mbuffer[EBSZ] ,
		pbuffer[EBSZ] ;
		
	if (!stat) {
		fprintf(fp,"time: %x message: %x\n",time,format);
		fprintf(fp,"param1: %x param2: %x param3: %x\n",param1,param2,
			param3);
		fprintf(fp,"\tparam4: %x param5: %x\n",param4,param5);
	}
		
	if(format)
		readmem((long)format,1,-1,mbuffer,EBSZ,"mbuffer");
	else mbuffer[0] = '\0';

	if(!stat)
		fprintf(fp,"%s\n",asctime(localtime((long *)&time)));

	for (i = 0; i < EBSZ; i++)	
	{
		if (mbuffer[i] == '\0')
			break ;
			
		if (mbuffer[i] == '%')
		{
			switch (paramcnt++)
			{
				case 1 :
					param = param1 ;
					break ;
				case 2 :
					param = param2 ;
					break ;
				case 3 :
					param = param3 ;
					break ;
				case 4 :
					param = param4 ;
					break ;
				case 5 :
					param = param5 ;
					break ;
				default:
					fprintf(fp,"%%%c",mbuffer[++i]) ;
					continue ;
			}
		
			switch (mbuffer[++i])
			{
				case 's' :
					readmem((long)param,1,-1,pbuffer,
						EBSZ,"pbuffer");
					for(j = 0; j < EBSZ && pbuffer[j] != '\0'; j++)
						fprintf(fp,"%c",pbuffer[j]) ;
					break ;

				case 'D' :
				case 'd' :
					fprintf(fp,"%d", param) ;
					break ;
				case 'o' :
					fprintf(fp,"%o", param) ;
					break ;
				case 'u' :
					fprintf(fp,"%u", param) ;
					break ;
				case 'x' :
					fprintf(fp,"%x", param) ;
					break ;
				default :
					fprintf(fp,"%%%c",mbuffer[i]) ;
			}
			continue ;
		}
		fprintf(fp,"%c",mbuffer[i]) ;
	}
	fprintf(fp,"\n") ;
}

/* get arguments for mmu function */
int
getmmu()
{
	int c;

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(!args[optind])
		prmmu();
	else longjmp(syn,0);
}

/* print mmu registers */
int
prmmu()
{
	if (!active) {
		fprintf(fp,"Fault Code Register = %x\n", xtranvram.systate.mmufltcr);
		fprintf(fp,"Fault Address Register = %x\n", xtranvram.systate.mmufltar);
		fprintf(fp,"Configuration Register = %x\n", conrptr);
	}
	else fprintf(fp,"These registers are not available from the core image on a running system.\n");
}

/* get arguments for srams function */
int
getsrams()
{
	int c;

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(!args[optind])
		prsrams(); 
	else longjmp(syn,0);
}

/* print srams information */
int
prsrams()
{
	int i;

	if(active) {
		fprintf(fp,"These values were saved when crash was first entered:\n");
		fprintf(fp,"        SRAMA          SRAMB\n");
		fprintf(fp,"[0]   %8x      %8x\n",
			sramapt[0],
			srambpt[0]);
		fprintf(fp,"[1]   %8x      %8x\n",
			sramapt[1],
			srambpt[1]);
	}
	else {
		fprintf(fp,"         SRAMA          SRAMB\n");
		for(i = 0; i < 4; i++) 
			fprintf(fp,"[%d]    %8x        %8x\n",
				i,
				sramapt[i],
				srambpt[i]);
	}
}


/* get arguments for stat function */
int
getstat()
{
	int c;

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind])
		longjmp(syn,0);
	else prstat(); 
}


/* print system statistics */
int
prstat()
{
	int toc, lbolt;
	int panicstr;
	struct utsname utsbuf;

	/* 
	 * Locate, read, and print the system name, node name, release number,
	 * version number, and machine name.
	 */

	if(!Sys)
		if(!(Sys = symsrch("utsname")))
			error("utsname not found in symbol table\n");
	readmem((long)Sys->n_value,1,-1,(char *)&utsbuf,
		sizeof utsbuf,"utsname structure");

	fprintf(fp,"system name:\t%s\nrelease:\t%s\n",
		utsbuf.sysname,
		utsbuf.release);
	fprintf(fp,"node name:\t%s\nversion:\t%s\n",
		utsbuf.nodename,
		utsbuf.version);
	fprintf(fp,"machine name:\t%s\n", utsbuf.machine) ;
	/*
	 * Locate, read, and print the time of the crash.
	 */

	if(!Time)
		if(!(Time = symsrch("time")))
			error("time not found in symbol table\n");

	readmem((long)Time->n_value,1,-1,(char *)&toc,
		sizeof toc,"time of crash");
	fprintf(fp,"time of crash:\t%s", ctime((long *)&toc));

	/*
	 * Locate, read, and print the age of the system since the last boot.
	 */

	if(!Lbolt)
		if(!(Lbolt = symsrch("lbolt")))
			error("lbolt not found in symbol table\n");

	readmem((long)Lbolt->n_value,1,-1,(char *)&lbolt,
		sizeof lbolt,"lbolt");

	fprintf(fp,"age of system:\t");
	lbolt = lbolt/(60*HZ);
	if(lbolt / (long)(60 * 24))
		fprintf(fp,"%d day, ", lbolt / (long)(60 * 24));
	lbolt %= (long)(60 * 24);
	if(lbolt / (long)60)
		fprintf(fp,"%d hr., ", lbolt / (long)60);
	lbolt %= (long) 60;
	if(lbolt)
		fprintf(fp,"%d min.", lbolt);
	fprintf(fp,"\n");

	/*
	 * Determine if a panic occured by examining the size of the panic string. If
	 * no panic occurred return to main(). If a panic did occur locate, read, and
	 *  print the panic registers. Note: in examining an on-line system, the panic
	 *  registers will always appear to be zero.
	 */

	fprintf(fp,"panicstr:\t");
	seekmem((long)Panic->n_value,1,-1);
	if(read(mem, (char *)&panicstr, sizeof panicstr) == sizeof panicstr &&
		panicstr != 0)

		errpr((char *)panicstr,xtranvram.errlog[0].param1,
			xtranvram.errlog[0].param2,
			xtranvram.errlog[0].param3,
			xtranvram.errlog[0].param4,
			xtranvram.errlog[0].param5,0,1) ;
	else fprintf(fp,"\n");
}

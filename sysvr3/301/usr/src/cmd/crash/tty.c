/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:tty.c	1.8"
/*
 * This file contains code for the crash function tty.
 */

#include "sys/param.h"
#include "a.out.h"
#include "stdio.h"
#include "sys/sbd.h"
#include "sys/sysmacros.h"
#include "sys/types.h"
#include "sys/fs/s5dir.h"
#include "string.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "crash.h"

/* get address of tty structure */
long
getaddr(type)
char *type;
{
	struct syment *sp;
	char temp[20];

	strcpy(temp,type);
	strcat(temp,"_tty");
	if(!(sp = symsrch(temp)))
		error("%s not found in symbol table\n",type);
	return(sp->n_value);
}

/* get tty count */
int
getcount(type)
char *type;
{
	struct syment *sp;
	char temp[20];
	int count = 2;

	if(!strcmp(type,"iu"))
		return(count);
	strcpy(temp,type);
	strcat(temp,"_cnt");
	if(!(sp = symsrch(temp)))
		error("count of %s not found\n",type);
	readmem((long)sp->n_value,1,-1,(char *)&count,sizeof count,
		"tty_cnt");
	return(count);
}

/* get arguments for tty function */
int
gettty()
{
	int slot = -1;
	int full = 0;
	int all = 0;
	int phys = 0;
	int count = 0;
	char *type = "";
	long addr = -1;
	long arg2 = -1;
	long arg1 = -1;
	int c;
	char *heading = "SLOT OUT RAW CAN DEL  PGRP STATE\n";

	optind = 1;
	while((c = getopt(argcnt,args,"efpw:t:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'f' :	full = 1;
					break;
			case 'p' :	phys = 1;
					break;
			case 't' :	type = optarg;
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if((!strcmp("",type)) && !args[optind]) {
		addr = getaddr("pp");
		count = getcount("pp");
		fprintf(fp,"pp TABLE SIZE = %d\n",count);
		if(!full)
			fprintf(fp,"%s",heading);
		for(slot = 0; slot < count; slot++)
			prtty(all,full,slot,phys,addr,"",heading);
		addr = getaddr("iu");
		count = getcount("iu");
		fprintf(fp,"\niu TABLE SIZE = %d\n",count);
		if(!full)
			fprintf(fp,"%s",heading);
		for(slot = 0; slot < count; slot++)
			prtty(all,full,slot,phys,addr,"",heading);
	}
	else {
		if(strcmp(type,"")) {
			addr = getaddr(type);
			count = getcount(type);
			fprintf(fp,"%s TABLE SIZE = %d\n",type,count);
		}
		if(!full)
			fprintf(fp,"%s",heading);
		if(args[optind]) {
			all = 1;
			do {
				getargs(count,&arg1,&arg2);
				if(arg1 == -1) 
					continue;
				if(arg2 != -1)
					for(slot = arg1; slot <= arg2; slot++)
						prtty(all,full,slot,phys,addr,
							type,heading);
				else {
					if(arg1 < count)
						slot = arg1;
					else addr = arg1;
					prtty(all,full,slot,phys,addr,
						type,heading);
				}
			slot = addr = arg1 = arg2 = -1;
			}while(args[++optind]);
		}
		else for(slot = 0; slot < count; slot++)
			prtty(all,full,slot,phys,addr,"",heading);
	}
}


/* print tty table */
prtty(all,full,slot,phys,addr,type,heading)
int all,full,slot,phys;
long addr;
char *type;
char *heading;
{
	struct tty tbuf;
	long base;

	if(phys || !Virtmode)
		readmem(addr&~MAINSTORE,0,-1,(char *)&tbuf,sizeof tbuf,
			"tty structure");
	else if(slot == -1)
		readmem((long)addr,1,-1,(char *)&tbuf,
			sizeof tbuf,"tty structure");
		else readmem((long)(addr + slot * sizeof tbuf),1,-1,
			(char *)&tbuf,sizeof tbuf,"tty structure");
	if(!tbuf.t_state && !all)
		return;
	if((slot == -1) && (strcmp(type,""))){
		base = getaddr(type);
		slot = getslot(addr,base,sizeof tbuf,phys);
	}
	if(full)
		fprintf(fp,"%s",heading);
	if(slot == -1)
		fprintf(fp,"  - ");
	else fprintf(fp,"%4d", slot);
	fprintf(fp," %3d %3d %3d %3d %5d",
		tbuf.t_outq.c_cc,
		tbuf.t_rawq.c_cc,
		tbuf.t_canq.c_cc,
		tbuf.t_delct,
		tbuf.t_pgrp);
	fprintf(fp,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		tbuf.t_state & TIMEOUT ? " tout" : "",
		tbuf.t_state & WOPEN ? " wtop" : "",
		tbuf.t_state & ISOPEN ? " isop" : "",
		tbuf.t_state & TBLOCK ? " tblk" : "",
		tbuf.t_state & CARR_ON ? " carr" : "",
		tbuf.t_state & OASLP ? " oslp" : "",
		tbuf.t_state & IASLP ? " islp" : "",
		tbuf.t_state & TTSTOP ? " stop" : "",
		tbuf.t_state & EXTPROC ? " xprc" : "",
		tbuf.t_state & TACT ? " tact" : "",
		tbuf.t_state & CLESC ? " lesc" : "",
		tbuf.t_state & RTO ? " rto" : "",
		tbuf.t_state & TTIOW ? " ttiw" : "",
		tbuf.t_state & TTXON ? " ttxn" : "",
		tbuf.t_state & TTXOFF ? " ttxf" : "",
		tbuf.t_state & BUSY ? " busy" : "");


	if(!full)
		return;
	fprintf(fp,"\trawq.cf: %8x, rawq.cl: %8x\n",
			tbuf.t_rawq.c_cf,tbuf.t_rawq.c_cl);
	fprintf(fp,"\tcanq.cf: %8x, canq.cl: %8x\n",
			tbuf.t_canq.c_cf,tbuf.t_canq.c_cl);
	fprintf(fp,"\toutq.cf: %8x, outq.cl: %8x\n",
			tbuf.t_outq.c_cf,tbuf.t_outq.c_cl);
	fprintf(fp,"\t\t  c_ptr  count  size\n");
	fprintf(fp,"\ttbuf:\t%8x %5d %5d\n",
		tbuf.t_tbuf.c_ptr,
		tbuf.t_tbuf.c_count,
		tbuf.t_tbuf.c_size);
	fprintf(fp,"\trbuf:\t%8x %5d %5d\n",
		tbuf.t_rbuf.c_ptr,
		tbuf.t_rbuf.c_count,
		tbuf.t_rbuf.c_size);
	fprintf(fp,"\t  proc   line term termflag col row vrow lrow hqcnt dstat\n");
	fprintf(fp,"\t%8x %4d %4d %8x %3d %3d  %3d  %3d %5d %5d\n",
		tbuf.t_proc,	
		tbuf.t_line,	
		tbuf.t_term,	
		tbuf.t_tmflag,	
		tbuf.t_col,	
		tbuf.t_row,	
		tbuf.t_vrow,	
		tbuf.t_lrow,	
		tbuf.t_hqcnt,	
		tbuf.t_dstat);	
	fprintf(fp,"\tintr: ");
	putch((char)tbuf.t_cc[VINTR]);
	fprintf(fp,"\tquit: ");
	putch((char)tbuf.t_cc[VQUIT]);
	fprintf(fp,"\terase: ");
	putch((char)tbuf.t_cc[VERASE]);
	fprintf(fp,"\tkill: ");
	putch((char)tbuf.t_cc[VKILL]);
	fprintf(fp,"\n");
	fprintf(fp,"\teof: ");
	putch((char)tbuf.t_cc[VEOF]);
	fprintf(fp,"\teol: ");
	putch((char)tbuf.t_cc[VEOL]);
	fprintf(fp,"\tmin: ");
	putch((char)tbuf.t_cc[VMIN]);
	fprintf(fp,"\ttime: ");
	putch((char)tbuf.t_cc[VTIME]);
	fprintf(fp,"\n");
	fprintf(fp,"\tswtch: ");
	putch((char)tbuf.t_cc[VSWTCH]);
	fprintf(fp,"\n");
	fprintf(fp,"\tiflag: %s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		(tbuf.t_iflag&IGNBRK) ? " ignbrk" : "",
		(tbuf.t_iflag&BRKINT) ? " brkint" : "",
		(tbuf.t_iflag&IGNPAR) ? " ignpar" : "",
		(tbuf.t_iflag&PARMRK) ? " parmrk" : "",
		(tbuf.t_iflag&INPCK)  ? " inpck"  : "",
		(tbuf.t_iflag&ISTRIP) ? " istrip" : "",
		(tbuf.t_iflag&INLCR)  ? " inlcr"  : "",
		(tbuf.t_iflag&IGNCR)  ? " icncr"  : "",
		(tbuf.t_iflag&ICRNL)  ? " icrnl"  : "",
		(tbuf.t_iflag&IUCLC)  ? " iuclc"  : "",
		(tbuf.t_iflag&IXON)   ? " ixon"   : "",
		(tbuf.t_iflag&IXANY)  ? " ixany"  : "",
		(tbuf.t_iflag&IXOFF)  ? " ixoff"  : "");
	fprintf(fp,"\toflag: %s%s%s%s%s%s%s%s",
		(tbuf.t_oflag&OPOST)  ? " opost"  : "",
		(tbuf.t_oflag&OLCUC)  ? " olcuc"  : "",
		(tbuf.t_oflag&ONLCR)  ? " onlcr"  : "",
		(tbuf.t_oflag&OCRNL)  ? " ocrnl"  : "",
		(tbuf.t_oflag&ONOCR)  ? " onocr"  : "",
		(tbuf.t_oflag&ONLRET) ? " onlret" : "",
		(tbuf.t_oflag&OFILL)  ? " ofill"  : "",
		(tbuf.t_oflag&OFDEL)  ? " ofpel"  : "");
	fprintf(fp,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		(tbuf.t_oflag&NLDLY)  == NL0  ? " nl0"  : "",
		(tbuf.t_oflag&NLDLY)  == NL1  ? " nl1"  : "",
		(tbuf.t_oflag&CRDLY)  == CR0  ? " cr0"  : "",
		(tbuf.t_oflag&CRDLY)  == CR1  ? " cr1"  : "",
		(tbuf.t_oflag&CRDLY)  == CR2  ? " cr2"  : "",
		(tbuf.t_oflag&CRDLY)  == CR3  ? " cr3"  : "",
		(tbuf.t_oflag&TABDLY) == TAB0 ? " tab0" : "",
		(tbuf.t_oflag&TABDLY) == TAB1 ? " tab1" : "",
		(tbuf.t_oflag&TABDLY) == TAB2 ? " tab2" : "",
		(tbuf.t_oflag&TABDLY) == TAB3 ? " tab3" : "",
		(tbuf.t_oflag&BSDLY)  == BS0  ? " bs0"  : "",
		(tbuf.t_oflag&BSDLY)  == BS1  ? " bs1"  : "",
		(tbuf.t_oflag&VTDLY)  == VT0  ? " vt0"  : "",
		(tbuf.t_oflag&VTDLY)  == VT1  ? " vt1"  : "",
		(tbuf.t_oflag&FFDLY)  == FF0  ? " ff0"  : "",
		(tbuf.t_oflag&FFDLY)  == FF1  ? " ff1"  : "");
	fprintf(fp,"\tcflag: %s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		(tbuf.t_cflag&CBAUD)==B0    ? " b0"    : "",
		(tbuf.t_cflag&CBAUD)==B50   ? " b50"   : "",
		(tbuf.t_cflag&CBAUD)==B75   ? " b75"   : "",
		(tbuf.t_cflag&CBAUD)==B110  ? " b110"  : "",
		(tbuf.t_cflag&CBAUD)==B134  ? " b134"  : "",
		(tbuf.t_cflag&CBAUD)==B150	? " b150"  : "",
		(tbuf.t_cflag&CBAUD)==B200  ? " b200"  : "",
		(tbuf.t_cflag&CBAUD)==B300  ? " b300"  : "",
		(tbuf.t_cflag&CBAUD)==B600  ? " b600"  : "",
		(tbuf.t_cflag&CBAUD)==B1200 ? " b1200" : "",
		(tbuf.t_cflag&CBAUD)==B1800 ? " b1800" : "",
		(tbuf.t_cflag&CBAUD)==B2400 ? " b2400" : "",
		(tbuf.t_cflag&CBAUD)==B4800 ? " b4800" : "",
		(tbuf.t_cflag&CBAUD)==B9600 ? " b9600" : "",
		(tbuf.t_cflag&CBAUD)==B19200 ? " b19200" : "");
	fprintf(fp,"%s%s%s%s%s%s%s%s%s%s\n",
		(tbuf.t_cflag&CSIZE)==CS5   ? " cs5"   : "",
		(tbuf.t_cflag&CSIZE)==CS6   ? " cs6"   : "",
		(tbuf.t_cflag&CSIZE)==CS7   ? " cs7"   : "",
		(tbuf.t_cflag&CSIZE)==CS8   ? " cs8"   : "",
		(tbuf.t_cflag&CSTOPB) ? " cstopb" : "",
		(tbuf.t_cflag&CREAD)  ? " cread"  : "",
		(tbuf.t_cflag&PARENB) ? " parenb" : "",
		(tbuf.t_cflag&PARODD) ? " parodd" : "",
		(tbuf.t_cflag&HUPCL)  ? " hupcl"  : "",
		(tbuf.t_cflag&CLOCAL) ? " clocal" : "");
	fprintf(fp,"\tlflag: %s%s%s%s%s%s%s%s\n",
		tbuf.t_lflag & ISIG   ? " isig"   : "",
		tbuf.t_lflag & ICANON ? " icanon" : "",
		tbuf.t_lflag & XCASE  ? " xcase"  : "",
		tbuf.t_lflag & ECHO   ? " echo"   : "",
		tbuf.t_lflag & ECHOE  ? " echoe"  : "",
		tbuf.t_lflag & ECHOK  ? " echok"  : "",
		tbuf.t_lflag & ECHONL ? " echonl" : "",
		tbuf.t_lflag & NOFLSH ? " noflsh" : "");
	fprintf(fp,"\n");
}

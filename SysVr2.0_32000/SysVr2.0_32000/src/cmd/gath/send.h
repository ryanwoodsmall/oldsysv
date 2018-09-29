/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*	send.h 1.2 of 10/4/82
	@(#)send.h	1.2
 */



#include <setjmp.h>
#define F 037&

#define LNL 501
#define LNX 502
#define NTB 22
#define NDR 12
#define DBR (1<<8)

#define BCD 1
#define ASC 2
#define SIG 4

#define ENG 1
#define ETB 2
#define EDT 4
#define ELL 8
#define EEX 16

#define SLS 0141
#define STR 0134
#define DOL 0133

#define TFIL  0
#define TCOM  1
#define TEOF  2
#define TINP  3
#define TTTY  4
#define TPIN  5
#define TPTY  6
#define TSFL  7
#define TRFL  8
#define TPKY  9
#define TDHX 10
#define TDKY 11
#define TEXC 12
#define TDOL 13
#define TFMT 14
#define TMSG 15
#define TVFL 16
#define TCHD 17
#define TQHX 18
#define TQKY 19
#define TQPK 20

#define ESPIPE 29

extern NHST;

#define NLIN 8
#define NLIX 600

struct iobf
 {int fd,bc,bb;
  char *bp,bf[512];};

struct format
 {int ffd,ffe,ffm,ffs,fft;
  char ftab[NTB];};

struct keywd
 {struct keywd *nx,*lk;
  int nk,nr;
  char *kp,*rp;};

struct context
 {struct context *ocx;
  char flg[32];
  struct format *dsp;
  struct keywd *kw0,*kw1;
  char *nam;
  int lno,lvl,typ;
  int sfd;
  char stm[24];
  int dfl,dsv[NDR];};

struct trap
 {struct trap *str;
  jmp_buf save;};

extern	struct	trap	*tchain;

struct hsts
 {int hcnt;
  char *hnam;};

extern int abort,error;

extern int didf;

extern int code,maxcol;

extern int	qat,cue;
extern long	cnt;

extern int tin,oed,tty;

extern char *qer,*tmpf,*home;

extern char ss[];

extern char msglvl[];

extern struct iobf obf,dbf,qbf,tbf;

extern struct keywd *ikw,*kwx[];

extern int nhst;
extern int seqflg;
extern char hstname[];


extern char trt[],rtr[];

/*end*/

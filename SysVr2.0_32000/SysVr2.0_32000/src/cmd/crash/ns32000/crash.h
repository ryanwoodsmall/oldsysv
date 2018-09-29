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
/* @(#)crash.h	1.9 */
#include  "sys/types.h"
#include  "sys/param.h"
#include  "a.out.h"
#include  "signal.h"
#include  "sys/sysmacros.h"
#include  "sys/dir.h"
#include  "sys/user.h"
#include  "sys/var.h"
#include  "stdio.h"

#if vax || ns32000
#define N_TEXT	1
#define N_DATA	2
#define N_BSS	3
#endif

#define	VIRT_MEM	0x3fffffff
#define	SYM_VALUE(ptr)	(ptr->n_value & VIRT_MEM)
#ifdef	pdp11
#define	FMT	"%6.6o"
#define	HDR	sizeof(struct exec)
#define TXT	(HDR + abuf.a_data)
#endif
#if vax || ns32000
#define	FMT	"%8.8x"
#endif
#define	SWAPPED	1	/* Returned by getuarea if process is swapped */


struct	frame	{
	unsigned  f_r5;
	unsigned  f_ret;
} ;

struct	dstk	{
	int	r0;
	int	r1;
	int	r2;
	int	r3;
	int	r4;
	int	r5;
	int	r6;
	int	r7;
#ifdef vax
	int	r8;
	int	r9;
	int	r10;
	int	r11;
	int	r12;
	int	r13;
	int	ipl;
	int	mapen;
	int	pcbb;
	int	stkptr;
#endif
#ifdef ns32000
	int	frameptr;
	int	eia;
	int	msr;
#endif
} ;

#ifdef pdp11
struct	glop	{
	int	g_x0;
	int	g_x1;
	int	g_r0;
	int	g_r1;
	int	g_r2;
	int	g_r3;
	int	g_r4;
	int	g_r5;
	int	g_sp;
	unsigned  int  g_ka6;
} ;
#endif

struct	tsw	{
	char	*t_nm;
	int	t_sw;
	char	*t_dsc;
} ;

struct	prmode	{
	char	*pr_name;
	int	pr_sw;
} ;

#define	USIZ	(USIZE * NBPC)	/* (pdp=16*64, vax=4*512, ns32000=2*1024) */
#define MAXI	30

extern	struct	var	v;
extern	int	mem;
extern	int	kmem;

struct	uarea {
	struct user u;
	char	stk[USIZ - sizeof(struct user)];
};

#ifdef	pdp11
struct	xinterface {	/* jsr r5, call; jmp function */
	int	i_jsr;
	int	i_call;
	int	i_jmp;
	int	i_func;
};
#endif

extern	struct	syment	*File, *Inode, *Mount, *Swap, *Core, *Proc, *Sbuf,
			*Sys, *Time, *Panic, *Etext, *Text, *V,
#ifdef	ns32000
			*Kpte2,
#else
			*Sbrpte,
#endif
			*Buf, *End, *Callout, *Lbolt, *Dmpstk, *Curproc, *U,
			*Region;

#define	STACK	1
#define	UAREA	2
#define	FILES	3
#define	TRACE	4
#define	QUIT	5
#define	PCBLK	6
#define	INODE	7
#define	MOUNT	8
#define	TTY	9
#define	Q	10
#define	TEXT	11
#define	TS	13
#define	DS	14
#define	PROC	15
#define	STAT	16
#define	KFP	17
#define REGION		18
#define PREGION		19
#define	BUFHDR	20
#define	BUFFER	21
#define	TOUT	22
#define	NM	23
#define	OD	24
#define	MAP	25
#define	VAR	28
#define	LCK     29

#define	DIRECT	2
#define	OCTAL	3
#define	DECIMAL	4
#define	CHAR	5
#define	WRITE	6
#define	INODE	7
#define	BYTE	8
#define	LDEC	9
#define	LOCT	10
#define	HEX	11


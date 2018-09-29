/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpadmin/lpadmin.h	1.4"

#define BEGIN_CRITICAL	{ ignore_signals(); {
#define END_CRITICAL	} trap_signals(); }

extern void		ignore_signals(),
			trap_signals();

extern int		a,
			banner,
#if	defined(DIRECT_ACCESS)
			C,
#endif
			filebreak,
			h,
			j,
			l,
			M,
			Q,
			W,
			scheduler_active;

extern char		*A,
			*c,
			*cpi,
			*d,
			*D,
			*e,
			*f,
			*ff,
			**f_allow,
			**f_deny,
			*F,
			*i,
			**I,
			*length,
			*lpi,
			*m,
			modifications[128],
			*o,
			*p,
			*r,
			*stty,
			**S,
			*T,
			*u,
			**u_allow,
			**u_deny,
			*U,
			*v,
			*width,
			*x;

#if	defined(LPUSER)
extern SCALED		cpi_sdn,
			length_sdn,
			lpi_sdn,
			width_sdn;
#endif

#if	defined(PR_MAX)
extern PRINTER		*oldp;

extern PWHEEL		*oldS;
#endif

extern unsigned short	daisy;

extern char		*getdflt();

extern int		ismodel(),
			output(),
			verify_form(),
			do_align();

extern void		do_fault(),
			do_mount(),
			do_printer(),
			do_pwheel(),
			done(),
			fromclass(),
			newdflt(),
			options(),
			rmdest(),
			send_message(),
			startup(),
			usage();

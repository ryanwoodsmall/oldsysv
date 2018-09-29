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
/*	@(#)killall.c	1.6	*/
/*
 *  Go through the process table and kill all valid processes that
 *  are not Process 0 (sched) or Process 1 (init) or that do not
 *  have the current process group leader id.
 */

#include	<stdio.h>
#include	<a.out.h>
#include	<fcntl.h>
#include	<sys/types.h>
#ifdef u3b
#include	<sys/seg.h>
#endif /* u3b */
#include	<sys/param.h>
#include	<sys/proc.h>
#include	<sys/var.h>

#define	ERR	(-1)
#define	PROC	nl[0]
#define	VAR	nl[1]
#define	PROCSZ	(((unsigned)v.ve_proc - PROC.n_value) / sizeof(struct proc))
#ifdef u370
				/* no sched in u370 */
#define	FIRSTPROC	1
#else
#define	FIRSTPROC	2
#endif

/*
 * u3b warning:  nl[0] will be "proc" and nl[1] will be "_v"
 * regardless of what variables appear below.
 */
#if pdp11 || vax || ns32000
#define PROC_STR	"_proc"
#define V_STR		"_v"
#endif
#if u3b || u3b5 || u370
#define PROC_STR	"proc"
#define V_STR		"v"
#endif

/*
 * The 3b5 has to perform a mapping to go from physical to virtual
 * memory references.  With this command, the 2 seeks (proc and v)
 * are in kernal space and if the source for the command is changed 
 * to open /dev/kmem instead of /dev/mem, no mapping need be done.
 */
#if u3b || u3b5
#define MEMORY	"/dev/kmem"
#else
#define MEMORY	"/dev/mem"
#endif

struct nlist nl[] = {
	{ PROC_STR },
	{ V_STR },
	{ 0 },
};

main(argc, argv)
int  argc;
char *argv[];
{
	register int	i = 0;
	register int	sig, fd, pgrp;
	unsigned	Proc, V;
	struct proc	*p;
	struct var	v;

	switch(argc) {
		case 1:
			sig = 9;
			break;
		case 2:
			sig = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [signal]\n", argv[0]);
			exit(1);
	}

	if((fd = open(MEMORY, O_RDONLY)) == ERR) {
		perror("read on mem");
		exit(1);
	}

	nl[0].n_value = 0;
	nl[1].n_value = 0;

#ifdef u3b
	nl[0].n_value = sys3b(4,1); /* proc */
	nl[1].n_value = sys3b(4,4); /* v */
#else /* u3b */
	nlist("/unix", nl);

	if(PROC.n_value == 0) {
		fprintf(stderr, "%s: can't find process table\n", argv[0]);
		exit(1);
	}

	if(VAR.n_value == 0) {
		fprintf(stderr, "%s: can't find variables table\n", argv[0]);
		exit(1);
	}
#endif /* u3b */

#ifdef u370
	Proc = PROC.n_value;
	V = VAR.n_value;
#else
	Proc = (PROC.n_value & 0x3fffffff);
	V = (VAR.n_value & 0x3fffffff);
#endif

	if((pgrp = getpgrp()) == ERR) {
		perror("getpgrp");
		exit(1);
	}

#ifdef	DEBUG
printf("effective pgrp = %d\n", pgrp);
#endif

/*
 *  Read in the variable table so that the current process
 *  table size can be used.
 */

	if(lseek(fd, (long)V, 0) == ERR) {
		perror("lseek on variable table");
		exit(1);
	}

	if(read(fd, &v, sizeof(struct var)) == ERR) {
		perror("read err on var");
		exit(1);
	}

/*
 *  Seek to process table and on past slots 0 and 1
 *  which are the scheduler and init.
 */
#ifdef u370
	/* u370 stores a pointer to the process table */
	if(lseek(fd, Proc, 0) == ERR) {
		perror("lseek on process table address");
		exit(1);
	}
	if(read(fd,&Proc,sizeof(Proc)) == ERR) {
		perror("read on process table address ");
		exit(1);
	}
#endif
	if(lseek(fd, (long)Proc, 0) == ERR) {
		perror("lseek on process table");
		exit(1);
	}
#ifdef	DEBUG
	printf("procsz=%d\n",PROCSZ);
#endif

	/* allocate a process table */
	if ((p = (struct proc *)malloc(sizeof(struct proc) * PROCSZ)) == 0) {
		perror("malloc on process table copy");
		exit(1);
	}
	if(read(fd, p, sizeof(struct proc) * PROCSZ) == ERR) {
		perror("read err on proc table");
		exit(1);
	}
	for(i=FIRSTPROC; i < PROCSZ; ++i) {
		if(p[i].p_stat == 0 || p[i].p_stat == SZOMB || p[i].p_pgrp == pgrp || p[i].p_pid == 0)
			continue;
		else {
#ifdef	DEBUG
			printf("slot=%4d:  kill(%6d,%2d)\tuid=%5u\tp_pgrp=%5d\n",
				i,p[i].p_pid,sig,p[i].p_uid, p[i].p_pgrp);
#else
			/* kill process group leaders */
			if(kill(p[i].p_pid, sig) == ERR) {
				fprintf(stderr, "kill: pid= %d: ", p[i].p_pid);
				perror("");
			}
#endif
		}
	}
	exit(0);
}
#ifdef u3b
int sys3b(cmd, arg1)
{
	asm("	ost	&50");
}
#endif /* u3b */

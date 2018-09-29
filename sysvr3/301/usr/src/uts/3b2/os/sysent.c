/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/sysent.c	10.12"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"

/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a system call.
 * Each row contains the number of arguments expected,
 * a switch that tells systrap() in trap.c whether a setjmp() is not necessary,
 * and a pointer to the routine.
 */

int	alarm();
int	chdir();
int	chmod();
int	chown();
int	chroot();
int	close();
int	creat();
int	dup();
int	exec();
int	exece();
int	fcntl();
int	fork();
int	fstat();
int	getgid();
int	getpid();
int	getuid();
int	gtime();
int	gtty();
int	ino_open();
int	ioctl();
int	kill();
int	link();
int	lock();
int	locking();	/* file locking hook -- Sept 1980, John Bass */
int	locsys();
int	mknod();
int	msgsys();
int	nice();
int	nosys();
int	nullsys();
int	open();
int	pause();
int	pipe();
int	profil();
int	ptrace();
int	read();
int	rexit();
int	saccess();
int	sbreak();
int	seek();
int	semsys();
int	setgid();
int	setpgrp();
int	setuid();
int	shmsys();
int	smount();
int	ssig();
int	stat();
int	stime();
int	stty();
int	sumount();
int	sync();
int	sysacct();
int	times();
#ifdef MEGA
int	uexch();
#endif /* MEGA */
int	ulimit();
int	umask();
int	unlink();
int	utime();
int	utssys();
int	wait();
int	write();
int 	advfs();
int	rfstart();
int 	rfstop();
int 	rdebug();
int 	rmount();
int 	rumount();
int	rfsys();
int	unadvfs();
int	rmdir();
int	mkdir();
int	getdents();
int	statfs();
int	fstatfs();
int	sysfs();
int	getmsg();
int	poll();
int	putmsg();
int	sys3b();
int	uadmin();

struct sysent sysent[] =
{
	0, 1, nosys,			/*  0 = indir */
	1, 1, rexit,			/*  1 = exit */
	0, 1, fork,			/*  2 = fork */
	3, 0, read,			/*  3 = read */
	3, 0, write,			/*  4 = write */
	3, 0, open,			/*  5 = open */
	1, 0, close,			/*  6 = close */
	0, 0, wait,			/*  7 = wait */
	2, 0, creat,			/*  8 = creat */
	2, 1, link,			/*  9 = link */
	1, 1, unlink,			/* 10 = unlink */
	2, 1, exec,			/* 11 = exec */
	1, 1, chdir,			/* 12 = chdir */
	0, 1, gtime,			/* 13 = time */
	3, 1, mknod,			/* 14 = mknod */
	2, 1, chmod,			/* 15 = chmod */
	3, 1, chown,			/* 16 = chown; now 3 args */
	1, 1, sbreak,			/* 17 = break */
	2, 1, stat,			/* 18 = stat */
	3, 1, seek,			/* 19 = seek */
	0, 1, getpid,			/* 20 = getpid */
	4, 1, smount,			/* 21 = mount */
	1, 1, sumount,			/* 22 = umount */
	1, 1, setuid,			/* 23 = setuid */
	0, 1, getuid,			/* 24 = getuid */
	1, 1, stime,			/* 25 = stime */
	4, 1, ptrace,			/* 26 = ptrace */
	1, 1, alarm,			/* 27 = alarm */
	2, 1, fstat,			/* 28 = fstat */
	0, 0, pause,			/* 29 = pause */
	2, 1, utime,			/* 30 = utime */
	2, 0, stty,			/* 31 = stty */
	2, 0, gtty,			/* 32 = gtty */
	2, 1, saccess,			/* 33 = access */
	1, 1, nice,			/* 34 = nice */
	4, 1, statfs,			/* 35 = statfs */
	0, 0, sync,			/* 36 = sync */
	2, 1, kill,			/* 37 = kill */
	4, 1, fstatfs,			/* 38 = fstatfs */
	1, 1, setpgrp,			/* 39 = setpgrp */
	0, 1, nosys,			/* 40 = tell - obsolete */
	1, 1, dup,			/* 41 = dup */
	0, 0, pipe,			/* 42 = pipe */
	1, 1, times,			/* 43 = times */
	4, 1, profil,			/* 44 = prof */
	1, 1, lock,			/* 45 = proc lock */
	1, 1, setgid,			/* 46 = setgid */
	0, 1, getgid,			/* 47 = getgid */
	2, 0, ssig,			/* 48 = sig */
	6, 0, msgsys,			/* 49 = IPC message */
	4, 1, sys3b,			/* 50 = 3b2-specific system call */
	1, 1, sysacct,			/* 51 = turn acct off/on */
	4, 0, shmsys,                   /* 52 = shared memory */
	5, 0, semsys,			/* 53 = IPC semaphores */
	3, 0, ioctl,			/* 54 = ioctl */
	3, 0, uadmin,			/* 55 = uadmin */
#ifdef MEGA
	3, 1, uexch,			/* 56 = uexch */
#else
	0, 1, nosys,			/* 56 = reserved for exch */
#endif /* MEGA */
	3, 1, utssys,			/* 57 = utssys */
	0, 1, nosys,			/* 58 = reserved for USG */
	3, 1, exece,			/* 59 = exece */
	1, 1, umask,			/* 60 = umask */
	1, 1, chroot,			/* 61 = chroot */
	3, 0, fcntl,			/* 62 = fcntl */
	2, 1, ulimit,			/* 63 = ulimit */

/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a vmunix special system call.
 * Each row contains the number of arguments expected
 * and a pointer to the routine.
 */

/* The following 6 entries were reserved for Safari 4 */
	0, 1, nosys,			/* 64 +0 = nosys */
	0, 1, nosys,			/* 64 +1 = nosys */
	0, 1, nosys,			/* 64 +2 = nosys */
	0, 1, nosys,			/* 64 +3 = file locking call */
	0, 1, nosys,			/* 64 +4 = local system calls */
	0, 1, nosys,			/* 64 +5 = inode open */
	4, 1, advfs,			/* 70 = advfs */
	1, 1, unadvfs,			/* 71 = unadvfs */
	4, 1, rmount,			/* 72 = rmount */
	1, 1, rumount,			/* 73 = rumount */
	5, 0, rfstart,			/* 74 = rfstart */
	0, 1, nosys,			/* 75 = not used */
	1, 0, rdebug,			/* 76 = rdebug */
	0, 1, rfstop,			/* 77 = rfstop */
	6, 1, rfsys,			/* 78 = rfsys */
	1, 1, rmdir,			/* 79 = rmdir */
	2, 1, mkdir,			/* 80 = mkdir */
	4, 1, getdents,			/* 81 = getdents */
	0, 1, nosys,			/* 82 = not used */
	0, 1, nosys,			/* 83 = not used */
	3, 1, sysfs,			/* 84 = sysfs */
	4, 0, getmsg,			/* 85 = getmsg */
	4, 0, putmsg,			/* 86 = putmsg */
	3, 0, poll,			/* 87 = poll */
};

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
/* @(#)sysent.c	6.2 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"

/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a system call.
 * Each row contains the number of arguments expected
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
int	ioctl();
int	kill();
int	link();
int	lock();
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
int	ulimit();
int	umask();
int	unlink();
int	utime();
int	utssys();
int	swapfunc();
int	wait();
int	write();

struct sysent sysent[] =
{
	0, 0, nosys,			/*  0 = indir */
	1, 0, rexit,			/*  1 = exit */
	0, 0, fork,			/*  2 = fork */
	3, 0, read,			/*  3 = read */
	3, 0, write,			/*  4 = write */
	3, 0, open,			/*  5 = open */
	1, 0, close,			/*  6 = close */
	0, 0, wait,			/*  7 = wait */
	2, 0, creat,			/*  8 = creat */
	2, 0, link,			/*  9 = link */
	1, 0, unlink,			/* 10 = unlink */
	2, 0, exec,			/* 11 = exec */
	1, 0, chdir,			/* 12 = chdir */
	0, 0, gtime,			/* 13 = time */
	3, 0, mknod,			/* 14 = mknod */
	2, 0, chmod,			/* 15 = chmod */
	3, 0, chown,			/* 16 = chown; now 3 args */
	1, 0, sbreak,			/* 17 = break */
	2, 0, stat,			/* 18 = stat */
	3, 0, seek,			/* 19 = seek */
	0, 0, getpid,			/* 20 = getpid */
	3, 0, smount,			/* 21 = mount */
	1, 0, sumount,			/* 22 = umount */
	1, 0, setuid,			/* 23 = setuid */
	0, 0, getuid,			/* 24 = getuid */
	1, 0, stime,			/* 25 = stime */
	4, 0, ptrace,			/* 26 = ptrace */
	1, 0, alarm,			/* 27 = alarm */
	2, 0, fstat,			/* 28 = fstat */
	0, 0, pause,			/* 29 = pause */
	2, 0, utime,			/* 30 = utime */
	2, 0, stty,			/* 31 = stty */
	2, 0, gtty,			/* 32 = gtty */
	2, 0, saccess,			/* 33 = access */
	1, 0, nice,			/* 34 = nice */
	0, 0, nosys,			/* 35 = sleep; inoperative */
	0, 0, sync,			/* 36 = sync */
	2, 0, kill,			/* 37 = kill */
	0, 0, nosys,			/* 38 = x */
	1, 0, setpgrp,			/* 39 = setpgrp */
	0, 0, nosys,			/* 40 = tell - obsolete */
	1, 0, dup,				/* 41 = dup */
	0, 0, pipe,			/* 42 = pipe */
	1, 0, times,			/* 43 = times */
	4, 0, profil,			/* 44 = prof */
	1, 0, lock,			/* 45 = proc lock */
	1, 0, setgid,			/* 46 = setgid */
	0, 0, getgid,			/* 47 = getgid */
	2, 0, ssig,			/* 48 = sig */
	6, 0, msgsys,			/* 49 = IPC Messages */
	0, 0, nosys,			/* 50 = reserved for local use */
	1, 0, sysacct,			/* 51 = turn acct off/on */
	4, 0, shmsys,			/* 52 = IPC Shared Memory */
	5, 0, semsys,			/* 53 = IPC Semaphores */
	3, 0, ioctl,			/* 54 = ioctl */
	0, 0, nosys,			/* 55 = x */
	0, 0, nosys,			/* 56 = x */
	3, 0, utssys,			/* 57 = utssys */
	1, 0, swapfunc,			/* 58 = swap functions */
	3, 0, exece,			/* 59 = exece */
	1, 0, umask,			/* 60 = umask */
	1, 0, chroot,			/* 61 = chroot */
	3, 0, fcntl,			/* 62 = fcntl */
	2, 0, ulimit,			/* 63 = ulimit */
};

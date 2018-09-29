/*	libc.h
	Declarations of functions in /xe/3b2/3.0/usr/lib/llib-lc
*/
/*LINTLIBRARY*/
#ident	"Based on:	lint:llib-lc	2.19"
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <pwd.h>
/* ACCESS(2) */
extern	int	access();
/* ACCT(2) */
extern	int	acct();
/* ALARM(2) */
extern	unsigned	alarm();
/* BRK(2) */
extern	int	brk();
extern	char *	sbrk();
/* CHDIR(2) */
extern	int	chdir();
/* CHMOD(2) */
extern	int	chmod();
/* CHOWN(2) */
extern	int	chown();
/* CHROOT(2) */
extern	int	chroot();
/* CLOSE(2) */
extern	int	close();
/* CREAT(2) */
extern	int	creat();
/* DUP(2) */
extern	int	dup();
/* EXEC(2) */
extern	int	execl();
extern	int	execv();
extern	int	execle();
extern	int	execve();
extern	int	execlp();
extern	int	execvp();
/* EXIT(2) */
extern	void	exit();
extern	void	_exit();
/* FCNTL(2) */
extern	int	fcntl();
/* FORK(2) */
extern	int	fork();
/* GETPID(2) */
extern	int	getpid();
extern	int	getpgrp();
extern	int	getppid();
/* GETUID(2) */
extern	unsigned short	getuid();
extern	unsigned short	geteuid();
extern	unsigned short	getgid();
extern	unsigned short	getegid();
/* IOCTL(2) */
extern	int	ioctl();
/* KILL(2) */
extern	int	kill();
/* LINK(2) */
extern	int	link();
/* LSEEK(2) */
extern	long	lseek();
#if pdp11
/* MAUS(2) PDP-11 IPC */
extern	int	dismaus();
extern	char *	enabmaus();
extern	int	freemaus();
extern	int	getmaus();
extern	char *	switmaus();
#endif
/* MKNOD(2) */
extern	int	mknod();
/* MOUNT(2) */
extern	int	mount();
/* MSGCTL(2) */
extern	int	msgctl();
/* MSGGET(2) */
extern	int	msgget();
/* MSGOP(2) */
extern	int	msgsnd();
extern	int	msgrcv();
/* NICE(2) */
extern	int	nice();
/* OPEN(2) */
extern	int	open();
/* PAUSE(2) */
extern	int	pause();
/* PIPE(2) */
extern	int	pipe();
/* PLOCK(2) */
extern	int	plock();
/* PROFIL(2) */
extern	void	profil();
/* PTRACE(2) */
extern	int	ptrace();
/* READ(2) */
extern	int	read();
/* SEMCTL(2) */
extern	int	semctl();
/* SEMGET(2) */
extern	int	semget();
/* SEMOP(2) */
extern	int	semop();
/* SETPGRP(2) */
extern	int	setpgrp();
/* SETUID(2) */
extern	int	setuid();
extern	int	setgid();
/* SHMCTL(2) */
#include <sys/shm.h>
extern	int	shmctl();
/* SHMGET(2) */
extern	int	shmget();
/* SHMOP(2) */
extern	char *	shmat();
extern	int	shmdt();
/* SIGNAL(2) */
extern	void    (*	signal( ))();
/* SIGSET(2) */
#include <signal.h>
extern	void    ();
/* STAT(2) */
#include <sys/stat.h>
extern	int	stat();
extern	int	fstat();
/* STIME(2) */
extern	int	stime();
/* SYNC(2) */
extern	void	sync();
#if u3b || u3b5
/* SYS3B(2) (3B20 and 3B5 only) */
extern	void	sys3b();
#endif
/* TIME */
extern	long	time();
/* TIMES(2) */
#include <sys/times.h>
extern	long	times();
/* ULIMIT(2) */
extern	daddr_t	ulimit();
/* UMASK(2) */
extern	int	umask();
/* UMOUNT(2) */
extern	int	umount();
/* UNAME(2) */
#include <sys/utsname.h>
extern	int	uname();
/* UNLINK(2) */
extern	int	unlink();
/* USTAT(2) */
#include <ustat.h>
extern	int	ustat();
/* UTIME(2) */
extern	int	utime();
/* WAIT(2) */
extern	int	wait();
/* WRITE(2) */
extern	int	write();
/* A64L(3C) */
extern	long	a64l();
extern	char *	l64a();
/* ABORT(3C) */
extern	int	abort();
/* ABS(3C) */
extern	int	abs();
/* ASSERT(3X) */
extern	void	_assert();
/* BSEARCH(3C) */
extern	char *	bsearch();
/* CONV(3C) */
/* CLOCK(3C) */
extern	long	clock();
/* CRYPT(3C) */
extern	char *	crypt();
extern	void	setkey();
extern	void	encrypt();
/* CTERMID(3S) */
extern	char *	ctermid();
/* CTIME(3C) */
extern	char *	ctime();
#include <time.h>
extern	struct tm *	localtime();
extern	struct tm *	gmtime();
extern	char *	asctime();
extern	void	tzset();
/* CTYPE(3C) */
/* CUSERID(3S) */
extern	char *	cuserid();
/* DIAL(3C) */
#include <dial.h>
extern	int	dial();
extern	void	undial();
/* DRAND48(3C) */
extern	double	drand48();
extern	double	erand48();
extern	long	lrand48();
extern	long	nrand48();
extern	long	mrand48();
extern	long	jrand48();
extern	void	srand48();
extern	unsigned short *	seed48();
extern	void	lcong48();
/* ECVT(3C) */
extern	char *	ecvt();
extern	char *	fcvt();
extern	char *	gcvt();
/* END(3C) */
/* FCLOSE(3S) */
extern	int	fclose();
extern	int	fflush();
/* FERROR(3S) */
/* FOPEN(3S) */
extern	FILE *	fopen();
extern	FILE *	freopen();
extern	FILE *	fdopen();
/* FPTRAP(3X) */
/* FREAD(3S) */
extern	int	fread();
extern	int	fwrite();
/* FREXP(3C) */
extern	double	frexp();
extern	double	ldexp();
extern	double	modf();
/* FSEEK(3S) */
extern	int	fseek();
extern	long	ftell();
extern	void	rewind();
/* FTW(3C) */
extern	int	ftw();
/* GETC(3S) */
/* GETCWD(3C) */
extern	char *	getcwd();
/* GETENV(3C) */
extern	char *	getenv();
/* GETGRENT(3C) */
#include <grp.h>
extern	struct group *	getgrent();
extern	struct group *	getgrgid();
extern	struct group *	getgrnam();
extern	void	setgrent();
extern	void	endgrent();
extern	struct group *	fgetgrent();
/* GETLOGIN(3C) */
extern	char *	getlogin();
/* GETOPT(3C) */
extern	int	getopt();
/* GETPASS(3C) */
extern	char *	getpass();
/* GETPW(3C) */
extern	int	getpw();
/* GETPWENT(3C) */
extern	struct passwd *	getpwent();
extern	struct passwd *	getpwuid();
extern	struct passwd *	getpwnam();
extern	void	setpwent();
extern	void	endpwent();
extern	struct passwd *	fgetpwent();
/* GETS(3S) */
extern	char *	gets();
extern	char *	fgets();
/* HSEARCH(3C) */
#include <search.h>
extern	ENTRY *	hsearch();
extern	int	hcreate();
extern	void	hdestroy();
/* L3TOL(3C) */
extern	void	l3tol();
extern	void	ltol3();
/* LD...(3X) libld -- shouldn't be documented in Section 3! */
/* LOGNAME(3X) libPW -- shouldn't be documented */
/* LSEARCH(3C) */
extern	char *	lsearch();
extern	char *	lfind();
/* MALLOC(3C) */
extern	char *	malloc();
extern	void	free();
extern	char *	realloc();
extern	char *	calloc();
/* MEMORY(3C) */
extern	char *	memccpy();
extern	char *	memchr();
extern	int	memcmp();
extern	char *	memcpy();
extern	char *	memset();
/* MKTEMP(3C) */
extern	char *	mktemp();
/* MONITOR(3C) */
#include <mon.h>
extern	void	monitor();
/* NLIST(3C) */
#include <a.out.h>
extern	int	nlist();
/* PERROR(3C) */
extern	void	perror();
/* PLOT(3X) not in libc */
/* POPEN(3S) */
extern	FILE *	popen();
extern	int	pclose();
/* PRINTF(3S) */
extern	int	printf();
extern	int	fprintf();
extern	int	sprintf();
/* PUTC(3S) */
/* PUTENV(3C) */
extern	int	putenv();
/* PUTPWENT(3C) */
extern	int	putpwent();
/* PUTS(3S) */
extern	int	puts();
extern	int	fputs();
/* QSORT(3C) */
extern	void	qsort();
/* RAND(3C) */
extern	void	srand();
extern	int	rand();
/* REGCMP(3X) libPW */
/* SCANF(3S) */
extern	int	scanf();
extern	int	fscanf();
extern	int	sscanf();
/* SETBUF(3S) */
extern	void	setbuf();
extern	int	setvbuf();
/* SETJMP(3C) */
#include <setjmp.h>
extern	int	setjmp();
extern	void	longjmp();
/* SLEEP(3C) */
extern	unsigned	sleep();
/* SSIGNAL(3C) */
extern	int (*	ssignal( ))();
extern	int	gsignal();
/* STDIO(3S) */
/* STDIPC(3C) */
extern	key_t	ftok();
/* STRING(3C) */
extern	char *	strcat();
extern	char *	strncat();
extern	int	strcmp();
extern	int	strncmp();
extern	char *	strcpy();
extern	char *	strncpy();
extern	int	strlen();
extern	char *	strchr();
extern	char *	strrchr();
extern	char *	strpbrk();
extern	int	strspn();
extern	int	strcspn();
extern	char *	strtok();
extern	char *  strdup();
/* STRTOD(3C) */
extern	double	strtod();
extern	double	atof();
/* STRTOL(3C) */
extern	long	strtol();
extern	long	atol();
extern	int	atoi();
/* SWAB(3C) */
extern	void	swab();
/* SYSTEM(3S) */
extern	int	system();
/* TMPFILE(3S) */
extern	FILE *	tmpfile();
/* TMPNAM(3S) */
extern	char *	tmpnam();
extern	char *	tempnam();
/* TSEARCH(3C) */
extern	char *	tsearch();
extern	char *	tfind();
extern	char *	tdelete();
extern	void	twalk();
/* TTYNAME(3C) */
extern	char *	ttyname();
extern	int	isatty();
/* UNGETC(3S) */
extern	int	ungetc();
/* VPRINTF(3S) */
#include <varargs.h>
extern	int	vprintf();
extern	int	vfprintf();
extern	int	vsprintf();
/* UNDOCUMENTED (not for general use) */
extern	int	syscall();
extern	long	tell();
extern	int	ttyslot();
extern	int	_filbuf();
extern	int	_flsbuf();

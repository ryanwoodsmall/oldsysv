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
/*	@(#)whodo.c	1.10	*/
/* '/etc/whodo -- combines info from utmp file and process table' */
#include   <stdio.h>
#include   <fcntl.h>
#include   <nlist.h>
#include   <time.h>
#include   <sys/types.h>
#ifdef  u3b
#include   <sys/seg.h>
#include   <sys/page.h>
#include   <sys/region.h>
#endif /* u3b */
#include   <sys/param.h>
#ifdef u3b2
#include   <sys/immu.h>
#endif /* u3b2 */
#include   <utmp.h>
#include   <pwd.h>
#include   <sys/utsname.h>
#include   <sys/proc.h>
#include   <sys/var.h>
#include   <sys/stat.h>
#include   <sys/signal.h>
#include   <sys/dir.h>
#include   <sys/user.h>
#include   <sys/sysmacros.h>
#ifndef pdp11

#define KMEM    "/dev/kmem"
#else
#define KMEM    "/dev/mem"
#endif
#define MEM     "/dev/mem"
#define ENTRY   sizeof(struct psdata)
#define ERR     (-1)
#define PROC    nl[0]
#define SWAP    nl[1]
#define VAR     nl[2]
#define SBR     nl[3]
#define error(str)      fprintf(stderr, "%s: %s\n", arg0, str)
#define prerror(str)    fprintf(stderr, "%s: %s: %s\n", \
                                arg0, str, sys_errlist[errno])

#if pdp11 || vax || ns32000
#define PROC_STR        "_proc"
#define V_STR           "_v"
#endif
#if u3b || u370 || u3b5 || u3b2
#define PROC_STR        "proc"
#define V_STR           "v"
#endif
#define SWAPLO_STR      "_swplo"
#define SBRPTE_STR      "_sbrpte"

struct nlist nl[] = {
        { PROC_STR },
        { SWAPLO_STR },
        { V_STR },
#if vax
        { SBRPTE_STR },
#endif
        { 0 }
};

/**
 * file /etc/ps_data is built by ps command,
 * only use 1st part (device info) and
 * 3rd part (/unix namelist info)
 **/
struct psdata {
        char    device[DIRSIZ];
        short   dev;    /* major/minor of device */
} ;
struct udata {  /* only used for size of entry */
        ushort  userid;
        char    usernm[8];
} ;
int             ndevs;  /* number of dev info entries to follow */
struct psdata   *psptr;

struct proc     *pp;
struct uproc {
#ifdef  pdp11
        char    p_stat;
        char    p_flag;
        ushort  p_uid;
        ushort  p_suid;
        short   p_pgrp;
        short   p_pid;
        short   p_ppid;
#else
        char    xx[18];
#endif
        short   p_ttyd;
        time_t  p_time;
        char    p_comm[DIRSIZ];
};

struct passwd   *getpwnam(), *getpwuid();
struct var      v;
unsigned        Proc, V, size;
int             fd, kmem, memfd, swap;
char            *arg0;
long            swaplo;
#ifdef  vax
int             sbrpte;
#endif
extern int      errno;
extern char     *sys_errlist[];
extern char     *optarg;
int             uid;
struct user     uarea;

main(argc, argv)
int  argc;
char *argv[];
{
        register struct utmp    *ut;
        register struct tm      *tm;
        register struct proc    *p;
        struct user     *u, *getuarea();
        struct uproc    *up;
        unsigned        utmpend;
        struct tm       *localtime();
        struct stat     sbuf;
        struct utsname  uts;
        char            *malloc();
        char            *getty();
        long            clock;
        int             aflag = 0;
        int             uflag = 0;
        int             c;

        arg0 = argv[0];

        /**
         * print current time and date
         **/
        time(&clock);
        printf("%s", ctime(&clock));

        /**
         * print system name
         **/
        uname(&uts);
        printf("%s\n", uts.sysname);

        /**
         * read in device info from ps_data file
         **/
        if((fd = open("/etc/ps_data", O_RDONLY)) == ERR) {
                prerror("open error of /etc/ps_data");
                exit(1);
        }
        /* first int tells how many entries follow */
        if(read(fd, (char *)&ndevs, sizeof(ndevs)) == ERR) {
                prerror("read error of size of device table info");
                exit(1);
        }
        if((psptr = (struct psdata *)malloc(ndevs*ENTRY)) == NULL) {
                prerror("malloc error of ps_data device table");
                exit(1);
        }
        if(read(fd, (char *)psptr, ndevs*ENTRY) == ERR) {
                prerror("read error of /etc/ps_data device info");
                exit(1);
        }
        /**
         * skip past uid data in /etc/ps_data
         **/
        if(read(fd, (char *)&size, sizeof(size)) == ERR) {
                prerror("read error of number of /etc/ps_data uid entries");
                exit(1);
        }
        if(lseek(fd, (long)(size*sizeof(struct udata)), 1) == ERR) {
                prerror("seek error of skipping uid entries in /etc/ps_data");
                exit(1);
        }
        /**
         * fill in the name list table from /etc/ps_data
         **/
        if(read(fd, (char *)nl, sizeof(nl)) == ERR) {
                prerror("read error of namelist data from /etc/ps_data");
                exit(1);
        }
        close(fd);

        /**
         * check offsets into memory for pointers
         **/
        if(PROC.n_value == 0) {
                error("can't find process table");
                exit(1);
        }
        if(VAR.n_value == 0) {
                error("can't find variables table");
                exit(1);
        }
        if((swaplo = (long)SWAP.n_value) == 0L) {
                error("can't find base of swap");
                exit(1);
        }
#ifdef  vax
        if(SBR.n_value == 0) {  /* sbrpte */
                error("can't find sbrpte symbol in namelist");
                exit(1);
        }
#endif
        Proc = PROC.n_value;
        V = VAR.n_value;
#ifdef  vax
        Proc &= 0x3fffffff;
        V &= 0x3fffffff;
        SBR.n_value &= 0x3fffffff;
        swaplo &= 0x3fffffff;
#endif

        /**
         * open memory file(s)
         **/
        if((kmem = open(KMEM, O_RDONLY)) == ERR) {
                prerror("open error of /dev/kmem");
                exit(1);
        }
        if((memfd  = open(MEM,  O_RDONLY)) == ERR) {
                prerror("open error of /dev/mem");
                exit(1);
        }

        swap = openswap();
        initswaplo();
#ifdef  vax
        initsbrpte();
#endif

        /**
         * read in system variable table
         **/
        if(lseek(kmem, (long)V, 0) == ERR) {
                prerror("seek error of variable table");
                exit(1);
        }
        if(read(kmem, (char *)&v, sizeof(struct var)) == ERR) {
                prerror("read error of variable table");
                exit(1);
        }

        /**
         * read in current /etc/utmp file
         **/
        if(stat(UTMP_FILE, &sbuf) == ERR) {
                prerror("stat error of /etc/utmp");
                exit(1);
        }
        size = (unsigned)sbuf.st_size;
        if((ut = (struct utmp *)malloc(size)) == NULL) {
                prerror("malloc error of utmp file");
                exit(1);
        }
        if((fd = open(UTMP_FILE, O_RDONLY)) == ERR) {
                prerror("open error of /etc/utmp");
                exit(1);
        }
        if(read(fd, (char *)ut, size) == ERR) {
                prerror("read error of /etc/utmp");
                exit(1);
        }
        utmpend = (unsigned)ut + size;  /* ptr to end of utmp data */
        close(fd);


#ifdef  u370
        /* u370 stores a pointer to the process table */
        if(lseek(kmem, (long)Proc, 0) == ERR) {
                prerror("seek error of process table address");
                exit(1);
        }
        if(read(kmem, (char *)&Proc, sizeof(Proc)) == ERR) {
                prerror("read error of process table address ");
                exit(1);
        }
#endif

        /**
         * allocate a process table
         **/
        if(sizeof(struct proc) < sizeof(struct uproc)) {
                error("size error: uproc larger than proc");
                exit(1);
        }
        size = (((unsigned)v.ve_proc & 0x3fffffff) - (unsigned)Proc) /
                sizeof(struct proc);
        if((pp = (struct proc *)malloc(sizeof(struct proc)*size)) == NULL) {
                prerror("malloc error of process table");
                exit(1);
        }
        if(lseek(kmem, (long)Proc, 0) == ERR) {
                prerror("seek error of process table");
                exit(1);
        }
        if(read(kmem, (char *)pp, sizeof(struct proc)*size) == ERR) {
                prerror("read error of process table");
                exit(1);
        }

        /**
         * gather uarea and plug in applicable data
         * into the process table slot mask
         */
        for(p = pp; p < (struct proc *)&pp[size]; ++p) {
                if(! p->p_stat || p->p_stat==SZOMB)
                        continue;
                if((u = getuarea(p)) == NULL) {
                        /* process has disappeared */
                        p->p_stat = 0;
                        continue;
                }
                up = (struct uproc *)p;
#ifdef  pdp11
                /* reassign overlay values to remove unwanted
                 * fields and make the uproc structure the
                 * same size as the proc structure.  Do not
                 * change the order of these assignments.
                 */
                up->p_uid = p->p_uid;
                up->p_suid = p->p_suid;
                up->p_pgrp = p->p_pgrp;
                up->p_pid = p->p_pid;
                up->p_ppid = p->p_ppid;
#endif
                up->p_ttyd = u->u_ttyd;
                up->p_time = u->u_utime + u->u_stime;
                strncpy(up->p_comm, u->u_comm, DIRSIZ);
        }
        close(kmem);
        close(memfd);

        /**
         * loop through utmp file, printing process info
         * about each logged in user
         **/
        for(; ut < (struct utmp *)utmpend; ut++) {
                if(ut->ut_type != USER_PROCESS)
                        continue;
                tm = localtime(&ut->ut_time);
                printf("\n%-8.8s %-8.8s %2.1d:%2.2d\n",
                        ut->ut_line, ut->ut_name, tm->tm_hour, tm->tm_min);
                uid = getpwnam(ut->ut_name)->pw_uid;
                sproc(ut->ut_pid);
        }
}

sproc(pid)
register short pid;
{
#ifdef  pdp11
        register struct uproc   *p;
#else
        register struct proc    *p;
#endif
        register struct uproc   *up;
        char                    *getty();
        register int            found = 0;

#ifdef  pdp11
        for(p = (struct uproc *)pp; p < &pp[size]; p++) {
#else
        for(p = pp; p < &pp[size]; p++) {
#endif
                if(! p->p_stat)
                        continue;

                if(pid == p->p_ppid || pid == p->p_pgrp)
                        found++;

                if(found || pid==p->p_pid) {
                        up = (struct uproc *)p;
                        if(p->p_stat == SZOMB)
                                printf("    %-7.7s %5d %4.1ld:%2.2ld %s\n",
                                        "  ?", p->p_pid, 0L, 0L, "<defunct>");
                        else {
                                up->p_time = (up->p_time + HZ / 2L) / HZ;
                                printf("    %-7.7s %5d %4.1ld:%2.2ld %s\n",
                                        getty(up->p_ttyd), p->p_pid,
                                        up->p_time/60L, up->p_time%60L,
                                        up->p_comm);
                        }
                        p->p_stat =  0;
                }
                if(found) {
                        found = 0;
                        sproc(p->p_pid);
                }
        }
}

struct user *
getuarea(p)
register struct proc *p;
{
        register long   addr;
        register int    c;
#if	vax || ns32000
	int	ww;
#endif
#ifdef  vax
        int     spte;
        int     upte[128];
#endif
#ifdef	ns32000
	struct {
		int pte, pte2;
	} udotpte[USIZE];
#endif

        fd = memfd;

        /* Determine if process is in memory or swap space */
        /* and read in user block */
#ifdef  u3b
        addr = p->p_uptbl[0] & PG_ADDR;
        if(lseek(fd, addr, 0) == ERR)
                perror("cannot seek to uarea");
        if(read(fd, &uarea, sizeof(uarea)) != sizeof(uarea)) {
                perror("read error on ublock");
                return(NULL);
        }
#endif /* u3b */

#ifdef  vax
        ww = (int)(p->p_spt + (p->p_nspt-1)*128);
        ww = sbrpte + ((ww&0x3fffffff)>>9)*4;
        lseek(fd,(long)ww&0x3fffffff,0);
        read(fd,&spte, 4);
        if(spte == 0)
                return(NULL);
        lseek(fd, (spte&0x1fffff)<<9,0);
        read(fd, upte, sizeof upte);
        for(c=0; c<USIZE; c++) {
                lseek(fd, (upte[128-USIZE+c]&0x1fffff)<<9, 0);
                     /*
                      * get u page
                      */
                if(read(fd,(char *)(((int *)&uarea)+128*c),512) != 512)
                        return(NULL);
                if(sizeof(struct user) - (c + 1) * 512 <= 0)
                        break;
        }
#endif
#ifdef  pdp11
        if (p->p_flag&SLOAD)
                addr = ctob((long)p->p_addr);
        else {
                addr = (p->p_addr+swaplo)<<9;
                fd = swap;
        }
        lseek(fd, addr, 0);
        if (read(fd, (char *)&uarea, sizeof(uarea)) != sizeof(uarea))
                return(NULL);
#endif
#ifdef	ns32000
	ww = (p->p_addr << 10) + 512;
	lseek(fd, (long)ww, 0);
	read(fd, udotpte, sizeof(udotpte));
	for(c=0; c<USIZE; c++) {
		lseek(fd, udotpte[c].pte & 0xfffc00, 0);
			/*
			 * get u page
			 */
		if (read(fd,(char *)(((int *)&uarea)+256*c),1024) != 1024)
			return(0);
		if (sizeof(struct user) - (c + 1) * 1024 <= 0)
			break;
	}
#endif
        return(&uarea);
}

char *
getty(dev)
register short dev;
{
        register struct psdata *ps;

        for(ps = psptr; ps < &psptr[ndevs]; ps++) {
                if(ps->dev == dev)
                        return(ps->device);
        }
        return("  ?  ");
}

openswap()
{
        int     swp;

        if((swp = open("/dev/swap", O_RDONLY)) == ERR) {
                prerror("open error of /dev/swap");
                exit(1);
        }
        return(swp);
}

initswaplo()
{
        /**
         * read in base of swap
         **/
        if(lseek(kmem, (long)swaplo, 0) == ERR) {
                prerror("seek error of swplo symbol");
                exit(1);
        }
        if(read(kmem, (char *)&swaplo, sizeof(swaplo)) == ERR) {
                prerror("read error of swplo value");
                exit(1);
        }
}

#ifdef vax
initsbrpte()
{
        /**
         * read to find system page tables
         **/
        if(lseek(kmem, (long)SBR.n_value, 0) == ERR) {
                prerror("seek error of sbrpte symbol");
                exit(1);
        }
        if(read(kmem, (char *)&sbrpte, sizeof(sbrpte)) == ERR) {
                prerror("read error of sbrpte value");
                exit(1);
        }
        sbrpte &= 0x3fffffff;
}
#endif

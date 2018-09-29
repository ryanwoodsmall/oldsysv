/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)fuser:fuser.c	1.27.1.8"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*			The last swap based fuser for the 3B5
			and the 3B2 is rev 1.17 */
#include <nlist.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>

#include <sys/param.h>
#include <sys/types.h>

#include <sys/fs/s5dir.h>
#include <sys/file.h>
#include <sys/inode.h>

#ifdef u3b
#include <sys/page.h>
#include <sys/region.h>
#endif

#ifdef u3b2
#include <sys/immu.h>
#include <sys/psw.h>
#include <sys/pcb.h>
#include <sys/region.h>
#include <sys/sbd.h>
#include <sys/sys3b.h>
#endif

#include <sys/proc.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/user.h>
#include <sys/var.h>
#include <sys/mount.h>
#include <sys/sema.h>
#include <sys/comm.h>

/* definition of memory page size */
#if pdp11 || vax
#define PSIZE	512
#define PSHIFT	9
#endif
#ifdef u3b2
#define PSIZE	2048
#define PSHIFT	11
#endif

/* symbol names */
#if pdp11 || vax
#define V_STR		"_v"
#define PROC_STR	"_proc"
#define FILE_STR	"_file"
#define INODE_STR	"_inode"
#define SWPLO_STR	"_swplo"
#define SBRPTE_STR	"_sbrpte"
#endif
#if u3b || u3b15 || u3b2
#define V_STR		"v"
#define PROC_STR	"proc"
#define FILE_STR	"file"
#define INODE_STR	"inode"
#define MOUNT_STR	"mount"
#ifdef u3b
#define SWPLO_STR	"swaplow"
#endif
#if u3b15 || u3b2
#define	SWPLO_STR	"swplo"
#endif
#endif
#define NSNDD_STR "nsndd"
#define SNDD_STR  "sndd"
#define NRCVD_STR "nrcvd"
#define RCVD_STR  "rcvd"
#define RFSFST_STR "dufstyp"
#define PREGPP_STR "pregpp"

#if vax || u3b15 || u3b2
#define MEMF "/dev/kmem"
#else
#define MEMF "/dev/mem"
#endif
#define SYSTEM "/unix"

/*	types of files, etc */
#define LOCAL	1
#define	REMOTE	2
#define	RESRC	3

#define IEQ(X, Y) ((X.i_dev == Y.i_dev) && (X.i_number == Y.i_number)) || \
	(((X.i_ftype & IFMT) == IFBLK) && (X.i_rdev == Y.i_dev))
#define MIN(X, Y)  ((X < Y) ? X : Y)

#ifdef u3b
/* NOTE:  On 3b nlist can't find FILE_STR do to a name
 * conflict with minfo.file (see sysinfo.h)
 * the u3b code should be changed when the nlist problem is fixed
 * i.e., remove this ifdef, keep the code in the else clause.
 */
#define valid_file(X) (X != 0)
#else
#define valid_file(X) (file_adr <= (unsigned) X &&\
	(unsigned) X < file_adr + v.v_file * sizeof (struct file))
#endif

#define valid_inode(X) (inode_adr <= (unsigned) X &&\
	(unsigned) X < (inode_adr + (v.v_inode * sizeof (struct inode))))

void exit(), perror();
extern char *malloc();
extern int errno;

int gun = 0, usrid = 0;
struct inode ina, inb;
int nsndd;
struct sndd sndd_ina, sndd_inb, *snddp;
int nrcvd;
struct rcvd *rcvdp;
int pregpp;
struct pregion *preg;

struct nlist nl[] = {
	{V_STR},
	{PROC_STR},
	{FILE_STR},
	{INODE_STR},
	{MOUNT_STR},
	{SWPLO_STR},
	{NSNDD_STR},
	{SNDD_STR},
	{NRCVD_STR},
	{RCVD_STR},
	{RFSFST_STR},
	{PREGPP_STR},
	{""}
};

copylval(symbol, ptr)
char *symbol;
unsigned *ptr;
{		/* Copies the lvalue of the UNIX symbol "symbol" into
		 * the variable pointed to by "ptr". The lvalue of
		 * "symbol" is read from SYSTEM.
		 */
	int i = 0;

	while(nl[i].n_name[0] != '\0') {
		if(!strcmp(symbol,nl[i].n_name)) {
			if(nl[i].n_value == 0) {
				return(-1);
			}
			*ptr = nl[i].n_value;
			return(0);
		}
		i++;
	}
	return(1);
}

copyrval(symbol, ptr, size, memdev)
char *symbol, *ptr;
int memdev, size;
{	/* Copies the rvalue of the UNIX symbol "symbol" into the structure
	 * pointed to by "ptr". The rvalue is read from memory at the location
	 * specified by the value of "symbol" in the name list file "nl_file".
	 */
	int mem;
	unsigned lval;

	if(copylval(symbol, &lval)) {
		printf("Copyrval: did not find '%s'\n",symbol);
		exit(1);
	}
	if(memdev >= 0) mem = memdev;
	else if((mem = open(MEMF, O_RDONLY)) == -1)
	{	perror(MEMF);
		exit(1);
	}
#ifdef vax
	lval &= 0x3fffffff;
#endif
	rread(mem, (long) lval, ptr, size);
	if(memdev != mem) close(mem);
}

file_to_inode(file_adr, in, memdev)
int memdev;
struct inode *in;
struct file *file_adr;
{	/* Takes a pointer to one of the system's file entries
	 * and copies the inode  to which it refers into the area
	 * pointed to by "in". "file_adr" is a system pointer, usually
	 * from the user area. */
	int mem;
	struct file f;
	union /* used to convert pointers to numbers */
	{	char	*ptr;
		unsigned addr; /* must be same length as char * */
	} convert;

	convert.ptr = (char *) file_adr;
#ifdef vax
	convert.addr &= 0x3fffffff;
#endif
	if(convert.addr == 0) return(-1);
	if(memdev >= 0) mem = memdev;
	else if((mem = open(MEMF, O_RDONLY)) == -1)
	{	perror(MEMF);
		return(-1);
	}

	/* read the file table entry */
	rread(mem, (long) convert.addr, (char *) &f, sizeof f);
	if(memdev != mem) close(mem);
	if(f.f_flag)
		return(read_inode(f.f_inode, in, memdev));
	else return(-1);
}
struct var v;
struct mount *mnt, *mp;
unsigned proc_adr;
unsigned file_adr, inode_adr, mount_adr;
unsigned nsndd_adr, sndd_adr, nrcvd_adr, rcvd_adr;
unsigned rfstyp_adr, pregpp_adr;
short rfstyp;
unsigned *pofile;
struct proc p;

main(argc, argv)	/* exits with errcnt <= 254 */
int argc;
char **argv;
{
	int mem, newfile = 0, errcnt = 0, tmperr;
	register i, j, k;
	struct user *ublock;

	/* once only code */
	if(argc < 2) {
		fprintf(stderr, "Usage:  %s [-ku] files [-] [[-ku] files]\n", argv[0]);
		exit(1);
	}

	/* open file to access memory */
	if((mem = open(MEMF, O_RDONLY)) == -1) {
		tmperr = errno;
		fprintf(stderr, "%s: ", argv[0]);
		errno = tmperr;
		perror(MEMF);
		exit(1);
	}
	/* get values of system variables */
	if(nlist(SYSTEM, nl) == -1) {
		perror("nlist:");
		return(1);
	}

	/* get values of system variables and address of process table */
	copyrval(V_STR, (char *) &v, sizeof v, mem);
	ublock = (user_t *)malloc(sizeof(user_t) + sizeof(int) * v.v_nofiles);
	if(copylval(PROC_STR, &proc_adr)) {
		printf("Copyrval: did not find '%s'\n",PROC_STR);
		exit(1);
	}
#ifdef vax
	proc_adr &= 0x3fffffff;
#endif
	if(copylval(FILE_STR, &file_adr)) {
		printf("Copyrval: did not find '%s'\n",FILE_STR);
		exit(1);
	}
	if(copylval(INODE_STR, &inode_adr)) {
		printf("Copyrval: did not find '%s'\n",INODE_STR);
		exit(1);
	}
	/* space for mount table */

	if((copylval(MOUNT_STR, &mount_adr))) 
		mnt = 0;
	else
		mnt = (struct mount *)malloc(v.v_mount * sizeof(struct mount));
	if(mnt) 
		rread(mem, mount_adr, mnt, v.v_mount * sizeof(struct mount));

/* RFS */
	/* space for send descriptors */

	if((copylval(NSNDD_STR, &nsndd_adr )) 
	|| (copylval(SNDD_STR, &sndd_adr))) 
		snddp = 0;
	else {
		rread(mem, nsndd_adr, &nsndd, sizeof(nsndd));
		snddp = (struct sndd *)malloc(nsndd * sizeof(struct sndd));
	}
	if(snddp) 
		rread(mem, sndd_adr, snddp, nsndd * sizeof(struct sndd));

	/* space for receive descriptors */

	if((copylval(NRCVD_STR, &nrcvd_adr )) 
	|| (copylval(RCVD_STR, &rcvd_adr))) 
		rcvdp = 0;
	else {
		rread(mem, nrcvd_adr, &nrcvd, sizeof(nrcvd));
		rcvdp = (struct rcvd *)malloc(nrcvd * sizeof(struct rcvd));
	}
	if (rcvdp)
		rread(mem, rcvd_adr, rcvdp, nrcvd * sizeof(struct rcvd));

	/* space for pregion list */

	if(copylval(PREGPP_STR, &pregpp_adr )) 
		preg = 0;
	else {
		rread(mem, pregpp_adr, &pregpp, sizeof(pregpp));
		preg = (struct pregion *)malloc(pregpp * sizeof(struct pregion));
	}

	if(copylval(RFSFST_STR, &rfstyp_adr ))
		rfstyp = -1; 
	else {
		rread(mem, rfstyp_adr, &rfstyp, sizeof(rfstyp));
		if (rfstyp == 0)
			rfstyp = -1;
	}

	/* for each argunent on the command line */
	for(i = 1; i < argc; i++) {
		if(argv[i][0] == '-') {
			/* options processing */
			if(newfile) {
				gun = 0;
				usrid = 0;
			}
			newfile = 0;
			for(j = 1; argv[i][j] != '\0'; j++)
			switch(argv[i][j]) {
			case 'k':
				gun++; 
				break;
			case 'u':
				usrid++; 
				break;
			default:
				fprintf(stderr,
					"Illegal option %c ignored.\n",
						argv[i][j]);
			}
			continue;
		} else
			newfile = 1;

		fflush(stdout);
	
		/* First print file name on stderr (so stdout (pids) can
		 * be piped to kill) */
		fprintf(stderr, "%s: ", argv[i]);

		/* then convert the path into an inode */
		if(path_to_inode(argv[i], &ina) != 0) {
/* RFS */
			if(rsrc_to_inode(argv[i], &ina, mem) != -1) {
				rread(mem, (struct sndd *)(ina.i_fsptr),
					&sndd_ina, sizeof(struct sndd));
			}
			else {
				fprintf(stderr," not found\n");
				errcnt++;
				continue;
			}
		}
		/* then for each process */
		for(j = 0; j < v.v_proc; j++) {
			/* read in the per-process info */
			rread(mem, (long) (proc_adr + j * sizeof p),
					(char *) &p, sizeof p);
			if(p.p_stat == 0 
			|| p.p_stat == SZOMB 
			|| p.p_stat == SIDL) {
				continue;
			}
			/* and the user area */
			read_user(&p, ublock, mem, j);
			pofile = (unsigned *)((unsigned)ublock 
				+ ((unsigned)(ublock->u_pofile) & 0xffff));

			if(valid_inode(ublock->u_cdir) 
			&& read_inode(ublock->u_cdir, &inb, mem) == 0) {
				if(ckuse(mem, "c") == -1) {
					errcnt++;
					continue;
				}
			}
	
	/* This code is only valid in transient cases and the inodes
	 * (pointed to by u_pdir) are not cleared after their use.
	*/
	/*		if(valid_inode(ublock->u_pdir) 
	/*		&& read_inode(ublock->u_pdir, &inb, mem) == 0)
	/*			if(ckuse(mem, "p") == -1) {
	/*				errcnt++;
	/*				continue;
	/*			}
	*/
	
			if(valid_inode(ublock->u_rdir) 
			&& read_inode(ublock->u_rdir, &inb, mem) == 0)
				if(ckuse(mem, "r") == -1) {
					errcnt++;
					continue;
				}

			/* then, for each file */
			for(k = 0; k < v.v_nofiles; k++) {
				/* is it is the fs being checked? */

				if(!valid_file(ublock->u_ofile[k])) 
					continue;
				if(file_to_inode(ublock->u_ofile[k], &inb, mem))
					continue;
				if(ckuse(mem, strncmp(ublock->u_comm,"server",6)
						== 0 ? "S" :  "") == -1) {
					errcnt++;
					break;
				}
			}

/* RFS */
			/* see if process is sleeping on a receive descriptor
			   using this resource */

			if(rcvdp) 
				if(ckrcvd(mem) == -1) {
					errcnt++;
					continue;
				}

			/* check for remote text files */
			if(preg)  
				if(cktxt(mem) == -1) {
					errcnt++;
					continue;
				}
		}
		/* scan receive descriptors for:
			1. pointers to the file table (open files)
			2. pointers to inodes (cd, mount, etc.)
	
		in either case, get the inode and 'chkuse()'
		The user of this descriptor is remote, can't be killed */

		gun = 0;	/* don't want to kill the last process
				  found above */
			/* for each receive descriptor entry */
		if(rcvdp) 
		for(k = 0; k < nrcvd; k++) {
			
			if(rcvdp[k].rd_stat & RDUNUSED) {
				continue;
			}
			if(valid_inode(rcvdp[k].rd_inode)
			&& (read_inode(rcvdp[k].rd_inode, &inb, mem))) {
				if(ckuse(mem, "U") == -1) {
					errcnt++;
					continue;
				}
		}
		}
		fprintf(stderr,"\n");
	}

	/* newfile is set when a file is found.  if it isn't set here,
	 * then the user did not use correct syntax  */
	if (! newfile) {
		fprintf(stderr, "fuser: missing file name\n");
		fprintf(stderr, "Usage: %s [-ku] files [-] [[-ku] files]\n", argv[0]);
		errcnt=(errcnt ? errcnt : 1);
	}
	if(errcnt > 254)
		errcnt = 254;	/* return is unsigned char */
	exit(errcnt);
}

rsrc_to_inode(name, ino, mem)
char *name;
struct inode *ino;
{
	int i;

			/* search kernel mount table for the resource name. */
	if(mnt)
		for(i = 0, mp = mnt; i < v.v_mount; i++, mp++)
			if(mp->m_flags & MINUSE)
			if(nmck(name, mp->m_name, mem))
				if(read_inode(mp->m_mount, ino, mem) != -1)
					return(0);
	return(-1);	/* resource not found in mount table */
}

nmck(n1,addr,mem)
char *n1;
{
	char *nm, kname[NMSZ];

	rread(mem, addr, kname, NMSZ);
	if(!strncmp(n1, kname, NMSZ))
		return(1);
	return(0);
}

ckrcvd(mem)
{
	int k;

		/* refresh receive descriptor list */
	rread(mem, rcvd_adr, rcvdp, nrcvd * sizeof(struct rcvd));

		/* determine if process is sleeping on a receive descriptor */
	k = ((unsigned)p.p_wchan - rcvd_adr)/sizeof(struct rcvd);
	if((k >= 0) && (k < nrcvd)){
		if(!(rcvdp[k].rd_stat & RDUNUSED)
		&&  (rcvdp[k].rd_qtype & SPECIFIC)) {
			rread(mem, rcvdp[k].rd_inode, &sndd_inb, 
							sizeof(struct sndd));
			if((sndd_ina.sd_queue == sndd_inb.sd_queue)
			&& (sndd_ina.sd_mntindx == sndd_inb.sd_mntindx))
				return(report("s"));
		} /* else ???. Should there be a WARNING here?  The process
		     should not be sleeping on an unused receive descriptor */
	}
	return(0);
}
cktxt(mem)
{
	int k, ret;
	struct region regn;

	rread(mem, p.p_region, preg, pregpp * sizeof(struct pregion));
	for(k = 0; k < pregpp; k++) {
		if(preg[k].p_type & (PT_TEXT | PT_LIBTXT)) {
			rread(mem, preg[k].p_reg, &regn, sizeof(struct region));
			if(!read_inode(regn.r_iptr, &inb, mem))
				if((ret = ckuse(mem, "t")) != 0)
					return(ret);
		}
	}
	return(0);
}

ckuse(mem, fc)
char *fc;
{
	/* determine if the inodes in question are local or remote */

			/* if both are remote */
	if(ina.i_fstyp == rfstyp) {
		if(inb.i_fstyp == rfstyp) {
			rread(mem, (struct sndd *)(inb.i_fsptr),
				&sndd_inb, sizeof(struct sndd));
			if((sndd_ina.sd_queue == sndd_inb.sd_queue)
			&& (sndd_ina.sd_mntindx == sndd_inb.sd_mntindx))
				return(report(fc));
		} 
		return(0);
	}
	else 
		if(inb.i_fstyp == rfstyp)
			return(0);

	if(IEQ(ina, inb)) {
		return(report(fc));
	}
	return(0);
}

report(fc)
char *fc;
{
	fprintf(stdout, " %7d", (int) p.p_pid);
	fflush(stdout);
	switch(fc[0]) {
	case 'c':	/* current dir */
	case 'p':	/* parent dir */
	case 'r':	/* root dir */
	case 't':	/* text file busy */
	case 's':	/* sleeping on receive descriptor */
	case 'S':	/* Server */
		fprintf(stderr,"%c", fc[0]);
	}
	if(usrid) 
		puname();
	if((fc[0] != 'S') && (gun)) {
		if(kill((int) p.p_pid, 9))
			return(-1);
	}
	return(1);
}

path_to_inode(path, in)
char *path;
struct inode *in;
{	/* Converts a path to inode info by the stat system call */

	struct stat s;

	if(stat(path, &s) == -1) {
/*			don't want to do this cuz it's normal for resources 
		perror("stat"); */
		return(-1);
	}
	in->i_ftype = s.st_mode;
	in->i_dev = s.st_dev;
	in->i_number = s.st_ino;
	in->i_rdev = s.st_rdev;
	return(0);
}

puname()
{	struct passwd *getpwuid(), *pid;

	pid = getpwuid(p.p_uid);
	if(pid == NULL) return;
	fprintf(stderr, "(%s)", pid->pw_name);
}

read_inode(inode_adr, i, memdev)
int memdev;
struct inode *i, *inode_adr;
{	/* Takes a pointer to one of the system's inode entries
	 * and reads the inode into the structure pointed to by "i" */

	int mem;
	union {		/* used to convert pointers to numbers */
		char	*ptr;
		unsigned addr; /* must be same length as char * */
	} convert;

	convert.ptr = (char *) inode_adr;
#ifdef vax
	convert.addr &= 0x3fffffff;
#endif
	if(convert.addr == 0) return(-1);
	if(memdev >= 0) mem = memdev;
	else if((mem = open(MEMF, O_RDONLY)) == -1) {
		perror(MEMF);
		return(-1);
	}

	/* read the inode */
	rread(mem, (long) convert.addr, (char *) i, sizeof (struct inode));
	if(memdev != mem) close(mem);
#if vax || u3b || u3b15
	i->i_dev = brdev(i->i_dev);	/* removes bits to distinguish small and large FS */
	i->i_rdev = brdev(i->i_rdev);	/* removes bits to distinguish small and large FS */
#endif
	if(i->i_count == 0)
		return(-1);
	return(0);
}

read_user(pr, ub, memdev, pndx)
int memdev, pndx;
struct proc *pr;
struct user *ub;
{	/* Copies the system's user area (in memory) pointed
	 * to by "pr" into the structure pointed to by "u"
	 */

	int i, x;
	static int mdev = -1;
	static daddr_t	swplo = (daddr_t) -1;

	if(swplo == (daddr_t) -1)
	  copyrval(SWPLO_STR, (char *) &swplo, sizeof swplo, memdev);

	/* Method depends on the machine */

	/* VAX */
#ifdef vax
	if(sbrpte == -1) {
		copyrval(SBRPTE_STR, (char *)&sbrpte,
		  sizeof sbrpte, memdev);
		sbrpte &= 0x3fffffff;
	}
	if(mdev < 0 && (mdev = open("/dev/mem", O_RDONLY)) == -1) {
		perror("/dev/mem");
		exit(1);
	}
	x = (int) (pr->p_spt + (pr->p_nspt - 1) * 128);
	x = (sbrpte + ((x & 0x3fffffff) >> PSHIFT) * 4) & 0x3fffffff;
	rread(mdev, (long) x, (char *) &spte, sizeof spte);
	rread(mdev, (long) ((spte&0x1fffff) << PSHIFT),
	  (char *) upte, sizeof upte);

	for(i = 0; i < sizeof (*ub); i += PSIZE)
		rread(mdev,
		   (long) ((upte[128-USIZE+i/PSIZE]&0x1fffff) << PSHIFT),
		   ((char *) ub) + i, MIN(PSIZE, sizeof(*ub) - i));
#endif

	/* Paging */
#ifdef u3b
	if((mdev < 0) && ((mdev = open("/dev/mem", O_RDONLY)) == -1)) {
		perror("/dev/mem");
		exit(1);
	}

	for(i = 0; i < sizeof (*ub); i += NBPP)
		rread(mdev, (long)(pr->p_uptbl[i/NBPP] & PG_ADDR),
			((char *) ub) + i, MIN(NBPP, sizeof(*ub) - i));
#endif


	/* Paging */
#ifdef u3b2
	if (sys3b(RDUBLK, pr->p_pid, (char *)ub, sizeof(*ub)+sizeof(int)*v.v_nofiles) < 0) {
		perror("fuser: unable to read U-area");
		exit(1);
	}
#endif

	/* Paging */
#ifdef u3b15
	if((mdev < 0) && ((mdev = open("/dev/mem", O_RDONLY)) == -1)) {
		perror("/dev/mem");
		exit(1);
	}
	rread(mdev, (long) (pr->p_addr << BPCSHIFT)-0x800000, (char *) ub,
		    sizeof (struct user));
#endif

	/* Paging */
#ifdef pdp11
	if(pr->p_flag & SLOAD /* in core */) {
		if((mdev < 0) 
		&& ((mdev = open("/dev/mem", O_RDONLY)) == -1))
		{	perror("/dev/mem");
			exit(1);
		}
		rread(mdev, ctob((long) pr->p_addr), (char *) ub, sizeof *ub);
	} else  {	/* swapped */
		if((sdev < 0) 
		&& ((sdev = open("/dev/swap", O_RDONLY)) == -1)) {
			perror("/dev/swap");
			exit(1);
		}
		rread(sdev, (pr->p_addr + swplo) << 9, (char *) ub, sizeof *ub);
	}
#endif
}

rread(device, position, buffer, count)
char *buffer;
int count, device;
long position;
{	/* Seeks to "position" on device "device" and reads "count"
	 * bytes into "buffer". Zeroes out the buffer on errors.
	 */
	int i, tmperr;
	long lseek();

	if(lseek(device, position, 0) == (long) -1) {
		tmperr = errno;
		fprintf(stderr, "Seek error for file number %d: ", device);
		errno = tmperr;
		perror("");
		for(i = 0; i < count; buffer++, i++) *buffer = '\0';
		return;
	}
	if(read(device, buffer, (unsigned) count) == -1) {
		tmperr = errno;
		fprintf(stderr, "Read error for file number %d: ", device);
		errno = tmperr;
		perror("");
		for(i = 0; i < count; buffer++, i++) *buffer = '\0';
	}
}




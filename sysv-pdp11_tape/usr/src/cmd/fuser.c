/*	@(#)fuser.c	1.4	*/
#include <a.out.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>

#include <sys/param.h>
#include <sys/types.h>

#include <sys/dir.h>
#include <sys/file.h>
#include <sys/inode.h>

#ifdef u3b
#include <sys/page.h>
#include <sys/istk.h>
#include <sys/seg.h>
#endif
#include <sys/proc.h>

#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/user.h>
#include <sys/var.h>

/* definition of memory page size */
#if pdp11 || vax
#define PSIZE	512
#define PSHIFT	9
#endif
#ifdef u3b
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
#ifdef u3b
#define V_STR		"v"
#define PROC_STR	"proc"
#define FILE_STR	"file"
#define INODE_STR	"inode"
#define SWPLO_STR	"swaplow"
#endif

#ifdef vax
#define memf "/dev/kmem"
#else
#define memf "/dev/mem"
#endif

#define system "/unix"

#define ieq( X, Y) ((X.i_dev == Y.i_dev) && (X.i_number == Y.i_number)) || \
	( ((X.i_mode & IFMT) == IFBLK) && (X.i_rdev == Y.i_dev) )
#define min( X, Y)  ( (X < Y) ? X : Y )
#define valid_file(X) (file_adr <= (unsigned) X &&\
	(unsigned) X < file_adr + v.v_file * sizeof (struct file))
#define valid_inode(X) (inode_adr <= (unsigned) X &&\
	(unsigned) X < inode_adr + v.v_inode * sizeof (struct inode))

void exit(), perror();

copylval( nl_file, symbol, ptr)
char *nl_file, *symbol;
unsigned *ptr;
{	/* Copies the lvalue of the UNIX symbol "symbol" into
	 * the variable pointed to by "ptr". The lvalue of
	 * "symbol" is read from the name list file "nl_file".
	 */
	char *strncpy();
	struct nlist nl[2];
	strncpy( nl[0].n_name, symbol, 8);
	strncpy( nl[1].n_name, "", 8);
	nlist( nl_file, nl);
	*ptr = nl[0].n_value;
}

copyrval( nl_file, symbol, ptr, size, memdev)
char *nl_file, *symbol, *ptr;
int memdev, size;
{	/* Copies the rvalue of the UNIX symbol "symbol" into the structure
	 * pointed to by "ptr". The rvalue is read from memory at the location
	 * specified by the value of "symbol" in the name list file "nl_file".
	 */
	int mem;
	unsigned lval;

	copylval( nl_file, symbol, &lval);
	if ( memdev >= 0 ) mem = memdev;
	else if ( (mem = open( memf, O_RDONLY)) == -1 )
	{	perror( memf);
		exit( 1);
	}
#ifdef vax
	lval &= 0x3fffffff;
#endif
	rread( mem, (long) lval, ptr, size);
	if ( memdev != mem ) close( mem);
}

file_to_inode( file_adr, in, memdev)
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
	if ( convert.addr == 0 ) return( -1);
	if ( memdev >= 0 ) mem = memdev;
	else if( (mem = open( memf, O_RDONLY)) == -1 )
	{	perror( memf);
		return( -1);
	}

	/* read the file table entry */
	rread( mem, (long) convert.addr, (char *) &f, sizeof f);
	if ( memdev != mem ) close( mem);
	if ( f.f_flag )
		return( read_inode( f.f_inode, in, memdev));
	else return( -1);
}

main( argc, argv)
int argc;
char **argv;
{	int mem, file = 0, gun = 0, uname = 0;
	register i, j, k;
	struct inode ina, inb;
	struct proc p;
	struct user u;
	struct var v;
	unsigned proc_adr, file_adr, inode_adr;

	/* once only code */

	/* open file to access memory */
	if ( (mem = open( memf, O_RDONLY)) == -1)
	{	fprintf( stderr, "%s: ", argv[0]);
		perror( memf);
		exit( 1);
	}
	/* get values of system variables and address of process table */
	copyrval( system, V_STR, (char *) &v, sizeof v, mem);
	copylval( system, PROC_STR, &proc_adr);
	copylval( system, FILE_STR, &file_adr);
	copylval( system, INODE_STR, &inode_adr);
#ifdef vax
	proc_adr &= 0x3fffffff;
#endif

	if ( argc < 2 )
	{	fprintf( stderr, "Usage:  %s [-ku] [file1 . . .]\n", argv[0]);
		exit( 1);
	}

	/* For each of the arguments in the command string */
	for ( i = 1; i < argc; i++)
	{	if ( argv[i][0] == '-' )
		{	/* options processing */
			if ( file )
			{	gun = 0;
				uname = 0;
			}
			file = 0;
			for ( j = 1; argv[i][j] != '\0'; j++)
			switch ( argv[i][j])
			{	case 'k':	++gun; break;
				case 'u':	++uname; break;
				default:
					fprintf( stderr,
						"Illegal option %c ignored.\n",
						argv[i][j]);
			}
			continue;
		} else file = 1;

	
		/* First print its name on stderr (so the output can
		 * be piped to kill) */
		fprintf( stderr, "%s: ", argv[i]);
		/* then convert the path into an inode */
		if ( path_to_inode( argv[i], &ina) == -1 ) continue;

		/* then for each process */
		for ( j = 0; j < v.v_proc; j++)
		{	/* read in the per-process info */
			rread( mem, (long) (proc_adr + j * sizeof p),
					(char *) &p, sizeof p);
			if ( p.p_stat == 0 || p.p_stat == SZOMB ) continue;
			/* and the user area */
			read_user( p, &u, mem);

			if ( valid_inode( u.u_cdir) &&
				read_inode( u.u_cdir, &inb, mem) == 0 )
				if ( ieq( ina, inb) )
				{	fprintf( stdout, " %7d", (int) p.p_pid);
					fflush(stdout);
					fprintf( stderr, "c");
					if (uname) puname( (int) p.p_uid);
					if (gun) kill( (int) p.p_pid, 9);
					continue;
				}

/* This code is only valid in transient cases and the inodes
 * (pointed to by u_pdir) are not cleared after their use.
 */
/*			if ( valid_inode( u.u_pdir) &&
/*				read_inode( u.u_pdir, &inb, mem) == 0 )
/*				if ( ieq( ina, inb) )
/*				{	fprintf( stdout, " %7d", (int) p.p_pid);
/*					fflush(stdout);
/*					fprintf( stderr, "p");
/*					if (uname) puname( (int) p.p_uid);
/*					if (gun) kill( (int) p.p_pid, 9);
/*					continue;
/*				}
 */

			if ( valid_inode( u.u_rdir) &&
				read_inode( u.u_rdir, &inb, mem) == 0 )
				if ( ieq( ina, inb) )
				{	fprintf( stdout, " %7d", (int) p.p_pid);
					fflush(stdout);
					fprintf( stderr, "r");
					if (uname) puname( (int) p.p_uid);
					if (gun) kill( (int) p.p_pid, 9);
					continue;
				}

			/* then, for each file */
			for ( k = 0; k < NOFILE; k++)
			{	/* check if it is the fs being checked */
				if ( ! valid_file( u.u_ofile[k]) ) continue;
				if (file_to_inode( u.u_ofile[k], &inb, mem))
					continue;
				if ( ieq( ina, inb) )
				{	fprintf(stdout, " %7d", (int) p.p_pid);
					fflush(stdout);
					if (uname) puname( (int) p.p_uid);
					if (gun) kill( (int) p.p_pid, 9);
					break;
				}
			}
		}

		fprintf( stderr, "\n");
	}
	printf( "\n");
}

path_to_inode( path, in)
char *path;
struct inode *in;
{	/* Converts a path to inode info by the stat system call */

	struct stat s;

	if ( stat( path, &s) == -1 )
	{	perror( "");
		return( -1);
	}
	in->i_mode = s.st_mode;
	in->i_dev = s.st_dev;
	in->i_number = s.st_ino;
	in->i_rdev = s.st_rdev;
	return( 0);
}

puname( uid)
int uid;
{	struct passwd *getpwuid(), *p;

	p = getpwuid( uid);
	if ( p == NULL ) return;
	fprintf( stderr, "(%s)", p->pw_name);
}

read_inode( inode_adr, i, memdev)
int memdev;
struct inode *i, *inode_adr;
{	/* Takes a pointer to one of the system's inode entries
	 * and reads the inode into the structure pointed to by "i" */

	int mem;
	union /* used to convert pointers to numbers */
	{	char	*ptr;
		unsigned addr; /* must be same length as char * */
	} convert;

	convert.ptr = (char *) inode_adr;
#ifdef vax
	convert.addr &= 0x3fffffff;
#endif
	if ( convert.addr == 0 ) return( -1);
	if ( memdev >= 0 ) mem = memdev;
	else if( (mem = open( memf, O_RDONLY)) == -1 )
	{	perror( memf);
		return( -1);
	}

	/* read the inode */
	rread( mem, (long) convert.addr, (char *) i, sizeof (struct inode));
	if ( memdev != mem ) close( mem);
#if vax || u3b
	i->i_dev = brdev(i->i_dev);	/* removes bits to distinguish small and large FS */
	i->i_rdev = brdev(i->i_rdev);	/* removes bits to distinguish small and large FS */
#endif
	if(i->i_count == 0)
		return(-1);
	return( 0);
}

read_user( p, u, memdev)
int memdev;
struct proc p;
struct user *u;
{	/* Copies the system's user area (in memory) pointed
	 * to by "p" into the structure pointed to by "u"
	 */

	int i, x, spte, upte[128];
	static int sdev = -1, mdev = -1, sbrpte = -1;
	static daddr_t	swplo = (daddr_t) -1;
	if ( swplo == (daddr_t) -1 )
	  copyrval( system, SWPLO_STR, (char *) &swplo, sizeof swplo, memdev);

	/* Method depends on the machine */

	/* VAX */
#ifdef vax
	if ( sbrpte == -1 )
	{
	  copyrval( system, SBRPTE_STR, (char *) &sbrpte, sizeof sbrpte, memdev);
	  sbrpte &= 0x3fffffff;
	}
#endif

#if vax || u3b
#ifdef vax
	if ( (p.p_flag & (SLOAD | SSPART)) == 0 /* swapped */ )
#endif
#ifdef u3b
	if ( (p.p_flag & SLOAD) == 0 /* swapped */ )
#endif
	{	if ( (sdev < 0) &&
			((sdev = open( "/dev/swap", O_RDONLY)) == -1) )
		{	perror( "/dev/swap");
			exit( 1);
		}
		rread( sdev, (p.p_swaddr + p.p_swsize - USIZE + swplo) << PSHIFT,
				(char *) u, sizeof *u);
	} else /* in core */
	{	if ( (mdev < 0) &&
			((mdev = open( "/dev/mem", O_RDONLY)) == -1) )
		{	perror( "/dev/mem");
			exit( 1);
		}
#ifdef vax
		x = (int) (p.p_spt + (p.p_nspt - 1) * 128);
		x = (sbrpte + (( x & 0x3fffffff ) >> PSHIFT ) * 4) & 0x3fffffff;
		rread( mdev, (long) x, (char *) &spte, sizeof spte);
		rread( mdev, (long) (spte << PSHIFT), (char *) upte, sizeof upte);
#endif

		for ( i = 0; i < sizeof (*u); i += PSIZE )
			rread( mdev,
#ifdef vax
				(long) ( upte[128-USIZE+i/PSIZE] << PSHIFT),
#endif
#ifdef u3b
				(long) ( p.p_uptbl[i/PSIZE] & PT_ADDR),
#endif
				((char *) u) + i, min( PSIZE, sizeof(*u) - i));
	}
#endif

	/* PDP-11 */
#ifdef pdp11
	if ( p.p_flag & SLOAD /* in core */)
	{	if ( (mdev < 0) &&
			((mdev = open( "/dev/mem", O_RDONLY)) == -1) )
		{	perror( "/dev/mem");
			exit( 1);
		}
		rread( mdev, ctob((long) p.p_addr), (char *) u, sizeof *u);
	} else /* swapped */
	{	if ( (sdev < 0) &&
			((sdev = open( "/dev/swap", O_RDONLY)) == -1) )
		{	perror( "/dev/swap");
			exit( 1);
		}
		rread( sdev, (p.p_addr + swplo) << 9, (char *) u, sizeof *u);
	}
#endif
}

rread( device, position, buffer, count)
char *buffer;
int count, device;
long position;
{	/* Seeks to "position" on device "device" and reads "count"
	 * bytes into "buffer". Zeroes out the buffer on errors.
	 */
	int i;
	long lseek();

	if ( lseek( device, position, 0) == (long) -1 )
	{	fprintf( stderr, "Seek error for file number %d: ", device);
		perror( "");
		for ( i = 0; i < count; buffer++, i++) *buffer = '\0';
		return;
	}
	if ( read( device, buffer, (unsigned) count) == -1 )
	{	fprintf( stderr, "Read error for file number %d: ", device);
		perror( "");
		for ( i = 0; i < count; buffer++, i++) *buffer = '\0';
	}
}




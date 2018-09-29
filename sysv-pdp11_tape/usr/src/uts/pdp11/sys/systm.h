/* @(#)systm.h	1.1 */
/*
 * Random set of variables used by more than one routine.
 */
extern struct inode *rootdir;	/* pointer to inode of root directory */
extern short cputype;		/* type of cpu = 40, 45, 70, 780 */
extern time_t lbolt;		/* time in HZ since last boot */
extern time_t time;		/* time in sec from 1970 */

extern char runin;		/* scheduling flag */
extern char runout;		/* scheduling flag */
extern char runrun;		/* scheduling flag */
extern char runlock;		/* sched flag used in text & data locking */
extern char curpri;		/* current priority */
extern struct proc *curproc;	/* current proc */
extern struct proc *runq;	/* head of linked list of running processes */
extern unsigned maxmem;		/* actual max memory per process */
extern unsigned physmem;	/* physical memory on this CPU */
extern daddr_t swplo;		/* block number of swap space */
extern	nswap;			/* size of swap space */
extern daddr_t rablock;		/* block to be read ahead */
extern char regloc[];		/* locs of saved user registers (trap.c) */
extern dev_t rootdev;		/* device of the root */
extern dev_t swapdev;		/* swapping device */
extern dev_t pipedev;		/* pipe device */
extern char *panicstr;		/* panic string pointer */
extern	blkacty;		/* active block devices */
extern	pwr_cnt, pwr_act;
extern int (*pwr_clr[])();

dev_t getmdev();
daddr_t	bmap();
struct inode *ialloc();
struct inode *iget();
struct inode *owner();
struct inode *maknode();
struct inode *namei();
struct buf *alloc();
struct buf *getblk();
struct buf *getablk();
struct buf *bread();
struct buf *breada();
struct filsys *getfs();
struct file *getf();
struct file *falloc();
int	uchar();

/*
 * Structure of the system-entry table
 */
extern struct sysent {
	char	sy_narg;		/* total number of arguments */
	char	sy_nrarg;		/* number of args in registers */
	int	(*sy_call)();		/* handler */
} sysent[];

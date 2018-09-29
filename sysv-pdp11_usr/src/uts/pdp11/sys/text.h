/* @(#)text.h	1.1 */
/*
 * Text structure.
 * One allocated per pure procedure on swap device.
 * Manipulated by text.c
 */
struct text
{
	ushort	x_daddr;	/* disk address of segment (relative to swplo) */
	ushort	x_size;		/* size (clicks) */
	ushort	x_caddr;	/* address of text (clicks) */
	struct inode *x_iptr;	/* inode of prototype */
	char	x_count;	/* reference count */
	char	x_ccount;	/* number of loaded references */
	char	x_flag;		/* traced, written flags */
	char	x_lcount;	/* number of locked references */
};

extern struct text text[];

#define	XTRC	01		/* Text may be written, exclusive use */
#define	XWRIT	02		/* Text written into, must swap out */
#define	XLOAD	04		/* Currently being read from file */
#define	XLOCK	010		/* Being swapped in or out */
#define	XWANT	020		/* Wanted for swapping */

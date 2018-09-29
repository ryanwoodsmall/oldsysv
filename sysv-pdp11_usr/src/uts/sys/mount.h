/* @(#)mount.h	1.2 */
/*
 * Mount structure.
 * One allocated on every mount.
 */
struct	mount
{
	int	m_flags;	/* status */
	dev_t	m_dev;		/* device mounted */
	struct inode *m_inodp;	/* pointer to mounted on inode */
	struct buf *m_bufp;	/* buffer for super block */
	struct inode *m_mount;	/* pointer to mount root inode */
};

#define	MFREE	0
#define	MINUSE	1
#define	MINTER	2

extern struct mount mount[];

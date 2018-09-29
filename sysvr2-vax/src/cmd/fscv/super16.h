/* @(#)super16.h	1.1 */
/* structure of a super block on a 16 bit pdp machine */
struct pdp16
{
	unsigned short b16_isize;
	short 	b16_fsize[2];
	short 	b16_nfree;
	short 	b16_free[100];
	short	b16_ninode;
	unsigned short	b16_inode[100];
	char	b16_flock;
	char	b16_ilock;
	char	b16_fmod;
	char	b16_ronly;
	short	b16_time[2];
	short	b16_dinfo[4];
	short	b16_tfree[2];
	unsigned short	b16_tinode;
	char	b16_fname[6];
	char	b16_fpack[6];
};

/* @(#)super32.h	1.1 */
/* structure of a super block on a 32 bit pdp machine */
struct pdp32
{
	unsigned short b32_isize;
	short	fill1;	/*generated for boundary allignment*/
	short 	b32_fsize[2];
	short 	b32_nfree;
	short	fill2;	/*generated for boundary allignment*/
	short 	b32_free[100];
	short	b32_ninode;
	unsigned short	b32_inode[100];
	char	b32_flock;
	char	b32_ilock;
	char	b32_fmod;
	char	b32_ronly;
	short	fill3;	/*generated for boundary allignment*/
	short	b32_time[2];
	short	b32_dinfo[4];
	short	b32_tfree[2];
	unsigned short	b32_tinode;
	char	b32_fname[6];
	char	b32_fpack[6];
};

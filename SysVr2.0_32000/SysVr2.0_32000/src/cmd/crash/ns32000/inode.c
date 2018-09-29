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
/*	@(#)inode.c	1.3	*/
#include	"crash.h"
#include	"sys/inode.h"

prinode(c, md, all)
	register int	c;
	int		md;
	int		all;
{
	struct inode	ibuf;
	register char	ch;
	register int	i;
	daddr_t		daddr;

	if(c == -1)
		return;
	if(c > (int)v.ve_inode) {
		printf("%4d out of range\n", c);
		return;
	}

	if(readmem(&ibuf, (long)(SYM_VALUE(Inode)+c*sizeof ibuf), sizeof ibuf) != sizeof ibuf) {
		printf("read error on inode table\n");
		return;
	}
	if(!all && ibuf.i_count == 0)
		return;
	printf("%4d %3.3o %4.4o %5u %3d %4d %4d %4d %6ld",
		c, major(ibuf.i_dev), minor(ibuf.i_dev), ibuf.i_number,
		ibuf.i_count,
		ibuf.i_nlink, ibuf.i_uid, ibuf.i_gid, ibuf.i_size);
	switch(ibuf.i_mode & IFMT) {
	case IFDIR: ch = 'd'; break;
	case IFCHR: ch = 'c'; break;
	case IFBLK: ch = 'b'; break;
	case IFREG: ch = 'f'; break;
	case IFIFO: ch = 'p'; break;
	case IFMPC: ch = 'C'; break;
	case IFMPB: ch = 'B'; break;
	default:    ch = '-'; break;
	}
	printf(" %c", ch);
	printf("%s%s%s%3o",
		ibuf.i_mode & ISUID ? "u" : "-",
		ibuf.i_mode & ISGID ? "g" : "-",
		ibuf.i_mode & ISVTX ? "v" : "-",
		ibuf.i_mode & 0777);
	printf(((ibuf.i_mode & IFMT) == IFCHR) ||
		((ibuf.i_mode & IFMT) == IFBLK) ||
		((ibuf.i_mode & IFMT) == IFBLK) ?
		" %4.4o %4.4o" : "    -    -",
		major(ibuf.i_rdev), minor(ibuf.i_rdev));
	printf("%s%s%s%s%s%s%s\n",
		ibuf.i_flag & ILOCK ? " lck" : "",
		ibuf.i_flag & IUPD ? " upd" : "",
		ibuf.i_flag & IACC ? " acc" : "",
		ibuf.i_flag & IMOUNT ? " mnt" : "",
		ibuf.i_flag & IWANT ? " wnt" : "",
		ibuf.i_flag & ITEXT ? " txt" : "",
		ibuf.i_flag & ICHG ? " chg" : "");
	if(md == 1 && (((ibuf.i_mode & IFMT) == IFREG) ||
		((ibuf.i_mode & IFMT) == IFDIR))) {
		printf("     addr:");
#ifdef	pdp11
		if(ibuf.i_flag & IADDR) {	/* blk addrs from cache */
			lseek(mem, (long)ibuf.i_addr, 0);
			for(i = 0; i < NADDR; i++) {
				read(mem, &daddr, sizeof(daddr));
				printf(" %3ld", daddr);
			}
		}
		else
#endif
			for(i = 0; i < NADDR; i++)
				printf(" %3ld", ibuf.i_addr[i]);
		printf("\n");
	}
}

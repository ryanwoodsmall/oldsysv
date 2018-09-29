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
/*	@(#)buf.c	1.3	*/
#include	"crash.h"
#include	"sys/inode.h"
#include	"sys/ino.h"
#include	"sys/buf.h"

prbufhdr(c)
	register  int  c;
{
	struct	buf	bbuf;
	register  int  b_flags;

	if(c == -1)
		return;
	if(c >= (v.v_buf + v.v_sabuf)) {
		printf("%3d out of range\n", c);
		return;
	}
	if(readmem(&bbuf, (long)SYM_VALUE(Buf) + c * sizeof bbuf,
		sizeof bbuf) != sizeof bbuf) {
		printf("%3d read error\n");
		return;
	}
	printf("%3d %3o %4o %7ld", c,
		major(bbuf.b_dev)&0377, minor(bbuf.b_dev),
		bbuf.b_blkno);
	b_flags = bbuf.b_flags;
	printf("%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		b_flags & B_WRITE ? " write" : "",
		b_flags & B_READ ? " read" : "",
		b_flags & B_DONE ? " done" : "",
		b_flags & B_ERROR ? "error" : "",
		b_flags & B_BUSY ? " busy" : "",
		b_flags & B_PHYS ? " phys" : "",
#ifndef ns32000
		b_flags & B_MAP ? " map" : "",
#else
		b_flags & B_UBLK ? " ublk" : "",
#endif
		b_flags & B_WANTED ? " wanted" : "",
		b_flags & B_AGE ? " age" : "",
		b_flags & B_ASYNC ? " async" : "",
		b_flags & B_DELWRI ? " delwri" : "",
		b_flags & B_OPEN ? " open" : "",
		b_flags & B_STALE ? " stale" : "");
}

prbuffer(c, sw)
	register  int  c;
	int  sw;
{
#if vax || ns32000
	char	buffer[1024];
#else
	char	buffer[512];
#endif
	struct	buf	bufhdr;
	register  int  *ip, i;
	struct	direct	*dp;
	struct	dinode	*dip;
	char	ch;
	long	_3to4();
	int	bad;
	int	j;
	char	*cp;
	char	*itoa();
	int	file;
	int	*address;
	char	name[20];

	if(c ==  -1)
		return;
	printf("\nBUFFER %d:   ", c);
	if(c >= (v.v_buf + v.v_sabuf)) {
		error("out of range");
		return;
	}
	if(readmem(&bufhdr, (long)SYM_VALUE(Buf) + c * sizeof bufhdr,
		sizeof bufhdr) != sizeof bufhdr) {
		error("buffer header read error");
		return;
	}
#ifdef	pdp11
	if(readmem(buffer, (long)bufhdr.b_paddr, sizeof buffer) != sizeof buffer) {
#endif
#if vax || ns32000
	if(readmem(buffer, (long)bufhdr.b_un.b_addr,
		sizeof buffer) != sizeof buffer) {
#endif
		error("buffer read error");
		return;
	}
	switch(sw) {

	default:
	case NULL:
		error("invalid mode");
		break;

	case DECIMAL:
	case HEX:
	case OCTAL:
		for(i=0, address = (int *)0, ip = (int *)buffer;
			address < (int *)(sizeof buffer);
			i++, address++, ip++) {
			if(((int)address % 020) == 0)
				printf("\n%5.5o:\t", address);
			switch(sw) {
				case DECIMAL:
					printf("%8.8u ", *ip);
					break;
				case HEX:
					printf("%8.8x ", *ip);
					break;
				case OCTAL:
					printf("%8.8o ", *ip);
			}
		}
		printf("\n");
		break;


	case WRITE:
		strcpy(name, "buf.");
		strcat(name, itoa(c));
		if((file = creat(name, 0666)) < 0) {
			error("creat error");
			break;
		}
		if(write(file, buffer, sizeof buffer) != sizeof buffer)
			error("write error");
		else
			printf("file:  %s\n", name);
		close(file);
		break;

	case CHAR:
	case BYTE:
		for(i=0, cp = buffer; cp != &buffer[(sizeof buffer)]; i++, cp++) {
			if(i % (sw == CHAR ? 16 : 8) == 0)
				printf("\n%5.5o:\t", i);
			if(sw == CHAR) putch(*cp);
			else printf(" %4.4o", *cp & 0377);
		}
		printf("\n");
		break;

	case INODE:
		for(i=1, dip = (struct dinode *) buffer; dip != (struct dinode *) buffer[(sizeof buffer)]; i++, dip++) {
			printf("\ni#: %ld  md: ", (bufhdr.b_blkno - 2) *
				INOPB + i);
			switch(dip->di_mode & IFMT) {
			case IFCHR: ch = 'c'; break;
			case IFBLK: ch = 'b'; break;
			case IFDIR: ch = 'd'; break;
			case IFREG: ch = 'f'; break;
			case IFIFO: ch = 'p'; break;
			default:    ch = '-'; break;
			}
			putc(ch, stdout);
			printf("%s%s%s%3o",
				dip->di_mode & ISUID ? "u" : "-",
				dip->di_mode & ISGID ? "g" : "-",
				dip->di_mode & ISVTX ? "t" : "-",
				dip->di_mode & 0777);
			printf("  ln: %u  uid: %u  gid: %u  sz: %ld",
				dip->di_nlink, dip->di_uid,
				dip->di_gid, dip->di_size);
			if((dip->di_mode & IFMT) == IFCHR ||
				(dip->di_mode & IFMT) == IFBLK ||
				(dip->di_mode & IFMT) == IFIFO)
				printf("\nmaj: %d  min: %1.1o\n",
					dip->di_addr[0] & 0377,
					dip->di_addr[1] & 0377);
			else
				for(j = 0; j < NADDR; j++) {
					if(j % 7 == 0)
						putc('\n', stdout);
					printf("a%d: %ld  ", j, 
						_3to4(&dip->di_addr[3 * j]));
				}
			printf("\nat: %s", ctime(&dip->di_atime));
			printf("mt: %s", ctime(&dip->di_mtime));
			printf("ct: %s", ctime(&dip->di_ctime));
		}
		printf("\n");
		break;

	case DIRECT:
		printf("\n");
		for(i=0, dp =(struct direct *)  buffer; dp != (struct direct *) &buffer[(sizeof buffer)]; i++, dp++) {
			bad = 0;
			for(cp = dp->d_name; cp != &dp->d_name[DIRSIZ]; cp++)
				if((*cp < 040 || *cp > 0176) && *cp != '\0')
					bad++;
			printf("d%2d: %5u  ", i, dp->d_ino);
			if(bad) {
				printf("unprintable: ");
				for(cp = dp->d_name; cp != &dp->d_name[DIRSIZ];
					cp++)
					putch(*cp);
			} else
				printf("%.14s", dp->d_name);
			putc('\n', stdout);
		}
		break;

	}
}

char	*
itoa(n)
	register  int  n;
{
	register  int  i;
	static    char d[10];

	d[9] = '\0';
	for(i = 8; i >= 0; --i) {
		d[i] = n % 10 + '0';
		n /= 10;
		if(n == 0)
			break;
	}
	return(&d[i]);
}

putch(c)
	register  char  c;
{
	c &= 0377;
	if(c < 040 || c > 0176) {
		putc('\\', stdout);
		switch (c) {
		case '\0': c = '0'; break;
		case '\t': c = 't'; break;
		case '\n': c = 'n'; break;
		case '\r': c = 'r'; break;
		case '\b': c = 'b'; break;
		default:   c = '?'; break;
		}
	} else
		putc(' ', stdout);
	putc(c, stdout);
	putc(' ', stdout);
}

long
_3to4(ptr)
	register  char  *ptr;
{
	long	lret;
	register  char  *vptr;

	vptr = (char *)&lret;
	*vptr++ = *ptr++;
	*vptr++ = 0;
	*vptr++ = *ptr++;
	*vptr++ = *ptr++;
	return(lret);
}

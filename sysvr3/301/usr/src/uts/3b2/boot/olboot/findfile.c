/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:boot/olboot/findfile.c	10.2"

#include	"sys/param.h"
#include	"sys/types.h"
#include	"sys/dir.h"
#include	"sys/inode.h"
#include	"sys/firmware.h"
#include	"sys/lboot.h"

extern unsigned short rb();


/*
**	findfile - Find inode for boot program.
**
**	This routine searches through the file system for the entry named
**	fname and copies the corresponding inode to Binode.
**
**	Return Codes:
**		success => FOUND returned and Fndinode is set
**		  to the inode found for the file.
**		fail => NOTFOUND returned.
**		   Linode is set to be the inode of the last good directory
**		   in the specified path.
*/



unsigned short
findfile(fname)
char	fname[];	/* file name to be searched for */
{

	struct inode di;		/* intermediate directory inode */
	register struct direct	*dep;	/* ptr to directory entry */
	register struct inode	*dip;	/* ptr to current directory inode */
	register int	i,	/* loop control */
			off,	/* directory file offset */
			found;	/* flag indicating directory search success */
	register char	*sp;	/* ptr to name being searched */
	char		ent[DIRSIZ + 1];/* current path entry wanted */
	int		n;	/* loop control */

	sp = fname;
	dip = &Dinode;

	/* Skip over leading blanks, tabs, and slashes.  All paths are grounded
		at root. */
	while (*sp && (*sp == ' ' || *sp == '\t' || *sp == '/'))
		sp++;

	/* Search through directories until the end is found. */
	while (*sp) {

		/* Verify that we have a directory inode. */
		if ((dip->i_mode & IFMT) != IFDIR) {

			/* Not a directory. */
			PRINTF("%s not a directory.\n", ent);
			Linode.i_mode = 0;
			return(NOTFOUND);
		}
		if (dip->i_nlink == 0) {

			/* Zero link count. */
			PRINTF("Link count = 0 for %s.\n", ent);
			Linode.i_mode = 0;
			return(NOTFOUND);
		}
		if ((dip->i_mode & 0111) == 0) {

			/* Directory entry in search path is not a searchable
				directory. */
			PRINTF("Directory %s not searchable.\n", ent);
			Linode.i_mode = 0;
			return(NOTFOUND);
		}

		/* Fetch current path name part and position sp for next entry. */
		for (i = 0; i < DIRSIZ && *sp && *sp != '/'; ent[i++] = *sp++);
		if (i >= DIRSIZ)
			while (*sp && *sp != '/')
				sp++;
		ent[i] = '\0';
		while (*sp == '/')
			*sp++;

		/* Search current directory for wanted entry. */
		for (off = found = 0; !found && off < dip->i_size; off += FsBSIZE(fstype)) {

			/* Read next directory block. */
			if (!rb(DIR, off, dip)) {
				Linode.i_mode = 0;
				return(NOTFOUND);
			}

			/* Search it. */
			dep = (struct direct *)(DIR);
			for (n = FsBSIZE(fstype) / sizeof(*dep); !found && n--;) {
				if (dep->d_ino == 0) {
					dep++;
					continue;
				}
				for (i = 0; i < DIRSIZ; i++) {
					if (ent[i] != dep->d_name[i])
						break;
					if (ent[i] == '\0')
						i = DIRSIZ;
				}
				if (i >= DIRSIZ)
					found = 1;
				else
					dep++;
			}
		}
		if (!found) {
			Linode = *dip;
			return(NOTFOUND);
		}

		/* Unpack next inode in appropriate place. */
		dip = *sp ? &di : &Fndinode;
		dip->i_number = dep->d_ino;
		liread(dip);
	}
	if ((dip->i_mode & IFMT) == IFDIR && dip->i_nlink) {
		Linode = *dip;
		return(DIRFOUND);
	}
	return(FOUND);
}

/*
**	lls - Little ls.
**
**	This routine prints the names of all allocated files in the directory
**	pointed to by ip.  Names are printed in order of appearance in the
**	directory.
*/

lls(ip)
struct inode	*ip;	/* pointer to directory inode */
{
	char			o[DIRSIZ + 2];	/* output buffer */
	register int		i,		/* loop control */
				n,		/* loop control */
				nf,		/* # of entries found */
				npl;		/* # of entries per line */
	int			off;		/* byte offset in directory */
	register char		*p;		/* ptr into output buffer */
	register struct direct	*dp;		/* ptr to directory entry */

	npl = 80 / (DIRSIZ + 1);
	for (off = 0;off < ip->i_size;off += FsBSIZE(fstype)) {

		/* Read next directory block. */
		if (!rb(DIR, off, ip))
			return;

		/* Print allocated entries. */
		for (n = FsBSIZE(fstype) / sizeof(*dp), dp = (struct direct *)(DIR);
			n--;dp++) {
			if (dp->d_ino == 0)
				continue;
			p = o;
			if ((nf++ % npl) == 0)
				*p++ = '\n';
			for (i = 0;i < DIRSIZ;i++)
				*p++ = dp->d_name[i] ? dp->d_name[i] : ' ';
			*p = '\0';
			PRINTF("%s ", o);
		}
	}
	PRINTF("\n");
}

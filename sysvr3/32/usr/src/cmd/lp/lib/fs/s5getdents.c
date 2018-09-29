/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/fs/s5getdents.c	1.2"

#include "fslib.h"

extern long		lseek();

/**
 ** s5getdents() - SIMULATE "getdents()" FOR SVR2.1 OR EARLIER SYSTEMS
 **/

int			s5getdents (fildes, buf, nbytes)
	int			fildes;
	char			*buf;
	unsigned		nbytes;
{
	register int		n;

	register unsigned	orig_nbytes	= nbytes;

	register struct direct	*s5dirp;

	register struct dirent	*FSSdirp;

	register off_t		offset;

	char			s5buf[DIRBUF];

	/*
	 * Each ``s5'' directory entry will take up no more
	 * than "sizeof(struct direct)" bytes (i.e. 16).
	 * When transfered into a FSS directory entry, this
	 * grows to at most
	 *
	 *	sizeof(long) + sizeof(off_t) + sizeof(ushort) + DIRSIZ + 4
	 *
	 * (Why +4? +1 for the terminating null, and in the worst
	 * case, +3 to round up to the nearest "long" boundary.)
	 * Thus we can calculate the number of ``s5'' entries
	 * that can be transferred to "nbytes" of FSS buffer.
	 * One problem is that some of the ``s5'' entries
	 * may be invalid, so we're not likely to get as many
	 * as we calculate. Another problem is that the above FSS
	 * entry size estimate is the MAXIMUM; actual entries will
	 * be shorter. We'll iterate on this process until
	 * we've filled the FSS buffer with as many as can fit.
	 */

#define	FSS_HD_SZ	(sizeof(long) + sizeof(off_t) + sizeof(ushort))

	/*
	 * Note the loop terminating condition: The first part is the
	 * estimate of the number of ``s5'' entries to read, and is based
	 * on the amount of space available to hold ``s5'' entries in the
	 * FSS buffer. If our estimate says we can't fit any entries,
	 * the loop ends. However, the estimate of the space needed in
	 * the FSS buffer is likely to be too high, so maybe another entry
	 * really would fit. We'd have to read the next ``s5'' entry to be
	 * sure, though, and if it indeed didn't fit, we'd be stuck with
	 * an extra entry.
	 */
	for (
		FSSdirp = (struct dirent *)buf,
		offset = lseek(fildes, 0L, 1);
		(n = nbytes / (FSS_HD_SZ + DIRSIZ + 4)) > 0
	     && (n = Read(fildes, s5buf, n * sizeof(struct direct))) > 0;
		/* "nbytes" is decremented below */
	) {

		/*
		 * Run through the ``s5'' directory entries, copying those
		 * that are valid (inode != 0) into the FSS buffer.
		 */
		for (
			s5dirp = (struct direct *)s5buf;
			n > 0;
			s5dirp++,
				n -= sizeof(struct direct),
					offset += sizeof(struct direct)
		)
			if (s5dirp->d_ino) {

				FSSdirp->d_ino = s5dirp->d_ino;

				FSSdirp->d_off = offset
#if	!defined(GETDENTS_FIXED)
				/*
				 * The SVR3.0 documentation for dirent(4)
				 * says ".d_off" is the ``offset of that
				 * directory entry ....'' Well, actually
				 * it would seem that it is the offset
				 * of the NEXT entry, or what the offset
				 * will be AFTER READING this entry.
				 */
					       + sizeof(struct direct)
#endif
						;

				strncpy (
					FSSdirp->d_name,
					s5dirp->d_name,
					DIRSIZ
				);
				FSSdirp->d_name[DIRSIZ] = 0;

#define	L (sizeof(long))
				FSSdirp->d_reclen = FSS_HD_SZ
					+ strlen(FSSdirp->d_name) + 1;
				FSSdirp->d_reclen = ((FSSdirp->d_reclen + L-1) / L) * L;
#undef	L

				/*
				 * The following will NOT move the
				 * "FSSdirp" pointer outside the
				 * buffer or drop "nbytes" below 0,
				 * because of our earlier calculation.
				 */
				nbytes -= FSSdirp->d_reclen;
				FSSdirp =
		(struct dirent *)((char *)FSSdirp + FSSdirp->d_reclen);

			}

	}

	return (n >= 0? orig_nbytes - nbytes : n);
}

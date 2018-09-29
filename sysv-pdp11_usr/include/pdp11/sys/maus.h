/* @(#)maus.h	1.1 */

/*
 * Common Header file for MAUS routines
 */

#define	MEMDEV	2		/* major dev number of /dev/mem */

struct mausmap {
	int boffset;
	int bsize;
};

extern struct mausmap	mausmap[];
extern	int	mauscore;
extern	int	mausend;
extern	int	nmausent;

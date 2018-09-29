/*	dmk.h 1.2 of 3/18/82
	@(#)dmk.h	1.2
 */

#define	DMK	('V'<<8)
#define DMKSETM	(DMK|13)
#define	DMKDTR	(1<<1)
#define	DMKRTS	(1<<2)
#define	DMKNS	(1<<3)

struct dmkcmd {
	short line, mode;
};

struct dmksave {
	unsigned char status[8];
};

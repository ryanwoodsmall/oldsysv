/* @(#)stermio.h	1.1 */
/*
 * ioctl commands for control channels
 */
#define STSTART		1	/* start protocol */
#define STHALT		2	/* cease protocol */
#define STPRINT		3	/* assign device to printer */

/*
 * ioctl commands for terminal and printer channels
 */
#define STGET	(('X'<<8)|0)	/* get line options */
#define STSET	(('X'<<8)|1)	/* set line options */
#define	STTHROW	(('X'<<8)|2)	/* throw away queued input */
#define	STWLINE	(('X'<<8)|3)	/* get synchronous line # */

struct stio {
	unsigned short	ttyid;
	char		row;
	char		col;
	char		orow;
	char		ocol;
	char		tab;
	char		aid;
	char		ss1;
	char		ss2;
	unsigned short	imode;
	unsigned short	lmode;
	unsigned short	omode;
};

/*
**	Mode Definitions.
*/
#define	STFLUSH	00400	/* FLUSH mode; lmode */
#define	STWRAP	01000	/* WRAP mode; lmode */
#define	STAPPL	02000	/* APPLICATION mode; lmode */

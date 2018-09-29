/*	    @(#)params.h	2.6     */
/***********************************************************************
*   This file contains system dependent parameters for the link editor.
*   There must be a different params.h file for each version of the
*   link editor (e.g. b16, n3b, mac80, etc.)
*
*   THIS COPY IS FOR THE VAX
*********************************************************************/


/*
 * Maximum size of a section
 */
#define MAXSCNSIZE      0x1000000L
#define MAXSCNSZ        MAXSCNSIZE


/*
 * Default size of configured memory
 */
#define MEMORG		0x0L
#define MEMSIZE         0x1000000L
#define NONULLORG	0x2000L

/*
 * Size of a region. If USEREGIONS is zero, the link editor will NOT
 * permit the use of REGIONS, nor partition the address space
 */

#define REGSIZE		0x0L

/*
 * define alignment factor for common symbols
 */

#define COM_ALIGN	0x3L

/*
 * define boundary for use by paging
 */

#define BOUNDARY	0x10000		/* 64K  */

/*
 * alignment size for dfltsec (special.c)
 */
#define ALIGNSIZE 1024

/* special symbols for program entry point */

#define _MAIN	"_main"
#define _CSTART	"start"

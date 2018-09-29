/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/io.h	10.1"

#ifdef INKERNEL
struct	{
	daddr_t	nblocks;	/* number of blocks in disk partition */
	int	cyloff;		/* starting cylinder # of partition */
} id_sizes[16] = {

	/* for seagate drives */
	20448,	21,	/* partition 0 -	cyl 21-305  (root fs) 	*/
	12888,	126,	/*     "     1 -	cyl 126-305 (swap)	*/
	9360,	175,	/*     "     2 -	cyl 175-305 (usr)	*/
	7200,	205,	/*     "     3 -	cyl 205-305 		*/
	3600,	255,	/*     "     4 -	cyl 255-305 		*/
	21816,	3,	/*     "     5 -	cyl 2-305   (init fs)	*/
	21888,	1,	/*     "     6 -	cyl 1-305   (full disk)	*/
	72,	1,	/*     "     7 - 	cyl 1 	    (boot)	*/
	/* for cdc drives */
	59760,	31,	/* partition 0 -	cyl 31-695  (root fs) 	*/
	49860,	141,	/*     "     1 -	cyl 141-695 (swap)	*/
	44730,	198,	/*     "     2 -	cyl 198-695 (usr fs)	*/
	31500,	345,	/*     "     3 -	cyl 345-695 		*/
	15750,	520,	/*     "     4 -	cyl 520-695 		*/
	62370,	2,	/*     "     5 -	cyl 2-695   (init fs)	*/
	62460,	1,	/*     "     6 -	cyl 1-695   (full disk)	*/
	90,	1,	/*     "     7 - 	cyl 1 	    (boot)	*/
};
struct	{
	daddr_t nblocks;	/* number of blocks in disk partition */
	int 	sblock;		/* starting cylinder # of partition */
} if_sizes[8] = {

	990,   24,	/* partition 0 -	cyl 24-78 (root)      */
	810,   34,	/* partition 1 -	cyl 34-78 	      */
	612,   45,	/* partition 2 -	cyl 45-78 	      */
	414,   56,	/* partition 3 -	cyl 56-78 	      */
	216,   67,	/* partition 4 -	cyl 67-78 	      */
	1404,  1,	/* partition 5 -	cyl 1-78  (init)      */
	1422,  0,	/* partition 6 -	cyl 0-78  (full disk) */
	18,    0,	/* partition 7 -	cyl 0     (boot)      */
};
#endif 

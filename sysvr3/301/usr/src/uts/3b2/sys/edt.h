/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/edt.h	10.1"
/************************************************************/
/*    Structures for the Equipped Device Table              */
/************************************************************/

#define EDTSBD 0

#define E_SINGLE  0
#define E_DOUBLE  1

#define E_8BIT  0
#define E_16BIT  1

#define E_DUMB 0
#define E_SMART 1

#define E_NAMLEN  10

#define MAX_IO  12

struct subdevice {
	unsigned short opt_code;	/* sixteen bit option code for	*/
					/* subdevice stored as a short	*/
	unsigned char name[E_NAMLEN];	/* ASCII name of subdevice	*/
};

struct edt {
	unsigned opt_code:16;		/* sixteen bit option code	*/
	unsigned opt_slot:4;		/* slot in which this board is  */
	unsigned opt_num:4;		/* which of given option type	*/
					/* this board is		*/
	unsigned rq_size:8;		/* request queue entry size	*/
	unsigned cq_size:8;		/* completion queue entry size	*/
	unsigned resrvd:14;		/* (reserved for future use)	*/
	unsigned cons_cap:1;		/* one = can support console	*/
					/* zero = cannot support cons   */
	unsigned cons_file:1;		/* zero = device has no pump    */
					/* file for floating console    */
					/* one = device has a floating  */
					/* console pump file            */
	unsigned boot_dev:1;		/* one for possible boot device	*/
	unsigned word_size:1;		/* zero = 8 bit ; one = 16 bit	*/
	unsigned brd_size:1;		/* zero = single width;		*/
					/* one = double width		*/
	unsigned smrt_brd:1;		/* zero = dumb board		*/
					/* one = intelligent board	*/
	unsigned n_subdev:4;		/* subdevice count		*/
	struct subdevice *subdev;	/* pointer to array of n_subdev	*/
					/* subdevice structures		*/
	char dev_name[E_NAMLEN];	/* ASCII name of device		*/
	char diag_file[E_NAMLEN];	/* Name of UN*X resident file   */
					/* containing diagnostic phases	*/
};

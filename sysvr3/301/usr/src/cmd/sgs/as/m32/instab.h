/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:m32/instab.h	1.7"
/*
 */

#define SHFT_MSK 0x1F
#define INST_MSK 0x3f
#define	DUMMY	0x01
#if	FLOAT
#define	INSTRS	0x0e
#define	INSTRD	0x0f
#define	INSTRX	0x10
#endif
#define	INSTRB	0x11
#define	INSTRH	0x12
#define	INSTRW	0x13
#define	INSTRBH	0x15
#define	INSTRBW	0x16
#define	INSTRHW	0x17
#define	INSTRWH	0x18
#define	INSTRWB	0x19
#define	INSTRHB	0x1a
#if	FLOAT
#define	INSTRSD	0x1b
#define	INSTRSX	0x1c
#define	INSTRDS	0x1d
#define	INSTRDX	0x1e
#define	INSTRXS	0x1f
#define	INSTRXD	0x20
#endif

#define CREGMD		0x4	/* register mode */
#define CREGDFMD	0x5	/* register deferred mode */
#define CFPDSPMD	0x6	/* FP displacement mode */
#define CAPDSPMD	0x7	/* AP displacement mode */
#define CDSPMD		0x8	/* displacement mode */
#define CDSPDFMD	0x9	/* displacement deferred mode */
#define CHDSPMD		0xA	/* halfword displacement mode */
#define CHDSPDFMD	0xB	/* halfword displacement deferred mode */
#define CBDSPMD		0xC	/* byte displacement mode */
#define CBDSPDFMD	0xD	/* byte displacement deferred mode */
#define CEXPAND		0xE	/* expand byte */
#define CIMMD		0x4F	/* immediate mode */
#define CHIMMD		0x5F	/* halfword immediate  */
#define CBIMMD		0x6F	/* byte immediate  */
#define CABSMD		0x7F	/* absolute address mode */
#define CABSDFMD	0xEF	/* absolute address deferred mode */

#if	FLOAT
#define	FPIMMD		0x13	/* floating point immediate mode */
#define	DECIMMD		0x14	/* floating point immediate mode */
#define	CDIMMD		0xCF	/* floating point double immediate mode */
#endif

#define FPREG		0x9	/* FP register number */
#define APREG		0xA	/* AP register number */
#define	PSWREG		0xB	/* PSW register number */
#define SPREG		0xC	/* SP register number */
#define	PCREG		0xF	/* PC register number */

#define UBYTE		0x3
#define SBYTE		0x7
#define UHALF		0x2
#define SHALF		0x6
#define UWORD		0x0
#define SWORD		0x4
#define NOTYPE		0xF	/* no new type (i.e. {...}) specified */

#define JMPOPCD 0x24L

#define	NULLSPEC	0x00L
#define BYTESPEC	0x01L
#define	HALFSPEC	0x02L
#define	WORDSPEC	0x03L
#define	LITERALSPEC	0x04L
#define	SHORTSPEC	0x05L

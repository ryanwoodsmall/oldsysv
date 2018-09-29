/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)convert:common/conv.h	2.6"
/*
 */

/*
 *	Definitions used by conversion routines
 */

/*
 *	Define flag representing host architecture
 */

#if AR16WR
#define	F_CURARCH	F_AR16WR
#endif
#if AR32WR
#define	F_CURARCH	F_AR32WR
#endif
#if AR32W
#define	F_CURARCH	F_AR32W
#endif


#define FLAGS(x)	x->header.f_flags
#define IS3B(x)		(x == N3BMAGIC || x == NTVMAGIC)
#define ISM32(x)	(x == FBOMAGIC || x == RBOMAGIC || x == MTVMAGIC)
#define ISB16(x)	(x == B16MAGIC || x == BTVMAGIC)
#define ISX86(x)	(x == X86MAGIC || x == XTVMAGIC)
#define	ISIAPX(x)	(x == IAPX16 || x == IAPX16TV || x == IAPX20 || x == IAPX20TV)
#define ISVAX(x)	(x == VAXROMAGIC || x == VAXWRMAGIC)
#define ISDEC(x)	(x & F_AR32WR  || x & F_AR16WR)
#define	ISMC68(x)	(x == MC68MAGIC || x == MC68TVMAGIC)
#define ISNONDEC(x)	(x & F_AR32W)
#define FLG32W1	"n3b"
#define FLG32W2	"m32"
#define FLG32W3	"ibm"
#define	FLG32W4	"mc68"
#define FLG16W1	"i86"
#define FLG16W2 "b16"
#define FLG16W3 "x86"
#define FLG32WR "vax"
#define FLG16WR "pdp"
#define IS32W(x)	(equal(x,FLG32W1,3)||equal(x,FLG32W2,3)||equal(x,FLG32W3,3)||equal(x,FLG32W4,4))
#define IS16W(x)	(equal(x,FLG16W1,3)||equal(x,FLG16W2,3)||equal(x,FLG16W3,3))
#define IS32WR(x)	(equal(x,FLG32WR,3))
#define IS16WR(x)	(equal(x,FLG16WR,3))

#define MAXFILES 200

union general
{
	char		c[4];
	short		s[2];
	unsigned short	us[2];
	long		l;
	int		i;
};

#define GEN union general

#define ANYSOURCE(x)	((F_AR16WR & x)||(F_AR32WR & x)||(F_AR32W & x))
#define ANYMAGIC(x)	(IS3B(x)||ISM32(x)||ISB16(x)||ISX86(x)||ISVAX(x)||ISMC68(x)||ISIAPX(x))

/*
 * Define standard a.out magic numbers for conversion program
 */

#define CNMAGIC		0410
#define COMAGIC		0407
#define	CPMAGIC		0413

/*
 * Conversion types for converting object files built on other machines
 */

#define	PDPVAX	1
#define PDPIBM	2
#define VAXIBM	3

/*
 * old (pre 5.0), random (5.0), or ascii (6.0) archive formats
 */
#define ASCIIAR	2
#define RANDOM	1
#define OLD 0

/*
 * used to set the target archive format equal to the current archive format
 */
#define CUR -1

/*
 * Archive magic numbers
 */
#define ASCII_MAGIC	"!<arch>\n"
#define RAND_MAGIC	"<ar>"
#ifndef ARFMAG
#define ARFMAG		"`\n"
#endif
#define AMAGSIZE	8
#define RMAGSIZE	4
#define ARFMAGSIZE	2

/*
 * archive headers for all three formats
 * convert has to know the structure of headers for all formats in
 * order to convert between them
 */
typedef struct {
	char	ar_name[14];
	long	ar_date;
	int	ar_uid;
	int	ar_gid;
	long	ar_mode;
	long	ar_size;
	} NEWARHDR;		/* information contained in all three headers */

typedef struct {
	char	arf_name[16];
	char	arf_date[4];
	char	arf_uid[4];
	char	arf_gid[4];
	char	arf_mode[4];
	char	arf_size[4];
	} RANDARF;		/* 5.0 archive file header */

typedef struct {
	char	ar_magic[4];
	char	ar_name[16];
	char	ar_date[4];
	char	ar_syms[4];
	} RANDARCH;		/* 5.0 archive header */

typedef struct {
	char	ar_name[16];
	char	ar_date[12];
	char	ar_uid[6];
	char	ar_gid[6];
	char	ar_mode[8];
	char	ar_size[10];
	char	ar_fmag[2];
	} PORT_HEAD;		/* 6.0 archive header */

/*
 * The a.out header is 8 bytes longer on the simplex than on all
 * other machines
 */

#if U3B
#define	AOUTSIZE	36
#else
#define AOUTSIZE	28
#endif

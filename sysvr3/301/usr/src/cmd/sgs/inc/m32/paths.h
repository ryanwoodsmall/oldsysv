/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)xenv:m32/paths.h	1.12"
/*
 *
 *
 *	Pathnames for m32/m32mau
 */

/*
 *	Directory containing libraries and executable files
 *	(e.g. assembler pass 1)
 */
#define LIBDIR	"/lib"
#define LLIBDIR1 "/usr/lib"
#define NDELDIRS 2

/*
 *	Directory containing executable ("bin") files
 */
#define BINDIR	"/bin"

/*
 *	Directory containing include ("header") files for building tools
 */
#define INCDIR	"/tmp"

/*
 *	Directory for "temp"  files
 */
#define TMPDIR	"/tmp"

/*
 *	Default name of output object file
 */
#define A_OUT	"a.out"

/*
 *	The following pathnames will be used by the "cc" command
 *
 *	m32/m32mau cross compiler
 */
#define CPP	"/lib/cpp"
/*
 *	Directory containing include ("header") files for users' use
 */
#define INCLDIR	"-I/tmp"
#define COMP	"/lib/comp"
#define C0	"/lib/front"
#define C1	"/lib/back"
#define OPTIM	"/lib/optim"
/*
 *	m32/m32mau cross assembler
 */
#define AS	"/bin/as"
#define AS1	"/lib/as1"	/* assembler pass 1 */
#define AS2	"/lib/as2"	/* assembler pass 2 */
#define M4	"/usr/bin/m4"			/* macro preprocessor */
#define CM4DEFS	"/lib/cm4defs"	/* C interface macros */
#define CM4TVDEFS "/lib/cm4tvdefs"	/* C macros with 'tv' call */
/*
 *	m32/m32mau link editor
 */
#define LD	"/bin/ld"
#define LD2	"/lib/ld2"	/* link editor pass 2 */

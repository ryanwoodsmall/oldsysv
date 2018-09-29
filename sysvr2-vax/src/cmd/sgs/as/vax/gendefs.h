/*	@(#)gendefs.h	2.2	*/
#define  NO       0
#define  YES      1

#define NCPS	8	/* number of characters per symbol */
#define BITSPBY	8
#define BITSPW	32

#define NBPW	32
#define	FILL	0L
#define	TXTFILL	0L

#define	NULLVAL 0L
#define NULLSYM	((symbol *)NULL)

#define SCTALIGN 4L /* byte alignment for sections */

/* constants used in testing and flag parsing */

#define	TESTVAL -2
#define NFILES 8


/* code generation actions for the second pass */

#define NACTION 45

#define GENVALUE 0
#define GENRELOC 1
#define GENPCREL 2
#define SDIDST	 3
#define SDIINST	 4 /* assumes 4..28 for each of the 25 sdi's */
#define NEWSTMT 29
#define SETFILE 30
#define LINENBR 31
#define LINENUM 32
#define LINEVAL 33
#define DEFINE  34
#define ENDEF   35
#define SETVAL  36
#define SETSCL  37
#define SETTYP  38
#define SETTAG  39
#define SETLNO  40
#define SETSIZ  41
#define SETDIM1 42
#define SETDIM2 43
#define GENDISP 44   /* simple summation of a symbol value for displacement */
#define GENABSSYM 45 /* generate an absolute difference of symbols */

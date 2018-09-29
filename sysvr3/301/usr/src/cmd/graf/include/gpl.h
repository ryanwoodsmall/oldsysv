/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:include/gpl.h	1.5"
#define TEXLEN     256
#define MAXLINES   256
#define LINES      0
#define MOVE       1
#define TEXT       2
#define ARCS       3
#define ALPHA      4
#define COMMENT    15
#define LIMIT      32768

/*
/*  A comment on the ordering of fields within the following unions:
/*	Due to the fact that the VAX and PDP-11 allocate fields
/*	(bytes) in a word from right to left, and the 370 from left to
/*	right, the order in which names are declared within these structures
/*	must be inverted to reproduce  the states encountered on the
/*	DEC machines.  Also, dummy halfword fields are required in 
/*	'cmdword' to force the fields 'cmbyte2' and 'cmbyte1' into
/*	the proper bytes in the word allocated for the union.
*/

#if u370 | u3b | u3b5 | u3b2

union cmdword{
	struct { int pntw;
	} cma;
	struct {
		short cmbdummy;
		char cmbyte2, cmbyte1;
	} cmb;
	struct fbits {
		short cmcdummy;
		unsigned cn  : 12; /*  count of words in cmd  */
		unsigned cm  : 4;  /*  command  */
	} cmc;
};
union styleword{
	struct { char stbyte2,stbyte1;
	} sta;
	struct sbits{
		unsigned st   : 4;  /*  style  */
		unsigned wt   : 4;  /*  weight  */
		unsigned col  : 8;  /*  color  */
	} stb;
	struct sbits1{
		unsigned  fon : 8;  /*  font   */
		unsigned  col : 8;  /*  color  */
	} stc;
};
union szorient{
	struct { char szbyte2,szbyte1;
	} sza;
	struct tbits{
		unsigned tr : 8; /*  text rotation  */
		unsigned ts : 8; /*  text size  */
	} szb;
};

#else

union cmdword{
	struct { int pntw;
	} cma;
	struct { char cmbyte1,cmbyte2;
	} cmb;
	struct fbits {
		unsigned cm  : 4;  /*  command  */
		unsigned cn  : 12; /*  count of words in cmd  */
	} cmc;
};
union styleword{
	struct { char stbyte1,stbyte2;
	} sta;
	struct sbits{
		unsigned col  : 8;  /*  color  */
		unsigned wt   : 4;  /*  weight  */
		unsigned st   : 4;  /*  style  */
	} stb;
	struct sbits1{
		unsigned  col : 8;  /*  color  */
		unsigned  fon : 8;  /*  font   */
	} stc;
};
union szorient{
	struct { char szbyte1,szbyte2;
	} sza;
	struct tbits{
		unsigned ts : 8; /*  text size  */
		unsigned tr : 8; /*  text rotation  */
	} szb;
};

#endif

struct command {
	int cmd,cnt,color,weight,style,font,trot,tsiz;
	int *aptr,array[MAXLINES * 2];
	char *tptr,text[TEXLEN];
};

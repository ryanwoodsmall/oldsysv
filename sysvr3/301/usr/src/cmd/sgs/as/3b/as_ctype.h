/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:3b/as_ctype.h	1.3"
/*
 */

#define WS  0x01	/* permissable white space character */
#define IDB 0x02	/* permissable as id first character */
#define IDF 0x04	/* permissable in id after first character */
#define OPC 0x08	/* permissable embedded in operand */
#define HOPC 0x10	/* non-whitespace permissable embedded in operand */
#define OCT 0x20	/* permissable octal number character */
#define DEC 0x40	/* permissable decimal number character */
#define HEX 0x80	/* permissable hex number character */

extern unsigned char as_ctype[];

#define WHITESPACE(c)	((as_ctype+1)[c] & WS)
#define IDBEGIN(c)	((as_ctype+1)[c] & IDB)
#define IDFOLLOW(c)	((as_ctype+1)[c] & IDF)
#define OPRCHAR(c)	((as_ctype+1)[c] & OPC)
/* HOPRCHAR(c) is true if a non-whitespace operand character */
#define HOPRCHAR(c)	((as_ctype+1)[c] & HOPC)
/* SINGCHAR(c) is true if c isn't combined with other chars to form a token */
#define SINGCHAR(c)	(((as_ctype+1)[c] & IDF) ^ IDF)
#define OCTCHAR(c)	((as_ctype+1)[c] & OCT)
#define DECCHAR(c)	((as_ctype+1)[c] & DEC)
#define HEXCHAR(c)	((as_ctype+1)[c] & HEX)

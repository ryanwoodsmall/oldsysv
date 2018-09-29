/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:boot/lboot/clibrary.c	10.2"

/*
 * This file contains the following routines from the C library:
 *
 *		char   *malloc()	<== rewritten for lboot
 *		void    free()		<== rewritten for lboot
 *		char   *strcat()
 *		int     strcmp()	<== u3b2 only
 *		int     strcspn()
 *		char   *strncat()
 *		int     strncmp()	<== u3b2 only
 *		char   *strncpy()
 *		char   *strpbrk()
 *		int     strspn()
 *		char   *strtok()
 *		long    strtol()
 *
 *		int     fdclose();	<== modified for lboot
 *		FILE   *fdopen();	<== modified for lboot
 *		char   *fgets();
 *		int     _filbuf();	<== modified for lboot
 *		int     fread();
 *		int     fseek();	<== modified for lboot
 *
 *		char   *itoa();		<== new routine (not from C library)
 *		char   *lcase();	<== new routine (not from C library)
 */

#include <sys/types.h>
#include <sys/dir.h>
#include "lboot.h"
#include <sys/param.h>
#include <sys/errno.h>
#include "error.h"
#include "io.h"



/*
 * C library error number
 */
int errno;

#include <ctype.h>

char	_ctype[] = { 0,

/*	 0	 1	 2	 3	 4	 5	 6	 7  */

/* 0*/	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
/* 10*/	_C,	_S|_C,	_S|_C,	_S|_C,	_S|_C,	_S|_C,	_C,	_C,
/* 20*/	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
/* 30*/	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
/* 40*/	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P,
/* 50*/	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P,
/* 60*/	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,
/* 70*/	_N|_X,	_N|_X,	_P,	_P,	_P,	_P,	_P,	_P,
/*100*/	_P,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U,
/*110*/	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U,
/*120*/	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U,
/*130*/	_U,	_U,	_U,	_P,	_P,	_P,	_P,	_P,
/*140*/	_P,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L,
/*150*/	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
/*160*/	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
/*170*/	_L,	_L,	_L,	_P,	_P,	_P,	_P,	_C,
/*200*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0
};

#if DEBUG2
#define ASSERT(p) if (!(p))botch("p");else
static
botch(s)
char *s;
{PROGRAM(botch)
	char message[100];
	panic(strcat(strcpy(message,"assertion botched: "),s));
}
#else
#define ASSERT(p)
#endif

/*	C storage allocator
 *	circular first-fit strategy
 *	works with noncontiguous, but monotonically linked, arena
 *	each block is preceded by a ptr to the (pointer of) 
 *	the next following block
 *	blocks are exact number of words long 
 *	aligned to the data type requirements of ALIGN
 *	pointers to blocks must have BUSY bit 0
 *	bit in ptr is 1 for busy, 0 for idle
 *	gaps in arena are merely noted as busy blocks
 *	last block of arena (pointed to by alloct) is empty and
 *	has a pointer to first
 *	idle blocks are coalesced during space search
 *
 *	a different implementation may need to redefine
 *	ALIGN, NALIGN, BUSY, INT
 *	where INT is integer type to which a pointer can be cast
*/
#define INT address
#define ALIGN address
#define NALIGN 1
#define WORD sizeof(union store)

#define BUSY 1
#define testbusy(p) ((INT)(p)&BUSY)
#define setbusy(p) (union store *)((INT)(p)|BUSY)
#define clearbusy(p) (union store *)((INT)(p)&~BUSY)

union store {
	union store *ptr;
	ALIGN dummy[NALIGN];
};

extern int	stext[];		/* bottom of boot program (u3b5) */
extern int	sstack[];		/* bottom of boot program (u3b2) */

#ifdef u3b5
address		lowater = (address)stext; /* low-water mark below boot program set by malloc() */
#endif

#ifdef u3b2
address		lowater = (address)sstack; /* low-water mark below boot program set by malloc() */
#endif

extern address	hiwater;		/* high-water mark above base of memory set by loadunix() */

static union store  allocs;   /*  initial arena  */
static union store *allocb;
static union store *allocp;
static union store *alloct;

char * 
malloc(nbytes)
unsigned nbytes;
{PROGRAM(malloc)
	register union store *p, *q;
	register int nw;
	register int temp;

	if (allocs.ptr == 0) {   /*  first time  */
#ifdef u3b5
		lowater = (address)stext - WORD;
#endif
#ifdef u3b2
		lowater = (address)sstack - WORD;
#endif
		allocb = (union store *) lowater;
		allocs.ptr = setbusy(allocb);
		allocb->ptr = setbusy(&allocs);
		alloct = allocb;
		allocp = &allocs;
	}
	ASSERT(allocp >= allocb);
	ASSERT(allock());
	ASSERT(nbytes != 0);

	if (hiwater > lowater)
		/* memory overflow */
		error(ER1);

	nw = (nbytes + WORD + WORD - 1) / WORD;
	for (p = allocp,temp = 0; temp <= 1;) {
		if (testbusy(p->ptr) == 0) {
			while (testbusy((q=p->ptr)->ptr) == 0) {
				ASSERT(q > p && p >= allocb);
				allocp = p;
				p->ptr = q->ptr;
			}
			if (q >= p+nw && p+nw >= p) {
				allocp = p + nw;
				ASSERT(allocp >= allocb);
				if (q > allocp)
					allocp->ptr = p->ptr;
				p->ptr = setbusy(allocp);
				return((char *)(p + 1));
			}
		}
		q = p;
		p = clearbusy(p->ptr);
		if (p < q)
			ASSERT(p == allocb && q == &allocs);
		else
			if (p == &allocs || q == alloct) {
				ASSERT(p == &allocs && q == alloct);
				++temp;
			}
	}
	lowater -= nw * WORD;
	q = (union store *) lowater;
	ASSERT(q < allocb);
	allocb = q;
	q->ptr = allocs.ptr;
	allocs.ptr = setbusy(allocb);
	return((char *)(q + 1));
}

/*	freeing strategy tuned for LIFO allocation */

void
free(ap)
register char *ap;
{PROGRAM(free)
	register union store *p = (union store *)ap;

	ASSERT(p > allocb && p < alloct);
	ASSERT(allock());
	allocp = --p;
	ASSERT(testbusy(p->ptr));
	p->ptr = clearbusy(p->ptr);
	ASSERT(p >= allocb && p->ptr <= alloct);
}


#if DEBUG2
static
allock()
{PROGRAM(allock)
	register union store *p;
	int x;
	x = 0;
	for (p = clearbusy(allocs.ptr); clearbusy(p->ptr) > p; p = clearbusy(p->ptr)) {
		if (p == allocp)
			x++;
	}
	ASSERT(p == &allocs);
	return(x == 1 || p == allocp);
}
#endif


#if DEBUG1i
/*
 * Mallocp()
 *
 * Print a map showing the status of allocated/free memory
 */
 void
mallocp()
	{PROGRAM(mallocp)

	register union store *p;

	printf("Malloc map\n");

	if (allocs.ptr != 0)
		{
		ASSERT(allock());

		for (p=allocb; p<alloct; p=clearbusy(p->ptr))
			printf("     %6lX-%6lX:  %6lX  [%s]\n", p+1, clearbusy(p->ptr), (address)(clearbusy(p->ptr))-(address)(p+1),
					testbusy(p->ptr)? "allocated" : "free");

		ASSERT(p==alloct);
		}
	}
#endif

/*
 * Concatenate s2 on the end of s1.  S1's space must be large enough.
 * Return s1.
 */

char *
strcat(s1, s2)
register char *s1, *s2;
{PROGRAM(strcat)
	register char *os1;

	os1 = s1;
	while (*s1++)
		;
	--s1;
	while (*s1++ = *s2++)
		;
	return(os1);
}

#ifdef u3b2
int
strcmp(ptr1,ptr2)
register char *ptr1,*ptr2;
{PROGRAM(strcmp)
	while (*ptr1 == *ptr2) {
		if (*ptr1++ == '\0')
			return(0);
		ptr2++;
	}
	return(*ptr1 - *ptr2);
}

/*
 *  Compare strings (at most n bytes)
 *   returns; s1>s2; >0 s1==s2; 0 s1<s2; <0
 *
 */
int
strncmp(s1, s2, n)
register char *s1, *s2;
register n;
{PROGRAM(strncmp)
	if (s1 == s2)
		return(0);
	while (--n >= 0 && *s1 == *s2++)
		if (*s1++ == '\0')
			return(0);
	return((n < 0)? 0: (*s1 - *--s2));
}
#endif

/*
 * Return the number of characters in the maximum leading segment
 * of string which consists solely of characters NOT from charset.
 */
int
strcspn(string, charset)
char	*string;
register char	*charset;
{PROGRAM(strcspn)
	register char *p, *q;

	for (q=string; *q != '\0'; ++q) {
		for (p=charset; *p != '\0' && *p != *q; ++p)
			;
		if (*p != '\0')
			break;
	}
	return(q-string);
}

/*
 * Concatenate s2 on the end of s1.  S1's space must be large enough.
 * At most n characters are moved.
 * Return s1.
 */

char *
strncat(s1, s2, n)
register char *s1, *s2;
register n;
{PROGRAM(strncat)
	register char *os1;

	os1 = s1;
	while (*s1++)
		;
	--s1;
	while (*s1++ = *s2++)
		if (--n < 0) {
			*--s1 = '\0';
			break;
		}
	return(os1);
}

/*
 * Copy s2 to s1, truncating or null-padding to always copy n bytes
 * return s1
 */

char *
strncpy(s1, s2, n)
register char *s1, *s2;
int	n;
{PROGRAM(strncpy)
	register i;
	register char *os1;

	os1 = s1;
	for (i = 0; i < n; i++)
		if ((*s1++ = *s2++) == '\0') {
			while (++i < n)
				*s1++ = '\0';
			return(os1);
		}
	return(os1);
}

/*
 * Return ptr to first occurance of any character from `brkset'
 * in the character string `string'; NULL if none exists.
 */

char *
strpbrk(string, brkset)
register char *string, *brkset;
{PROGRAM(strpbrk)
	register char *p;

	do {
		for (p=brkset; *p != '\0' && *p != *string; ++p)
			;
		if (*p != '\0')
			return(string);
	}
	while (*string++);
	return(NULL);
}

/*
 * Return the number of characters in the maximum leading segment
 * of string which consists solely of characters from charset.
 */
int
strspn(string, charset)
char	*string;
register char	*charset;
{PROGRAM(strspn)
	register char *p, *q;

	for (q=string; *q != '\0'; ++q) {
		for (p=charset; *p != '\0' && *p != *q; ++p)
			;
		if (*p == '\0')
			break;
	}
	return(q-string);
}

/*
 * uses strpbrk and strspn to break string into tokens on
 * sequentially subsequent calls.  returns NULL when no
 * non-separator characters remain.
 * `subsequent' calls are calls with first argument NULL.
 */

char *
strtok(string, sepset)
char	*string, *sepset;
{PROGRAM(strtok)
	register char	*p, *q, *r;
	static char	*savept;

	/*first or subsequent call*/
	p = (string == NULL)? savept: string;

	if (p == 0)		/* return if no tokens remaining */
		return(NULL);

	q = p + strspn(p, sepset);	/* skip leading separators */

	if (*q == '\0')		/* return if no tokens remaining */
		return(NULL);

	if ((r = strpbrk(q, sepset)) == NULL)	/* move past token */
		savept = 0;	/* indicate this is last token */
	else {
		*r = '\0';
		savept = ++r;
	}
	return(q);
}

#define DIGIT(x) (isdigit(x)? ((x)-'0') : (10+(isupper(x)? (_tolower(x)-'a') : (x-'a'))))
#define MBASE 36

 long
strtol(str, ptr, base)
register char *str, **ptr;
register int base;
{PROGRAM(strtol)
	register long val;
	register int xx, sign;

	val = 0L;
	sign = 1;
	if (base < 0 || base > MBASE)
		goto OUT;
	while (isspace(*str))
		++str;
	if (*str == '-') {
		++str;
		sign = -1;
	} else if (*str == '+')
		++str;
	if (base == 0) {
		if (*str == '0') {
			++str;
			if (*str == 'x' || *str == 'X') {
				++str;
				base = 16;
			} else
				base = 8;
		} else
			base = 10;
	} else if (base == 16)
		if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
			str += 2;
	/*
	 * for any base > 10, the digits incrementally following
	 *	9 are assumed to be "abc...z" or "ABC...Z"
	 */
	while (isalnum(*str) && (xx=DIGIT(*str)) < base) {
		/* accumulate neg avoids surprises near maxint */
		val = base*val - xx;
		++str;
	}
OUT:
	if (ptr != (char**)0)
		*ptr = str;
	return(sign*(-val));
}

/*
 *	Stripped STDIO library
 */


/*
 *	Copied from /usr/src/lib/libc/3b5/stdio/stdio.h
 */

#define BUFSIZ	BSIZE


/*
 *	Copied from /usr/src/lib/libc/3b5/stdio/data.c
 */

FILE _iob[NOFILE];

/*
 *	Copied from /usr/src/lib/libc/3b5/stdio/fdopen.c
 */

/*
 * Unix routine to do an "fopen" on file descriptor
 */
FILE *
fdopen(fd)
int	fd;
{PROGRAM(fdopen)
	register FILE *iop;

	if ((unsigned)fd >= NOFILE || (iop = &_iob[fd])->_flag & _IOREAD)
		return(NULL);

	iop->_cnt = 0;
	iop->_file = fd;
	iop->_flag |= _IOREAD;

	return(iop);
}

/*
 *	Copied from /usr/src/lib/libc/3b5/stdio/filbuf.c
 */
int
_filbuf(iop)
register FILE *iop;
{PROGRAM(_filbuf)

	if (!(iop->_flag & _IOREAD))
		return(EOF);

	if (iop->_base == NULL) {
		if ((iop->_base=(unsigned char*)malloc(BUFSIZ)) == NULL)
			panic("no memory for FILE buffer");
		iop->_flag |= _IOMYBUF;
	}
	iop->_ptr = iop->_base;
	iop->_cnt = read(fileno(iop), (char*)iop->_ptr, (unsigned)BUFSIZ);
	if (--iop->_cnt < 0) {
		if (iop->_cnt == -1 || !(iop->_flag & _IOMYBUF))
			iop->_flag |= _IOEOF;
		else
			iop->_flag |= _IOERR;
		iop->_cnt = 0;
		return(EOF);
	}
	return(*iop->_ptr++);
}

/*
 *	Copied from /usr/src/lib/libc/3b5/stdio/fgets.c
 */
char *
fgets(s, n, iop)
char *s;
register int n;
register FILE *iop;
{PROGRAM(fgets)
	register int c;
	register char *cs;

	cs = s;
	while (--n > 0 && (c = getc(iop)) != EOF) {
		*cs++ = c;
		if (c == '\n')
			break;
	}
	if (c == EOF && cs == s)
		return(NULL);
	*cs++ = '\0';
	return(s);
}

/*
 *	Copied from /usr/src/lib/libc/3b5/stdio/fread.c
 */
int
fread(ptr, size, count, iop)
register char *ptr;
int size, count;
register FILE *iop;
{PROGRAM(fread)
	register int c, ndone, s;

	if (size <= 0 || count <= 0)
		return(0);
	for (ndone = 0; ndone < count; ++ndone) {
		for (s = 0; s < size; ++s) {
			if ((c = getc(iop)) == EOF)
				return(ndone);
			*ptr++ = c;
		}
	}
	return(ndone);
}

/*
 *	Copied from /usr/src/lib/libc/3b5/stdio/fseek.c
 */

/*
 * Seek for standard library.  Coordinates with buffering.
 */
int
fseek(iop, offset, ptrname)
register FILE *iop;
register long offset;
register int ptrname;
{PROGRAM(fseek)
	register int c;
	register long p;

	iop->_flag &= ~_IOEOF;
	if (iop->_flag & _IOREAD) {
		if (ptrname < 2 && iop->_base) {
			c = iop->_cnt;
			p = offset;
			if (ptrname == 0)
				p += (long)c-lseek(fileno(iop), 0L, 1);
			else
				offset -= (long)c;
			if (c > 0 && p <= c && p >= iop->_base - iop->_ptr) {
				iop->_ptr += (int)p;
				iop->_cnt -= (int)p;
				return(0);
			}
		}
		p = lseek(fileno(iop), offset, ptrname);
		iop->_cnt = 0;
	}
	return((p == -1)? -1: 0);
}

/*
 *	Copied from /usr/src/lib/libc/3b5/stdio/flsbuf.c
 */

/*
 *	Fdclose(iop) just like fclose(), but the file is not closed,
 *	just the buffering is detached from the file.
 *
 *	The original file descriptor is returned if there is no error;
 *	otherwise -1 is returned and errno is set to the error number.
 */
int
fdclose(iop)
register FILE *iop;
{PROGRAM(fdclose)
	register int rtn;

	if (iop->_flag & _IOREAD) {
		rtn = fileno(iop);
		if (iop->_flag & _IOMYBUF) {
			free((char*)iop->_base);
			iop->_base = NULL;
		}
	}
	else {
		errno = ENOENT;
		rtn = EOF;
	}

	iop->_flag = '\0';
	iop->_cnt = 0;
	iop->_ptr = iop->_base;
	return(rtn);
}

/*
 * Itoa(number)
 *
 * Convert number to right adjusted ascii string; pointer returned is to
 * beginning of char [15] array, with result right adjusted and left padded
 * with blanks (and an optional minus sign).
 */
 char *
itoa(number)
	register int number;
	{PROGRAM(itoa)

	static char buffer[15+1];
	register boolean minus;
	register char *p;

	if (!(minus = number<0))
		/*
		 * compute using negative numbers to avoid problems at MAXINT
		 */
		number = -number;

	*(p=buffer+sizeof(buffer)-1) = '\0';

	do	{
		*--p = '0' - (number%10);
		}
		while ((number/=10) < 0);

	if (minus)
		*--p = '-';

	while (p > buffer)
		*--p = ' ';

	return(buffer);
	}

/*
 * Lcase(string)
 *
 * Convert upper case letters in string to lower case.  Return the string.
 */
 char *
lcase(s)
	char *s;
	{PROGRAM(lcase)

	register char *p;
	register c;

	for (p=s; c = *p; ++p)
		if (isascii(c) && isupper(c))
			*p = _tolower(c);

	return(s);
	}

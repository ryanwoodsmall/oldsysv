/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/string.c	10.4"
/*	.file	"string.s"

#  string functions copied from the C library

# Fast assembler language version of the following C-program for
#			strcmp
# which represents the `standard' for the C-library.

	strcmp(s1, s2)
	register char *s1, *s2;
	{
		if (s1 == s2)
			return(0);
		while (*s1 == *s2++)
			if (*s1++ == '\0')
				return(0);
		return(*s1 - *--s2);
	}

#	L. Rosler, 3/82
#
	.globl	strcmp
	.align	4
strcmp:
	save	&0		# uses scratch regs (0, 1, 2)
	movw	&0,%r0		# escape char (null)
	movw	0(%ap),%r1	# s1
	movw	4(%ap),%r2	# s2
	cmpw	%r1,%r2
	je	.L0		# s1 == s2 implies strings equal
	cmpce	%r1,%r2,%r0	# let your fingers do the walking
	jz	.L0		# strings equal
	subb3	0(%r2),0(%r1),%r0	# return (*s2 - *s1)
	movbbw	%r0,%r0		# extend sign
.L0:
	ret	&0


# Fast assembler language version of the following C-program
#			strncmp
# which represents the `standard' for the C-library.

# Compare strings (at most n bytes): return *s1-*s2 for the last
# characters in s1 and s2 which were compared.
*/

	strncmp(s1, s2, n)
	register char *s1, *s2;
	register n;
	{
		if (s1 == s2)
			return(0);
		while (--n >= 0 && *s1 == *s2++)
			if (*s1++ == '\0')
				return(0);
		return((n < 0)? 0: *s1 - *--s2);
	}
/*
#
#	L. Rosler, 3/82
#
	.globl	strncmp
	.align	4
strncmp:
	save	&1		# non-scratch reg used (8)
	movw	&0,%r0		# escape char (null)
	movw	0(%ap),%r1	# s1
	movw	4(%ap),%r2	# s2
	cmpw	%r1,%r2
	je	.L5		# s1 == s2 implies strings equal
	movw	8(%ap),%r8	# n
	jnpos	.L1		# done, n <= 0
	cmpcce	%r1,%r8,%r2,%r0	# let your fingers do the walking
	jz	.L5		# strings equal
	subb3	0(%r2),0(%r1),%r0	# return (*s2 - *s1)
.L1:
	movbbw	%r0,%r0		# extend sign
.L5:
	ret	&1


# Fast assembler language version of the following C-program
#			strlen
# which represents the `standard' for the C-library.

# Given string s, return length (not including the terminating null).
*/

	strlen(s)
	register char *s;
	{
		register n;
	
		n = 0;
		while (*s++)
			n++;
		return(n);
	}
/*
#
#	L. Rosler, 3/82
#
	.globl	strlen
	.align	4
strlen:
	save	&0		# uses scratch regs only
	movw	0(%ap),%r0	# s
	movw	&0,%r1		# null char to search for
	locce	%r0,%r1,%r1	# let your fingers do the walking
#if it turns out that loccc is faster, the following should be used instead:
#	mnegw	&1,%r2		# very large pseudo-count
#	loccc	%r0,%r2,%r1	# let your fingers do the walking
	subw2	0(%ap),%r0	# calculate string length in r0
	ret	&0



# Fast assembler language version of the following C-program
#			strcpy
# which represents the `standard' for the C-library.

# Copy string s2 to s1.  s1 must be large enough. Return s1.
*/

	char *
	strcpy(s1, s2)
	register char *s1, *s2;
	{
		register char *os1;
	
		os1 = s1;
		while (*s1++ = *s2++)
			;
		return(os1);
	}

/*
#
#	L. Rosler, 3/82
#
	.globl	strcpy
	.align	4
strcpy:
	save	&0		# uses scratch regs (0, 1, 2)
	movw	0(%ap),%r0	# s1
	movw	4(%ap),%r1	# s2
	movw	&0,%r2		# escape char (null)
	movce	%r1,%r0,%r2	# let your fingers do the walking
	movw	0(%ap),%r0	# return s1
	ret	&0
*/

/* #ident	"@(#)libc-port:gen/strncpy.c	1.8" */
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Copy s2 to s1, truncating or null-padding to always copy n bytes
 * return s1
 */
#include <sys/types.h>

char *
strncpy(s1, s2, n)
register char *s1, *s2;
register size_t n;
{
	register char *os1 = s1;

	n++;				
	while ((--n > 0) &&  ((*s1++ = *s2++) != '\0'))
		;
	if (n > 0)
		while (--n > 0)
			*s1++ = '\0';
	return (os1);
}

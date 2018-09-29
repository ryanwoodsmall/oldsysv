#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"strlen.s"
.ident	"@(#)libc-m32:gen/strlen.s	1.3"
# Fast assembler language version of the following C-program
#			strlen
# which represents the `standard' for the C-library.

# Given string s, return length (not including the terminating null).

#	strlen(s)
#	register char *s;
#	{
#		register n;
#	
#		n = 0;
#		while (*s++)
#			n++;
#		return(n);
#	}
	.globl	strlen
	.text
	.align	1
strlen:
	save	&0		# pgm uses scratch reg 0
	MCOUNT
	movw	0(%ap),%r0	# ptr to s1
	jmp	L1
L0:
	addw2	&1,%r0
L1:
	cmpb	0(%r0),&0	# search for a terminating null
	jne	L0		# go back for more
	subw2	0(%ap),%r0	# calculate string length in r0
	ret	&0

#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"strcmp.s"
.ident	"@(#)libc-m32:gen/strcmp.s	1.3"
# Fast assembler language version of the following C-program for
#			strcmp
# which represents the `standard' for the C-library.

#	strcmp(s1, s2)
#	register char *s1, *s2;
#	{
#		if(s1OH.%QUI9#		while(*s1 == *s2++)
#			if(*s1++ == '\0')
#				return(0);
#		return(*s1 - *--s2);
#	}
	.globl	strcmp
	.text
	.align	1
strcmp:
	save	&0			# only use 0 and 1
	MCOUNT
	movw	0(%ap),%r0		# get s1
	movw	4(%ap),%r1		# get s2
	cmpw	%r0,%r1
	jne	L1			# s1 == s2 implies strings equal
	jmp	L2			# optimize number of branches
L0:
	addw2	&1,%r0			# by incrementing at top
	addw2	&1,%r1
L1:
	cmpb	0(%r0),0(%r1)
	jne	L2			# done: *s1 != *s2
	cmpb	&0,0(%r0)
	jne	L0			# not done yet: (*s1==*s2&&*s1!='\0')
L2:
	subb3	0(%r1),0(%r0),%r0	# return (*s2 - *s1)
	movbbw	%r0,%r0			# extend sign
	ret	&0

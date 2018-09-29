#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"memcpy.s"
	.ident	"@(#)libc-m32:gen/memcpy.s	1.5"
#
#	/* Copy s2 to s1, always copy n bytes. */
#
#	char *
#	memcpy(s1, s2, n)
#	register char *s1, *s2;
#	register int n;
#	{
#		register char *os1 = s1;
#		while (--n >= 0)
#			*s1++ = *s2++;
#		return (os1);
#	}
#
	.text
	.align	4
	.globl	memcpy
	.def	memcpy;	.val	memcpy;	.scl	2; .type 0142;	.endef

memcpy:
	MCOUNT
	movw	0(%ap),%r0	# register char *os1 = s1
	movw	4(%ap),%r1	# register char *s2
	movw	8(%ap),%r2	# register int   n
	jnpos	.return		# if (n <= 0) return

	bitw	&0x3,%r0	# if (dest is word aligned)
	jne	.mov_byte
	bitw	&0x3,%r1	# if (src is word aligned)
	jne	.mov_byte
	jmp	.wordmov

				# do word move:
	.align	4
.mov_word:
	movw	0(%r1),0(%r0)
	addw2	&4,%r0
	addw2	&4,%r1
.wordmov:
	subw2	&4,%r2
	jge	.mov_word

	addw2	&4,%r2
	jg	.mov_byte
	jmp	.return


.inc_pointers:			# do
	addw2	&1,%r0		#	s1++
	addw2	&1,%r1		#	s2++
.mov_byte:
	movb	0(%r1),0(%r0)	#	*s1 = *s2
	subw2	&1,%r2		#	n--
	jpos	.inc_pointers	# while ( n > 0 )

.return:
	movw	0(%ap),%r0	# return (os1)
	RET
	.def	memcpy;	.val	.;	.scl	-1;	.endef

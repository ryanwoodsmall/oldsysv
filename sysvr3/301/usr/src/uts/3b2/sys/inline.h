/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/inline.h	10.6"
#ifndef CXREF
asm	char *
strend(cp)
{
%	mem	cp;

	MOVW	cp,%r0
	STREND
}

asm	int
strlen(cp)
{
%	mem	cp;

	MOVW	cp,%r0
	STREND
	SUBW2	cp,%r0
}

asm	void
strcpy(dp, sp)
{
%	mem	dp, sp;

	MOVW	sp,%r0
	MOVW	dp,%r1
	STRCPY
}

asm	void
strcat(dp, sp)
{
%	mem	dp, sp;

	MOVW	dp,%r0
	STREND
	MOVW	%r0,%r1
	MOVW	sp,%r0
	STRCPY
}



asm	int
spl1()
{
	MOVW	%psw,%r0
	INSFW	&3,&13,&8,%psw
}

asm	int
spl4()
{
	MOVW	%psw,%r0
	INSFW	&3,&13,&10,%psw
}

asm	int
spl5()
{
	MOVW	%psw,%r0
	INSFW	&3,&13,&10,%psw	
}

asm	int
spl6()
{
	MOVW	%psw,%r0
	INSFW	&3,&13,&12,%psw
}

asm	int
splpp()
{
	MOVW	%psw,%r0
	INSFW	&3,&13,&10,%psw
}

asm	int
splni()
{
	MOVW	%psw,%r0
	INSFW	&3,&13,&12,%psw
}

asm	int
spltty()
{
	MOVW	%psw,%r0
	INSFW	&3,&13,&13,%psw
}

asm	int
spl7()
{
	MOVW	%psw,%r0
	ORW2	&0x1e000,%psw
}

asm	int
splhi()
{
	MOVW	%psw,%r0
	ORW2	&0x1e000,%psw
}

asm	int
spl0()
{
	MOVW	%psw,%r0
	ANDW2	&0xfffe1fff,%psw
}

asm	int
splx(opsw)
{
%	mem	opsw;
	MOVW	%psw,%r0
	MOVW	opsw,%psw
}

asm	void
movpsw(dest)
{
%	mem	dest;
	MOVW	%psw,dest
}
#endif

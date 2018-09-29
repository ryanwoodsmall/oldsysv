/ @(#)end.s	1.1
.data
/ Bootstrap program executed in user mode
/ to bring up the system.
exec = 11.
.globl	_icode, _szicode
_icode:
/ runs at location 0 in user space
loc0:
	sys	exec; 2f-loc0; 1f-loc0
	br	.
1:
	2f-loc0
	0
2:
	</etc/init\0>
9:
_szicode:
	9b-_icode

.globl	_ka6
.globl	_cputype

_ka6:	KDSA0+[6*2]
_cputype:45.
stk:	0
	0
	0

.bss
nofault:.=.+2
ssr:	.=.+6
dispdly:.=.+2
saveps:	.=.+2

.globl	_pwr_cnt, _pwr_act
_pwr_cnt:	.=.+2
_pwr_act:	.=.+2
power:	.=.+2
pfloop:	.=.+2
pflock:	.=.+16.
pfreg:	.=.+14.
pfareg:	.=.+14.
pfksp:	.=.+2
	.=.+4
pffreg:	.=.+2.
	.=.+48.
pfmreg:	.=.+64.
	.=.+64.
	.=.+64.
pfsreg:	.=.+2
	.=.+124.
pfcreg:	.=.+14.
pftime:	.=.+4

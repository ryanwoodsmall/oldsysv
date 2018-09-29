/ @(#)end.s	1.1
.data
/ Bootstrap program executed in user mode
/ to bring up the system.
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

.globl	_cputype

_cputype:40.
stk:	0

.bss

.globl	_u
_u:	.=.+[usize*64.]

nofault:.=.+2
ssr:	.=.+6
mmr:	.=.+10.
saveps:	.=.+2
pir:	.=.+2
tpir:	.=.+2

.globl	_pwr_cnt, _pwr_act
_pwr_cnt:	.=.+2
_pwr_act:	.=.+2
power:	.=.+2
pfloop:	.=.+2
pflock:	.=.+16.
pfreg:	.=.+14.
pfksp:	.=.+2
	.=.+4
pffreg:	.=.+2.
	.=.+48.
pfmreg:	.=.+32.
	.=.+32.
pfcreg:	.=.+4.
pftime:	.=.+4

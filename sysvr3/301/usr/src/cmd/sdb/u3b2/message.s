	.file	"message.c"
	.version	"02.01"
	.text
	.ident	"@(#)head:setjmp.h	1.9"
	.ident	"@(#)kern-port:sys/param.h	10.10"
	.ident	"@(#)	10.4"
	.ident	"@(#)kern-port:sys/types.h	10.5"
	.ident	"@(#)head:signal.h	1.5"
	.ident	"@(#)kern-port:sys/signal.h	10.6"
	.ident	"@(#)kern-port:sys/sysmacros.h	10.3"
	.ident	"@(#)kern-port:sys/dir.h	10.3"
	.ident	"@(#)	10.1"
	.ident	"@(#)kern-port:sys/immu.h	10.5"
	.ident	"@(#)kern-port:sys/psw.h	10.2"
	.ident	"@(#)kern-port:sys/pcb.h	10.1"
	.ident	"@(#)kern-port:sys/user.h	10.10"
	.ident	"@(#)kern-port:sys/errno.h	10.6"
	.ident	"@(#)head:a.out.h	2.12"
	.ident	"@(#)head:nlist.h	1.8"
	.ident	"@(#)sgs-inc:common/filehdr.h	1.19"
	.ident	"@(#)head:aouthdr.h	2.6"
	.ident	"@(#)sgs-inc:common/scnhdr.h	1.10"
	.ident	"@(#)head:reloc.h	2.6"
	.ident	"@(#)sgs-inc:common/linenum.h	1.4"
	.ident	"@(#)head:syms.h	2.6"
	.ident	"@(#)sgs-inc:common/storclass.h	1.4"
	.ident	"@(#)kern-port:sys/reg.h	10.2"
	.ident	"@(#)kern-port:sys/stat.h	10.5"
	.ident	"@(#)head:stdio.h	2.14"
	.ident	"@(#)head:termio.h	1.3"
	.ident	"@(#)kern-port:sys/termio.h	10.3"
	.globl	BADMOD
BADMOD:
	.word	8:66,8:97,8:100,8:32
	.word	8:109,8:111,8:100,8:105
	.word	8:102,8:105,8:101,8:114
	.word	8:0,24:0
	.globl	NOBKPT
NOBKPT:
	.word	8:78,8:111,8:32,8:98
	.word	8:114,8:101,8:97,8:107
	.word	8:112,8:111,8:105,8:110
	.word	8:116,8:32,8:115,8:101
	.word	8:116,8:0,16:0
	.globl	NOPCS
NOPCS:
	.word	8:78,8:111,8:32,8:112
	.word	8:114,8:111,8:99,8:101
	.word	8:115,8:115,8:0,8:0
	.globl	BADTXT
BADTXT:
	.word	8:84,8:101,8:120,8:116
	.word	8:32,8:97,8:100,8:100
	.word	8:114,8:101,8:115,8:115
	.word	8:32,8:110,8:111,8:116
	.word	8:32,8:102,8:111,8:117
	.word	8:110,8:100,8:0,8:0
	.globl	BADDAT
BADDAT:
	.word	8:68,8:97,8:116,8:97
	.word	8:32,8:97,8:100,8:100
	.word	8:114,8:101,8:115,8:115
	.word	8:32,8:110,8:111,8:116
	.word	8:32,8:102,8:111,8:117
	.word	8:110,8:100,8:0,8:0
	.globl	EXBKPT
EXBKPT:
	.word	8:84,8:111,8:111,8:32
	.word	8:109,8:97,8:110,8:121
	.word	8:32,8:98,8:114,8:101
	.word	8:97,8:107,8:112,8:111
	.word	8:105,8:110,8:116,8:115
	.word	8:0,24:0
	.globl	ENDPCS
ENDPCS:
	.word	8:80,8:114,8:111,8:99
	.word	8:101,8:115,8:115,8:32
	.word	8:116,8:101,8:114,8:109
	.word	8:105,8:110,8:97,8:116
	.word	8:101,8:100,8:0,8:0
	.globl	SZBKPT
SZBKPT:
	.word	8:66,8:107,8:112,8:116
	.word	8:58,8:32,8:67,8:111
	.word	8:109,8:109,8:97,8:110
	.word	8:100,8:32,8:116,8:111
	.word	8:111,8:32,8:108,8:111
	.word	8:110,8:103,8:0,8:0
	.globl	BADWAIT
BADWAIT:
	.word	8:87,8:97,8:105,8:116
	.word	8:32,8:101,8:114,8:114
	.word	8:111,8:114,8:58,8:32
	.word	8:80,8:114,8:111,8:99
	.word	8:101,8:115,8:115,8:32
	.word	8:100,8:105,8:115,8:97
	.word	8:112,8:112,8:101,8:97
	.word	8:114,8:101,8:100,8:33
	.word	8:0,24:0
	.globl	NOFORK
NOFORK:
	.word	8:84,8:114,8:121,8:32
	.word	8:97,8:103,8:97,8:105
	.word	8:110,8:0,16:0
	.globl	BADMAGN
BADMAGN:
	.word	8:66,8:97,8:100,8:32
	.word	8:99,8:111,8:114,8:101
	.word	8:32,8:109,8:97,8:103
	.word	8:105,8:99,8:32,8:110
	.word	8:117,8:109,8:98,8:101
	.word	8:114,8:0,16:0
	.align	4
	.globl	signals
signals:
	.word	.L243
	.word	.L244
	.word	.L245
	.word	.L246
	.word	.L247
	.word	.L248
	.word	.L249
	.word	.L250
	.word	.L251
	.word	.L252
	.word	.L253
	.word	.L254
	.word	.L255
	.word	.L256
	.word	.L257
	.word	.L258
	.word	.L259
	.word	.L260
	.word	.L261
	.word	.L262
	.zero	12
.L243:

	.byte	85,78,75,78,79,87,78,32,83,73
	.byte	71,78,65,76,32,40,48,41,0
.L244:

	.byte	72,97,110,103,117,112,32,40,49,41
	.byte	0
.L245:

	.byte	73,110,116,101,114,114,117,112,116,32
	.byte	40,50,41,0
.L246:

	.byte	81,117,105,116,32,40,51,41,0
.L247:

	.byte	73,108,108,101,103,97,108,32,73,110
	.byte	115,116,114,117,99,116,105,111,110,32
	.byte	40,52,41,0
.L248:

	.byte	84,114,97,99,101,47,66,80,84,32
	.byte	40,53,41,0
.L249:

	.byte	73,79,84,32,40,54,41,0
.L250:

	.byte	69,77,84,32,40,55,41,0
.L251:

	.byte	70,108,111,97,116,105,110,103,32,69
	.byte	120,99,101,112,116,105,111,110,32,40
	.byte	56,41,0
.L252:

	.byte	75,105,108,108,101,100,32,40,57,41
	.byte	0
.L253:

	.byte	66,117,115,32,69,114,114,111,114,32
	.byte	40,49,48,41,0
.L254:

	.byte	77,101,109,111,114,121,32,70,97,117
	.byte	108,116,32,40,49,49,41,0
.L255:

	.byte	66,97,100,32,83,121,115,116,101,109
	.byte	32,67,97,108,108,32,40,49,50,41
	.byte	0
.L256:

	.byte	66,114,111,107,101,110,32,80,105,112
	.byte	101,32,40,49,51,41,0
.L257:

	.byte	65,108,97,114,109,32,67,97,108,108
	.byte	32,40,49,52,41,0
.L258:

	.byte	84,101,114,109,105,110,97,116,101,100
	.byte	32,40,49,53,41,0
.L259:

	.byte	85,115,101,114,32,83,105,103,110,97
	.byte	108,32,49,32,40,49,54,41,0
.L260:

	.byte	85,115,101,114,32,83,105,103,110,97
	.byte	108,32,50,32,40,49,55,41,0
.L261:

	.byte	68,101,97,116,104,32,111,102,32,97
	.byte	32,67,104,105,108,100,32,40,49,56
	.byte	41,0
.L262:

	.byte	80,111,119,101,114,45,70,97,105,108
	.byte	32,82,101,115,116,97,114,116,32,40
	.byte	49,57,41,0

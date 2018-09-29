#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

        .file   "fnegd.s"
.ident	"@(#)libc-m32:fp/fnegs.s	1.3"
#       float _fnegs(srcF)
#       float srcF;

        .text
        .align  4
        .def    _fnegs; .val    _fnegs; .scl    2;      .type   046;    .endef
        .globl  _fnegs
_fnegs:
        MCOUNT
        xorw3   &0x80000000,0(%ap),%r0
        RET
        .def    _fnegs; .val    .;      .scl    -1;     .endef


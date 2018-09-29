#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libF77:libF77.mk	1.10.1.1"
# f77(1) FORTRAN INTRINSIC FUNCTIONS library makefile

INS = install
INSDIR = $(ROOT)/usr/lib
CC = cc
AR = ar
STRIP = strip
CFLAGS= -O
LIB=$(ROOT)/lib
RM=/bin/rm
LIBP=$(ROOT)/usr/lib/libp
VARIANT = F77
LIBNAME = lib$(VARIANT).a

OBJ =	$(LIBNAME)(Version.o) \
	$(LIBNAME)(main.o) \
	$(LIBNAME)(s_rnge.o) \
	$(LIBNAME)(abort_.o) \
	$(LIBNAME)(getarg_.o) \
	$(LIBNAME)(iargc_.o) \
	$(LIBNAME)(getenv_.o) \
	$(LIBNAME)(signal_.o) \
	$(LIBNAME)(s_stop.o) \
	$(LIBNAME)(s_paus.o) \
	$(LIBNAME)(system_.o) \
	$(LIBNAME)(pow_ci.o) \
	$(LIBNAME)(pow_dd.o) \
	$(LIBNAME)(pow_di.o) \
	$(LIBNAME)(pow_hh.o) \
	$(LIBNAME)(pow_ii.o) \
	$(LIBNAME)(pow_ri.o) \
	$(LIBNAME)(pow_zi.o) \
	$(LIBNAME)(pow_zz.o) \
	$(LIBNAME)(c_abs.o) \
	$(LIBNAME)(c_cos.o) \
	$(LIBNAME)(c_div.o) \
	$(LIBNAME)(c_exp.o) \
	$(LIBNAME)(c_log.o) \
	$(LIBNAME)(c_sin.o) \
	$(LIBNAME)(c_sqrt.o) \
	$(LIBNAME)(z_abs.o) \
	$(LIBNAME)(z_cos.o) \
	$(LIBNAME)(z_div.o) \
	$(LIBNAME)(z_exp.o) \
	$(LIBNAME)(z_log.o) \
	$(LIBNAME)(z_sin.o) \
	$(LIBNAME)(z_sqrt.o) \
	$(LIBNAME)(r_abs.o) \
	$(LIBNAME)(r_acos.o) \
	$(LIBNAME)(r_asin.o) \
	$(LIBNAME)(r_atan.o) \
	$(LIBNAME)(r_atn2.o) \
	$(LIBNAME)(r_cnjg.o) \
	$(LIBNAME)(r_cos.o) \
	$(LIBNAME)(r_cosh.o) \
	$(LIBNAME)(r_dim.o) \
	$(LIBNAME)(r_exp.o) \
	$(LIBNAME)(r_imag.o) \
	$(LIBNAME)(r_int.o) \
	$(LIBNAME)(r_lg10.o) \
	$(LIBNAME)(r_log.o) \
	$(LIBNAME)(r_mod.o) \
	$(LIBNAME)(r_nint.o) \
	$(LIBNAME)(r_sign.o) \
	$(LIBNAME)(r_sin.o) \
	$(LIBNAME)(r_sinh.o) \
	$(LIBNAME)(r_sqrt.o) \
	$(LIBNAME)(r_tan.o) \
	$(LIBNAME)(r_tanh.o) \
	$(LIBNAME)(rand_.o) \
	$(LIBNAME)(d_abs.o) \
	$(LIBNAME)(d_acos.o) \
	$(LIBNAME)(d_asin.o) \
	$(LIBNAME)(d_atan.o) \
	$(LIBNAME)(d_atn2.o) \
	$(LIBNAME)(d_cnjg.o) \
	$(LIBNAME)(d_cos.o) \
	$(LIBNAME)(d_cosh.o) \
	$(LIBNAME)(d_dim.o) \
	$(LIBNAME)(d_exp.o) \
	$(LIBNAME)(d_imag.o) \
	$(LIBNAME)(d_int.o) \
	$(LIBNAME)(d_lg10.o) \
	$(LIBNAME)(d_log.o) \
	$(LIBNAME)(d_mod.o) \
	$(LIBNAME)(d_nint.o) \
	$(LIBNAME)(d_prod.o) \
	$(LIBNAME)(d_sign.o) \
	$(LIBNAME)(d_sin.o) \
	$(LIBNAME)(d_sinh.o) \
	$(LIBNAME)(d_sqrt.o) \
	$(LIBNAME)(d_tan.o) \
	$(LIBNAME)(d_tanh.o) \
	$(LIBNAME)(i_abs.o) \
	$(LIBNAME)(i_dim.o) \
	$(LIBNAME)(i_dnnt.o) \
	$(LIBNAME)(i_indx.o) \
	$(LIBNAME)(i_len.o) \
	$(LIBNAME)(i_mod.o) \
	$(LIBNAME)(i_nint.o) \
	$(LIBNAME)(i_sign.o) \
	$(LIBNAME)(h_abs.o) \
	$(LIBNAME)(h_dim.o) \
	$(LIBNAME)(h_dnnt.o) \
	$(LIBNAME)(h_indx.o) \
	$(LIBNAME)(h_len.o) \
	$(LIBNAME)(h_mod.o) \
	$(LIBNAME)(h_nint.o) \
	$(LIBNAME)(h_sign.o) \
	$(LIBNAME)(l_ge.o) \
	$(LIBNAME)(l_gt.o) \
	$(LIBNAME)(l_le.o) \
	$(LIBNAME)(l_lt.o) \
	$(LIBNAME)(hl_ge.o) \
	$(LIBNAME)(hl_gt.o) \
	$(LIBNAME)(hl_le.o) \
	$(LIBNAME)(hl_lt.o) \
	$(LIBNAME)(ef1asc_.o) \
	$(LIBNAME)(ef1cmc_.o) \
	$(LIBNAME)(s_cat.o) \
	$(LIBNAME)(s_cmp.o) \
	$(LIBNAME)(s_copy.o) \
	$(LIBNAME)(erf_.o) \
	$(LIBNAME)(erfc_.o) \
	$(LIBNAME)(derf_.o) \
	$(LIBNAME)(derfc_.o) \
	$(LIBNAME)(cabs.o) \
	$(LIBNAME)(tanh.o) \
	$(LIBNAME)(sinh.o) \
	$(LIBNAME)(mclock_.o) \
	$(LIBNAME)(outstr_.o) \
	$(LIBNAME)(subout.o) \
	$(LIBNAME)(and_l.o) \
	$(LIBNAME)(or_l.o) \
	$(LIBNAME)(xor_l.o) \
	$(LIBNAME)(not_l.o) \
	$(LIBNAME)(shift_l.o) \
	$(LIBNAME)(shiftc_l.o) \
	$(LIBNAME)(bclr_l.o) \
	$(LIBNAME)(bext_l.o) \
	$(LIBNAME)(bset_l.o) \
	$(LIBNAME)(btest_l.o) \
	$(LIBNAME)(mvbits.o) \
	$(LIBNAME)(bittab.o) \
	$(LIBNAME)(and_h.o) \
	$(LIBNAME)(or_h.o) \
	$(LIBNAME)(xor_h.o) \
	$(LIBNAME)(not_h.o) \
	$(LIBNAME)(shift_h.o) \
	$(LIBNAME)(shiftc_h.o) \
	$(LIBNAME)(bclr_h.o) \
	$(LIBNAME)(bext_h.o) \
	$(LIBNAME)(bset_h.o) \
	$(LIBNAME)(btest_h.o)
.c.o:;

all: $(LIBNAME) profiledlib

$(LIBNAME):	$(OBJ)
		$(CC) -c $(CFLAGS) $(?:.o=.c)
		$(AR) r $(LIBNAME) $?
		$(RM) $?

$(OBJ): complex bit.h

.c.a:;

profiledlib:
	$(MAKE) -f libF77.mk -e libp.$(LIBNAME) \
	LIBNAME=libp.$(LIBNAME) CFLAGS="-p $(CFLAGS)"

install: all
	cp $(LIBNAME) $(LIB)/$(LIBNAME)
	cp libp.$(LIBNAME) $(LIBP)/$(LIBNAME)
clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIBNAME) libp.$(LIBNAME)

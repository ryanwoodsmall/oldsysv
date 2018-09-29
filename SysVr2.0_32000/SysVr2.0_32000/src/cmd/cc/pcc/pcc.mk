#	@(#)pcc.mk	1.9
#
#	This makefile generates the vax or pdp11 portable C compiler.
#
#	The directories assumed (multi-machine view) are:
#
#			 /c2 --- c2.mk		(vax only)
#			  /
#			 / cc.c
#			/  /
#		       /  /  pcc.mk		(this makefile)
#	/usr/src/cmd/cc --   /
#		       \    /   /mip
#			\  /     /
#		       /pcc -----
#			   \     \
#			    \   /vax --- pcc.mk
#			     \
#			      \
#			     /pdp11 --- pcc.mk
#
TESTDIR=.
INS= /etc/install -n /bin
INSDIR=
CFLAGS=-O
LDFLAGS = -s -n

all: make_cc make_pcc

make_cc: ../cc.c
	if pdp11; \
	then \
		$(CC) $(LDFLAGS) $(CFLAGS) -UFLEXNAME -DCSW=SCJ1 \
			-'DCCNAME="pcc"' -o $(TESTDIR)/pcc ../cc.c; \
	elif vax || ns32000; \
	then \
		$(CC) $(LDFLAGS) $(CFLAGS) -DCSW=JFR -o $(TESTDIR)/cc ../cc.c; \
	fi

make_pcc:
	-if vax; \
	then \
		cd ../c2; \
		$(MAKE) -$(MAKEFLAGS) -f c2.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) all; \
	fi
	-if ns32000; \
	then \
		cd ../pci/ns32000; \
		$(MAKE) -$(MAKEFLAGS) -f c2.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) all; \
	fi
	-if vax ; \
	then \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) all; \
	fi
	-if ns32000 ; \
	then \
		cd ns32000; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) all; \
	fi
	-if pdp11; \
	then \
		cd pdp11; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS) -UFLEXNAME" TESTDIR=$(TESTDIR) all; \
	fi

clean:
	-rm -f *.o
	-if pdp11; \
	then \
		cd pdp11; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk TESTDIR=$(TESTDIR) clean; \
	fi
	-if vax; \
	then \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk TESTDIR=$(TESTDIR) clean; \
	fi
	-if ns32000; \
	then \
		cd ns32000; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk TESTDIR=$(TESTDIR) clean; \
	fi
	-if vax; \
	then \
		cd ../c2; \
		$(MAKE) -$(MAKEFLAGS) -f c2.mk TESTDIR=$(TESTDIR) clean; \
	fi
	-if ns32000; \
	then \
		cd ../pci/ns32000; \
		$(MAKE) -$(MAKEFLAGS) -f c2.mk TESTDIR=$(TESTDIR) clean; \
	fi

clobber: clean
	-if pdp11; \
	then \
		rm -f $(TESTDIR)/pcc; \
		cd pdp11; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk TESTDIR=$(TESTDIR) clobber; \
	fi
	-if vax; \
	then \
		rm -f $(TESTDIR)/cc; \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk TESTDIR=$(TESTDIR) clobber; \
	fi
	-if ns32000; \
	then \
		rm -f $(TESTDIR)/cc; \
		cd ns32000; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk TESTDIR=$(TESTDIR) clobber; \
	fi
	-if vax; \
	then \
		cd ../c2; \
		$(MAKE) -$(MAKEFLAGS) -f c2.mk TESTDIR=$(TESTDIR) clobber; \
	fi
	-if ns32000; \
	then \
		cd ../pci/ns32000; \
		$(MAKE) -$(MAKEFLAGS) -f c2.mk TESTDIR=$(TESTDIR) clobber; \
	fi

install: all
	-if pdp11; \
	then \
		$(INS) $(TESTDIR)/pcc; \
		cd pdp11; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS) -UFLEXNAME" TESTDIR=$(TESTDIR) \
			INSTALL=/lib install; \
	fi
	-if vax; \
	then \
		$(INS) $(TESTDIR)/cc; \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) \
			INSTALL=ccom install; \
		cd ../../c2; \
		$(MAKE) -$(MAKEFLAGS) -f c2.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) install; \
	fi
	-if ns32000; \
	then \
		$(INS) $(TESTDIR)/cc; \
		cd ns32000; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) \
			INSTALL=ccom install; \
		cd ../../pci/ns32000; \
		$(MAKE) -$(MAKEFLAGS) -f c2.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) install; \
	fi

justinstall: make_pcc
	#
	# justinstall is just like install except that it only depends
	# on make_pcc.  This forces the C compiler interface (cc or pcc)
	# to be assumed to be up to date.  This is needed when there is
	# incompatability between the old compilation package, and the
	# new one.  justinstall must only do installs.
	#
	-if pdp11; \
	then \
		$(INS) $(TESTDIR)/pcc; \
		cd pdp11; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS) -UFLEXNAME" TESTDIR=$(TESTDIR) \
			INSTALL=/lib install; \
	fi
	-if vax; \
	then \
		$(INS) $(TESTDIR)/cc; \
		cd vax; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) \
			INSTALL=ccom install; \
		cd ../../c2; \
		$(MAKE) -$(MAKEFLAGS) -f c2.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) install; \
	fi
	-if ns32000; \
	then \
		$(INS) $(TESTDIR)/cc; \
		cd ns32000; \
		$(MAKE) -$(MAKEFLAGS) -f pcc.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) \
			INSTALL=ccom install; \
		cd ../../pci/ns32000; \
		$(MAKE) -$(MAKEFLAGS) -f c2.mk LDFLAGS="$(LDFLAGS)" \
			CFLAGS="$(CFLAGS)" TESTDIR=$(TESTDIR) install; \
	fi

FRC:	# what is this for?

#	@(#)cc.mk	1.6
#
#	This makefile checks which machine (currently vax or pdp11)
#	it is used on, and builds a C compiler by wandering down to
#	the assumed directories, and invoking the makefiles for each
#	piece.  For the pdp11, it builds both cc and pcc.
#
#	The directory structure (global multi-machine view) assumed is:
#
#			/cpp --- cpp.mk
#			/
#		       /     /c2 --- c2.mk	(only for vax)
#		      /	      /
#		     /	     /	/cc --- cc.mk	(Ritchie's pdp11 C compiler)
#		    /	    /	/
#	/usr/src/cmd --- /cc ------- cc.mk	(this makefile)
#			    \   \
#			     \  cc.c
#			      \	     /mip
#			       \      /
#			        \    /   /vax --- pcc.mk
#			         \  /    /
#				/pcc ------- pcc.mk
#				    \
#				     \
#				    /pdp11 --- pcc.mk
#
TESTDIR = .
FRC =
INSDIR =
CFLAGS = -O -DFLEXNAMES
IFLAG = -i
LDFLAGS = -n -s
FFLAG =
YACCRM=-rm

all:
	-if pdp11; \
	then \
		cd cc; \
		$(MAKE) -$(MAKEFLAGS) -f cc.mk CFLAGS="$(CFLAGS) -UFLEXNAME" \
			LDFLAGS="$(LDFLAGS)" IFLAG=$(IFLAG) FFLAG=$(FFLAG) \
			TESTDIR=$(TESTDIR) all; \
	fi
	-cd pcc; \
	$(MAKE) -$(MAKEFLAGS) -f pcc.mk CFLAGS="$(CFLAGS)" \
		IFLAG=$(IFLAG) LDFLAGS="$(LDFLAGS)" \
		TESTDIR=$(TESTDIR) FFLAG=$(FFLAG) all
	-cd ../cpp; \
	$(MAKE) -$(MAKEFLAGS) -f cpp.mk CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS)" TESTDIR=$(TESTDIR)

install:
	-if pdp11; \
	then \
		cd cc; \
		$(MAKE) -$(MAKEFLAGS) -f cc.mk CFLAGS="$(CFLAGS) -UFLEXNAME" \
			LDFLAGS="$(LDFLAGS)" IFLAG=$(IFLAG) FFLAG=$(FFLAG) \
			TESTDIR=$(TESTDIR) install; \
	fi
	-cd pcc; \
	$(MAKE) -$(MAKEFLAGS) -f pcc.mk CFLAGS="$(CFLAGS)" \
		IFLAG=$(IFLAG) LDFLAGS="$(LDFLAGS)" \
		TESTDIR=$(TESTDIR) FFLAG=$(FFLAG) install
	-cd ../cpp; \
	$(MAKE) -$(MAKEFLAGS) -f cpp.mk CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS)" TESTDIR=$(TESTDIR) install

justinstall:
	#
	# justinstall acts just like install, except that in pcc.mk,
	# the C compiler interface (cc or pcc) is assumed to be built
	# and up to date - this is only necessary when a change in the
	# functionality of the whole compilation package forces installs
	# to only do installs, and no compiles, etc.
	#
	-if pdp11; \
	then \
		cd cc; \
		$(MAKE) -$(MAKEFLAGS) -f cc.mk CFLAGS="$(CFLAGS) -UFLEXNAME" \
			LDFLAGS="$(LDFLAGS)" IFLAG=$(IFLAG) FFLAG=$(FFLAG) \
			TESTDIR=$(TESTDIR) install; \
	fi
	-cd pcc; \
	$(MAKE) -$(MAKEFLAGS) -f pcc.mk CFLAGS="$(CFLAGS)" \
		IFLAG=$(IFLAG) LDFLAGS="$(LDFLAGS)" \
		TESTDIR=$(TESTDIR) FFLAG=$(FFLAG) justinstall
	-cd ../cpp; \
	$(MAKE) -$(MAKEFLAGS) -f cpp.mk CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS)" TESTDIR=$(TESTDIR) install

clean:
	-if pdp11; \
	then \
		cd cc; \
		$(MAKE) -$(MAKEFLAGS) -f cc.mk YACCRM=$(YACCRM) \
			TESTDIR=$(TESTDIR) clean; \
	fi
	-cd pcc; \
	$(MAKE) -$(MAKEFLAGS) -f pcc.mk TESTDIR=$(TESTDIR) \
		YACCRM=$(YACCRM) clean
	-cd ../cpp; \
	$(MAKE) -$(MAKEFLAGS) -f cpp.mk TESTDIR=$(TESTDIR) clean

clobber:
	-if pdp11; \
	then \
		cd cc; \
		$(MAKE) -$(MAKEFLAGS) -f cc.mk YACCRM=$(YACCRM) \
			TESTDIR=$(TESTDIR) clobber; \
	fi
	-cd pcc; \
	$(MAKE) -$(MAKEFLAGS) -f pcc.mk TESTDIR=$(TESTDIR) \
		YACCRM=$(YACCRM) clobber
	-cd ../cpp; \
	$(MAKE) -$(MAKEFLAGS) -f cpp.mk TESTDIR=$(TESTDIR) clobber

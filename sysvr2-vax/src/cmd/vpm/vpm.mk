#
#   @(#)vpm.mk	1.2
#
all:
	if u3b ; then \
		cd u3b20; $(MAKE) -f vpm.mk ; \
	else \
		cd dec; $(MAKE) -f vpm.mk all ; \
	fi

install:
	if u3b ; then \
		cd u3b20; $(MAKE) -f vpm.mk install ; \
	else \
		cd dec; $(MAKE) -f vpm.mk install ; \
	fi

clean:
	if u3b ; then \
		cd dec; $(MAKE) -f vpm.mk clean ; \
	else \
		cd u3b20; $(MAKE) -f vpm.mk clean ; \
	fi

clobber:
	if u3b ; then \
		cd u3b20; $(MAKE) -f vpm.mk clobber ; \
	else \
		cd dec; $(MAKE) -f vpm.mk clobber ; \
	fi

vpm:
	if u3b ; then \
		cd u3b20; $(MAKE) -f vpm.mk vpm ; \
	else \
		echo "Target vpm undefined." ; \
	fi

LIB:
	if u3b ; then \
		cd u3b20; $(MAKE) -f vpm.mk LIB ; \
	else \
		echo "Target LIB undefined." ; \
	fi

SCALLS:
	if u3b ; then \
		cd u3b20; $(MAKE) -f vpm.mk SCALLS ; \
	else \
		echo "Target SCALLS undefined." ; \
	fi

LDFILES:
	if u3b ; then \
		cd u3b20; $(MAKE) -f vpm.mk LDFILES ; \
	else \
		echo "Target LDFILES undefined." ; \
	fi

OS:
	if u3b ; then \
		cd u3b20; $(MAKE) -f vpm.mk OS ; \
	else \
		echo "Target OS undefined." ; \
	fi

CMDS:
	if u3b ; then \
		cd u3b20; $(MAKE) -f vpm.mk CMDS ; \
	else \
		echo "Target CMDS undefined." ; \
	fi

vpmc:
	if u3b ; then \
		echo "Target vpmc undefined." ; \
	else \
		cd dec; $(MAKE) -f vpm.mk vpmc ; \
	fi

vpmstart:
	if u3b ; then \
		echo "Target vpmstart undefined." ; \
	else \
		cd dec; $(MAKE) -f vpm.mk vpmstart ; \
	fi

vpmset:
	if u3b ; then \
		echo "Target vpmset undefined." ; \
	else \
		cd dec; $(MAKE) -f vpm.mk vpmset ; \
	fi

dmkset:
	if u3b ; then \
		echo "Target dmkset undefined." ; \
	else \
		cd dec; $(MAKE) -f vpm.mk dmkset ; \
	fi

vpmsave:
	if u3b ; then \
		echo "Target vpmsave undefined." ; \
	else \
		cd dec; $(MAKE) -f vpm.mk vpmsave ; \
	fi

vpmfmt:
	if u3b ; then \
		echo "Target vpmfmt undefined." ; \
	else \
		cd dec; $(MAKE) -f vpm.mk vpmfmt ; \
	fi

vpmtest:
	if u3b ; then \
		echo "Target vpmtest undefined." ; \
	else \
		cd dec; $(MAKE) -f vpm.mk vpmtest ; \
	fi

vpmfsr:
	if u3b ; then \
		echo "Target vpmfsr undefined." ; \
	else \
		cd dec; $(MAKE) -f vpm.mk vpmfsr ; \
	fi

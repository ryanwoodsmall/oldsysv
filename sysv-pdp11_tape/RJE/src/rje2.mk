#	rje2.mk 1.3 of 3/9/82
#	@(#)rje2.mk	1.3

install:
	-mkdir /usr/rje2
	-chmod 755 /usr/rje2
	-chown rje /usr/rje2; chgrp bin /usr/rje2

	-mkdir /usr/rje2/spool
	-chmod 777 /usr/rje2/spool
	-chown rje /usr/rje2/spool; chgrp bin /usr/rje2/spool

	-mkdir /usr/rje2/rpool
	-chmod 755 /usr/rje2/rpool
	-chown rje /usr/rje2/rpool; chgrp bin /usr/rje2/rpool

	-mkdir /usr/rje2/squeue
	-chmod 755 /usr/rje2/squeue
	-chown rje /usr/rje2/squeue; chgrp bin /usr/rje2/squeue

	-mkdir /usr/rje2/job
	-chmod 777 /usr/rje2/job
	-chown rje /usr/rje2/job; chgrp bin /usr/rje2/job

	-rm -f /usr/rje2/rje2disp /usr/rje2/rje2halt /usr/rje2/rje2init
	-rm -f /usr/rje2/rje2qer /usr/rje2/rje2recv /usr/rje2/rje2xmit
	-rm -f /usr/rje2/rje2load /usr/rje2/cvt /usr/rje2/snoop*

	ln /usr/rje/rjedisp /usr/rje2/rje2disp
	ln /usr/rje/rjehalt /usr/rje2/rje2halt
	ln /usr/rje/rjeinit /usr/rje2/rje2init
	ln /usr/rje/rjeqer /usr/rje2/rje2qer
	ln /usr/rje/rjerecv /usr/rje2/rje2recv
	ln /usr/rje/rjexmit /usr/rje2/rje2xmit
	ln /usr/rje/rjeload /usr/rje2/rje2load
	ln /usr/rje/cvt /usr/rje2/cvt
	ln /usr/rje/snoop /usr/rje2/snoop1
	cp /usr/src/cmd/rje/src/testjob1 /usr/rje2/testjob1
	-chown rje /usr/rje2/testjob1; chgrp bin /usr/rje2/testjob1
	cp /usr/src/cmd/rje/src/testjob2 /usr/rje2/testjob2
	-chown rje /usr/rje2/testjob2; chgrp bin /usr/rje2/testjob2

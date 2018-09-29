#	rje1.mk 1.3 of 3/9/82
#	@(#)rje1.mk	1.3

install:
	-mkdir /usr/rje1
	-chmod 755 /usr/rje1
	-chown rje /usr/rje1; chgrp bin /usr/rje1

	-mkdir /usr/rje1/spool
	-chmod 777 /usr/rje1/spool
	-chown rje /usr/rje1/spool; chgrp bin /usr/rje1/spool

	-mkdir /usr/rje1/rpool
	-chmod 755 /usr/rje1/rpool
	-chown rje /usr/rje1/rpool; chgrp bin /usr/rje1/rpool

	-mkdir /usr/rje1/squeue
	-chmod 755 /usr/rje1/squeue
	-chown rje /usr/rje1/squeue; chgrp bin /usr/rje1/squeue

	-mkdir /usr/rje1/job
	-chmod 777 /usr/rje1/job
	-chown rje /usr/rje1/job; chgrp bin /usr/rje1/job

	-rm -f /usr/rje1/rje1disp /usr/rje1/rje1halt /usr/rje1/rje1init
	-rm -f /usr/rje1/rje1qer /usr/rje1/rje1recv /usr/rje1/rje1xmit
	-rm -f /usr/rje1/rje1load /usr/rje1/cvt /usr/rje1/snoop*

	ln /usr/rje/rjedisp /usr/rje1/rje1disp
	ln /usr/rje/rjehalt /usr/rje1/rje1halt
	ln /usr/rje/rjeinit /usr/rje1/rje1init
	ln /usr/rje/rjeqer /usr/rje1/rje1qer
	ln /usr/rje/rjerecv /usr/rje1/rje1recv
	ln /usr/rje/rjexmit /usr/rje1/rje1xmit
	ln /usr/rje/rjeload /usr/rje1/rje1load
	ln /usr/rje/cvt /usr/rje1/cvt
	ln /usr/rje/snoop /usr/rje1/snoop0
	cp /usr/src/cmd/rje/src/testjob1 /usr/rje1/testjob1
	-chown rje /usr/rje1/testjob1; chgrp bin /usr/rje1/testjob1
	cp /usr/src/cmd/rje/src/testjob2 /usr/rje1/testjob2
	-chown rje /usr/rje1/testjob2; chgrp bin /usr/rje1/testjob2

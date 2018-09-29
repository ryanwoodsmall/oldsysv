#	rje4.mk 1.3 of 3/9/82
#	@(#)rje4.mk	1.3

install:
	-mkdir /usr/rje4
	-chmod 755 /usr/rje4
	-chown rje /usr/rje4; chgrp bin /usr/rje4

	-mkdir /usr/rje4/spool
	-chmod 777 /usr/rje4/spool
	-chown rje /usr/rje4/spool; chgrp bin /usr/rje4/spool

	-mkdir /usr/rje4/rpool
	-chmod 755 /usr/rje4/rpool
	-chown rje /usr/rje4/rpool; chgrp bin /usr/rje4/rpool

	-mkdir /usr/rje4/squeue
	-chmod 755 /usr/rje4/squeue
	-chown rje /usr/rje4/squeue; chgrp bin /usr/rje4/squeue

	-mkdir /usr/rje4/job
	-chmod 777 /usr/rje4/job
	-chown rje /usr/rje4/job; chgrp bin /usr/rje4/job

	-rm -f /usr/rje4/rje4disp /usr/rje4/rje4halt /usr/rje4/rje4init
	-rm -f /usr/rje4/rje4qer /usr/rje4/rje4recv /usr/rje4/rje4xmit
	-rm -f /usr/rje4/rje4load /usr/rje4/cvt /usr/rje4/snoop*

	ln /usr/rje/rjedisp /usr/rje4/rje4disp
	ln /usr/rje/rjehalt /usr/rje4/rje4halt
	ln /usr/rje/rjeinit /usr/rje4/rje4init
	ln /usr/rje/rjeqer /usr/rje4/rje4qer
	ln /usr/rje/rjerecv /usr/rje4/rje4recv
	ln /usr/rje/rjexmit /usr/rje4/rje4xmit
	ln /usr/rje/rjeload /usr/rje4/rje4load
	ln /usr/rje/cvt /usr/rje4/cvt
	ln /usr/rje/snoop /usr/rje4/snoop3
	cp /usr/src/cmd/rje/src/testjob1 /usr/rje4/testjob1
	-chown rje /usr/rje4/testjob1; chgrp bin /usr/rje4/testjob1
	cp /usr/src/cmd/rje/src/testjob2 /usr/rje4/testjob2
	-chown rje /usr/rje4/testjob2; chgrp bin /usr/rje4/testjob2

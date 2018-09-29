#	send.mk 1.1 of 9/30/81
#	@(#)send.mk	1.1

send:
	@if [ -f /usr/bin/gath ]; then \
	ln /usr/bin/gath /usr/bin/send; \
	else \
	echo "** error in the order of making of cmds: \
	'gath' must be made before 'rje' **"; \
	fi

install:	send

clean:

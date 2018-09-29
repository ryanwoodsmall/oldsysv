#	@(#)mail.mk	1.3
#	mail make file
ROOT=
INSDIR = $(ROOT)/bin
LDFLAGS = -O -s
INS=:

mail:
	$(CC) $(LDFLAGS) -o mail mail.c
	$(INS) $(INSDIR) -m 2755 -g mail -u bin mail
	if [ "$(INS)" != ":" ]; \
	then \
		rm -f $(INSDIR)/rmail; \
		ln $(INSDIR)/mail $(INSDIR)/rmail; \
	fi

all:	install clobber

install:
	$(MAKE) -f mail.mk INS="install -f"

clean:
	-rm -f *.o

clobber: clean
	rm -f mail

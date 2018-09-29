
#	@(#)rc.sh	1.14

TZ=EST5EDT
export TZ

if [ ! -f /etc/mnttab ]
then
	/etc/devnm / | grep -v swap | grep -v root | /etc/setmnt
fi
set `who -r`
if [ $7 = 2 ]
then
	# put mounts here (/usr, etc.)
	/usr/lib/expreserve -
	rm -f /tmp/*
	rm -f /usr/spool/uucp/LCK*
	rm -f /usr/adm/acct/nite/lock*
	mv /usr/adm/sulog /usr/adm/OLDsulog
	mv /usr/lib/cron/log /usr/lib/cron/OLDlog
	> /usr/lib/cron/log
	/etc/cron
	echo cron started
	/bin/su - adm -c /usr/lib/acct/startup
	echo process accounting started
	/usr/lib/errdemon
	echo errdemon started
#	/bin/su - sys -c "/usr/lib/sa/sadc /usr/adm/sa/sa`date +%d`"
fi

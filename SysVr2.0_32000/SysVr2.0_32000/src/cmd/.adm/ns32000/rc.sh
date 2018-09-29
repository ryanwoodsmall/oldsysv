
#	@(#)rc.sh	1.12

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
	/etc/mount /dev/dsk/1s3 /usr
	/usr/lib/ex3.9preserve -
	rm -fr /tmp/*
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
# Delay to give daemons a chance to run before rc exits
	sleep 2
fi

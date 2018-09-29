
#	@(#)uudemon.hr.sh	1.4

# 'perform every hour on the 56-minute mark'
PATH=:/usr/lib/uucp:/bin:/usr/bin
cd /usr/lib/uucp
uucico -r1
uulog

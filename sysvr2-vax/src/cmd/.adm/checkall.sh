#	@(#)checkall.sh	1.2
# Warning: this is only a sample.
fsck $* /dev/dsk/0s0
dfsck -T/tmp/1 /dev/rdsk/1s1 - /dev/rdsk/0s[12345]

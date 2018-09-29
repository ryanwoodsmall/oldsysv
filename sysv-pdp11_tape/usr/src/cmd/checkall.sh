#	@(#)checkall.sh	1.1
# Warning: this is only a sample.
fsck $* /dev/rp0
dfsck -T/tmp/1 /dev/rrp11 - /dev/rrp[12345]

#	@(#)brc.sh	1.3

# ***** This command file's function is to remove the mnttab

if
	[ -r /etc/mnttab ]
then
	rm -f /etc/mnttab
fi

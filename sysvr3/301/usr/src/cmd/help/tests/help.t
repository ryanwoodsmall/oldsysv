#ident	"@(#)help:tests/help.t	1.1"
# This tests the new getopt change in help, mr bl85-19625

#1.0
help usage -d cat <<!
q
!
#1.1
help usage cat <<!
q
!
#1.2
help usage -e cat <<!
q
!
#1.3
help usage -o cat <<!
q
!
#1.4
help usage <<!
q
!

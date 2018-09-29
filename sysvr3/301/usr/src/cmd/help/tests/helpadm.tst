#ident	"@(#)help:tests/helpadm.tst	1.2"
echo "You must be super user to execute this acceptance test"
echo "and you must have your SCROLL variable set to no and exported."

ROOT=/cmds/cmds/REALHELP
HELPADM=${ROOT}/etc/helpadm
HELP=${ROOT}/usr/bin/help
$HELPADM <<!  >> helpadm.$$ 2>&1
1
c
/mkdir
s/mkdir     /mkdir junk/
w
q
e
y
!
$HELP <<! >> helpadm.$$ 2>&1
s
c
q
!
$HELPADM <<! >> helpadm.$$ 2>&1
1
c
/junk
s/junk/    /
w
q
e
y
!
$HELP <<! >> helpadm.$$ 2>&1
s
c
q
!
$HELPADM <<! >> helpadm.$$ 2>&1
2
junk
a

This is a junk file for acceptance testing
.
w
q
y
y
!
$HELP <<! >> helpadm.$$ 2>&1
g
junk
q
!
$HELPADM <<! >> helpadm.$$ 2>&1
2
junk
d
y
!
$HELP <<! >> helpadm.$$ 2>&1
g
junk
q
!
$HELPADM <<! >> helpadm.$$ 2>&1
3
junk
a

This is the description for junk for the acceptance test.
.
w
q
y
a

This is the options screen for junk for the acceptance test.
.
w
q
y
a

This is the examples screen for junk for the acceptance test.
.
w
q
y
garbage
.
y
y
!
$HELP <<! >> helpadm.$$ 2>&1
l
k
garbage
c
junk
d
e
o
q
!
$HELPADM <<! >> helpadm.$$ 2>&1
3
junk
y
y
!
$HELP <<! >> helpadm.$$ 2>&1
l
k
garbage
c
junk
q
!
$HELPADM <<! >> helpadm.$$ 2>&1
4
!
ls -l ${ROOT}/usr/lib/help/HELPLOG | sed 's/^\(..........\).*/\1/' >> helpadm.$$ 2>&1
$HELPADM <<! >> helpadm.$$ 2>&1
5
!
ls -l ${ROOT}/usr/lib/help/HELPLOG | sed 's/^\(..........\).*/\1/' >> helpadm.$$ 2>&1
echo "\n" >> helpadm.$$
diff helpadm.$$ admfile > admdiff.$$
if [ -s admdiff.$$ ]
then
	echo "Helpadm acceptance test failed!"
else
	echo "Helpadm acceptance test passed!"
fi
rm -fr helpadm.$$ 

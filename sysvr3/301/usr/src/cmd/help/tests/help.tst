#ident	"@(#)help:tests/help.tst	1.2"
echo "You must have your SCROLL variable set to no and exported"
/usr/bin/help <<! > help.$$ 2>&1
s
e
n
b
h
l
k
print
c
pwd
d
e
p
h
u
c
grep
o
h
g
n
kill character
login
n
h
q
!
echo "\n" >> help.$$
diff help.$$ helpfile > helpdiff.$$
if [ -s helpdiff.$$ ]
then
	echo "Help acceptance test failed!"
else
	echo "Help acceptance test passed!"
fi
rm -rf help.$$ helpdiff.$$

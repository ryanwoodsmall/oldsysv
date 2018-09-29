#	This is a prototype file for tapesaves of file systems.
#	It may need to be modified for local use.
#	It will copy the named file system onto tape drive 0.

echo 'Enter file system name (e.g., usr):'
read f
echo 'Enter device name (e.g., /dev/rdsk/1s0):'
read d
echo 'Enter pack volume label (e.g., p0045):'
read v
t=/dev/rmt/0m
echo 'Enter tape volume label (e.g., t0005):'
read l

/etc/labelit  $t
if test  $? -gt 0
then
  /etc/labelit $t $f  $l -n
  if test $? -gt 0
  then
    exit 1
  fi
fi
/etc/volcopy $f $d $v $t $l

# <@(#)tapesave.sh	1.2>

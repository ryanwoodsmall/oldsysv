# @(#)graphics.sh	1.1
: PWB/Graphics environment change
: GRAF directory optionally used for uselog and
: announcements.
: r option creates restricted environment.
:
GRAF=/a2/graf

if test "$TERM" = 4014
then /usr/bin/graf/tekset
fi

if test -w $GRAF/.uselog
then
  if  tty>/dev/null
  then
    tty=`tty | sed "s/.*\///"`
    who | grep ${tty} >>${GRAF}/.uselog
  else
    who are you >>${GRAF}/.uselog
  fi
fi

if test -r ${GRAF}/announce
then  cat ${GRAF}/announce
fi

if test "$1" != "-r"
then
  PATH=$PATH:/usr/bin/graf PS1="^ " sh $@
else
  PATH=:/rbin:/usr/rbin:/bin:/usr/bin:/usr/bin/graf PS1="^ " rsh $@
fi

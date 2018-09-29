export PATH PS1 TERM IDENTIFY VMSYS OASYS
PS1='UNIX: '
VMSYS=`sed -n -e '/^vmsys:/s/^.*:\([^:][^:]*\):[^:]*$/\1/p' < /etc/passwd`
OASYS=`sed -n -e '/^oasys:/s/^.*:\([^:][^:]*\):[^:]*$/\1/p' < /etc/passwd`
PATH=:/bin:/usr/bin:$VMSYS/lbin:$VMSYS/bin:$OASYS/bin:$HOME/bin
export `sed 's/=.*//' < $HOME/pref/.variables`
IDENTIFY=$OASYS/bin
. $HOME/pref/.variables
stty erase \^H kill \^U echoe echok
while :
do
	if [ -z "$TERM" ]
	then

		echo "TERM=\c"
		read TERM
		if [ -z "$TERM" ]
		then
			eval "`termtest`"
			if [ -z "$TERM" ]
			then
				echo "Cannot determine terminal type - please enter terminal name"
				continue
			fi
		fi
	fi
	if [ -r /usr/lib/terminfo/?/"$TERM" ]
	then
		break
	else
		echo "The terminal '$TERM' is unknown - please try a different response"
		TERM=""
	fi
done
if [ -x /usr/bin/tabs ]
then
	/usr/bin/tabs
fi

if [ -x /usr/bin/tput ]
then
	tput init
fi
exec face

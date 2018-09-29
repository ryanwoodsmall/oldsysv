form="Modify Service"
done=`message ""; modserv "$F1" "$F2" "$F3" "$F4" "HOME" "$OF1" "$OF2" "$OF3" "$OF4"` close $VMSYS/OBJECTS/Form.mod $VMSYS/OBJECTS/Menu.list
help=OPEN TEXT OBJECTS/Text.mfhelp  T.mod "Modify Services"
`grep TERM= $HOME/bin/${ARG1}.ins | cut -d= -f2 | cut -d";" -f1 | set -l OF1; set -l OF2=${ARG1};
tail -1 $HOME/bin/${ARG1}.ins | set -l OF3;grep cd $HOME/bin/${ARG1}.ins | cut -d" " -f2 | set -l OF4`

name=Terminal Type:
nrow=1
ncol=1
rows=1
columns=14
frow=1
fcol=20
value=`grep TERM= $HOME/bin/${ARG1}.ins | cut -d= -f2 | cut -d";" -f1`
valid=`message "Please wait validating";$VMSYS/bin/chkterm ${F1}`
choicemsg=const "Enter the Terminal Type for command invoked, default is your terminal type"
invalidmsg=const "Terminfo entry ${F1} doesn't exist"
fieldmsg=const "Enter the correct Terminal type needed for the command invoked"

name=Service Menu Name:
nrow=2
ncol=1
rows=1
columns=10
frow=2
fcol=20
value=${ARG1}
valid=`message "Please wait validating";shell test -n ${F2} && $VMSYS/bin/mnucheck -s "${F2}" -n "${ARG1}"`
choicemsg=const "enter the name of the menu item as it appears in The Service Menu"
invalidmsg=`regex -v ${RET}
'2'	"$F2 already exists"
'.*'	"Must enter a valid string"`
fieldmsg=const "Name that appears in the Service Menu"

name=Name of Command:
nrow=3
ncol=1
rows=1
columns=45
frow=3
fcol=20
value=`tail -1 $HOME/bin/${ARG1}.ins`
valid=`message "Please wait validating";regex -v "$F3" '^[a-zA-Z0-9_/]*$' && $VMSYS/bin/chexec "${F3}"`
invalidmsg=`echo "$F3 not in path or an illegal command"`
choicemsg=const "Enter the name of the command to execute or supply a full path name"
fieldmsg=const "Enter the command to execute, full path names are acceptable"
scroll=true

name=Working Directory:
nrow=4
ncol=1
rows=1
columns=45
frow=4
fcol=20
value=`grep cd $HOME/bin/${ARG1}.ins | cut -d" " -f2`
valid=`message "Please wait validating";shell "test -d $F4"`
choicemsg=const "Enter the path to the directory you want to change to when $F2 is invoked"
invalidmsg=const "The Path entered must be a valid directory"
fieldmsg=const "The directory you want to change to when command $F2 invoked"
wrap=true

name=RESET
button=8
action=reset

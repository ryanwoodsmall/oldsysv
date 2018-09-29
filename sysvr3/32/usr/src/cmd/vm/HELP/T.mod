
   MODIFY  SERVICES opens  a menu in which you can select the service
   to be modified. Once a  service  is selected  the  MODIFY SERVICES
   form is opened.

   The  MODIFY SERVICES  form  allows you  to modify the services you
   have  added to  your Services menu. The following list defines the
   fields included in this form.

      - TERMINAL TYPE - This field allows you to enter in a different
      term  type if  the  application doesn't  recognize your default
      term type.  You  must check the manual pages of the application
      or  see your  System  Administrator to map the correct terminal
      type.  Default  is whatever  was  set  for  that  service being
      modified.

      - SERVICE  MENU NAME - This  field  allows  you to enter in the
      name of the menu item as it appears in the services menu.

      - NAME  OF COMMAND - This field allows you to enter in the full
      pathname of  the program  to  execute.  A check is made to warn
      you if  it cannot  find the program you have named.  Default is
      the command name of the service being modified.

      - WORKING DIRECTORY - This  field allows  you to enter the path
      to  change to when this service is invoked.  The default is the
      working directory of the service being modified.

    Function Keys active for this form (in addition to PREV-FRM,
    NEXT-FRM, CMD-MENU, and HELP) are:

           - SAVE -- Saves all the field values entered in  the  form,
             closes  the  frame,  and returns you to the point in the
             interface  that you  were at prior to accessing the form.
	     Saving the form causes your services menu to be modified.

           - CANCEL -- Cancels  the form without  saving the new field
             values  entered  in  the  form,  closes   the  frame, and
             returns you to the point in the interface that  you  were
             at prior to accessing the form.

           - CHOICES  --  For  the  field  where  the  cursor  resides
             pressing  CHOICES  either  toggles  between  the possible
             values for the  field,  or  displays  a  pop-up  menu  of
             available  choices.  For fields requiring you to set your
             own values a definition of  the  field  requirement  will
             appear on the message line.

           - RESET - resets the field back to the value(s)  in  effect
             at the beginning of the current editing session.

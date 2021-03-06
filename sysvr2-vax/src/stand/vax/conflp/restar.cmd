! @(#)restar.cmd	1.1
!
!	RESTART COMMMAND FILE - RESTAR.CMD
!
!	THIS COMMAND FILE IS INVOKED IN THE EVENT OF POWER RECOVERY AND
!	OTHER CONSOLE DETECTED RESTART CONDITIONS IF THE AUTO RESTART SWITCH
!	IS SET.  IT CAN ALSO BE INVOKED MANUALLY WITH THE COMMAND:
!				@RESTAR.CMD
!
HALT			! HALT PROCESSOR
UNJAM			! UNJAM SBI
INIT			! INITIALIZE PROCESSOR
D/I 11 20003800	! SET ADDRESS OF SCB BASE
D R0 0 		! CLEAR UNUSED REGISTERS
D R1 8		! MBA TR=8
D R2 0		! CLEAR UNUSED REGISTER
D R3 0 		! CLEAR UNUSED REGISTER
D R4 0		! CLEAR UNUSED REGISTER
D R5 0		! CLEAR UNUSED REGISTER
D FP 0		! NO MACHINE CHECK EXPECTED
START 20003004		! START RESTART REFEREE

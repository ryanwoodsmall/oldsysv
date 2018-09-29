/	@(#)syscall.s	1.3
/ C library -- general system call
/	syscall(sysnum, r0, r1, arg1, arg2, ...)
/		max of 5 args (size u.u_arg in system)

.globl	_syscall, cerror, csav, cret

_syscall:
	MCOUNT
	jsr	r0,csav
	mov	r5,r2
	add	$4,r2
	mov	$9f,r3
	mov	(r2)+,r0
	bic	$!377,r0
	bis	$104400,r0
	mov	r0,(r3)+
	mov	(r2)+,r0
	mov	(r2)+,r1
	mov	(r2)+,(r3)+
	mov	(r2)+,(r3)+
	mov	(r2)+,(r3)+
	mov	(r2)+,(r3)+
	mov	(r2)+,(r3)+
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	jmp	cret

.data
9:
	sys	0; .=.+10.

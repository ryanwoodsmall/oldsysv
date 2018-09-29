*
* dev	vector	addr	bus	count
*
dc	3	d00000	5
tc	4	d00200	5
lp	12	fff800	4
sio	14	a00000	4
*
* Pseudo Devices
*
sxt	0	0	0	20
*
* System Devices
*
root	dc	10
swap	dc	11	0	5040
pipe	dc	10
dump	tc	0	0
*
* Tunable Parameters
*
buffers	200
inodes	175
files	175
mounts	8
calls	50
procs	65
clists	150
regions 163
mesg	1
sema	1
shmem	1

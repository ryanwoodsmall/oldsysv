is:s:initdefault:
bl::bootwait:/etc/bcheckrc </dev/console >/dev/console 2>&1 #bootlog
bc::bootwait:/etc/brc 1>/dev/console 2>&1 #bootrun command
sl::wait:(rm -f /dev/syscon;ln /dev/systty /dev/syscon;) 1>/dev/console 2>&1
rc::wait:/etc/rc 1>/dev/console 2>&1 #run com
pf::powerfail:/etc/powerfail 1>/dev/console 2>&1 #power fail routines
co::respawn:/etc/getty console console
t0:2:respawn:/etc/getty tty00 9600
t1:2:respawn:/etc/getty tty01 9600
t2:2:respawn:/etc/getty tty02 9600
t3:2:respawn:/etc/getty tty03 9600
t4:2:respawn:/etc/getty tty04 9600
t5:2:respawn:/etc/getty tty05 9600
t6:2:respawn:/etc/getty tty06 9600
t7:2:respawn:/etc/getty tty07 9600
ls::wait:/usr/lib/lpshut >/dev/null 2>&1 #Clean up lp Lock File
lp:2:once:/usr/lib/lpsched >/dev/console 2>&1 #print scheduler

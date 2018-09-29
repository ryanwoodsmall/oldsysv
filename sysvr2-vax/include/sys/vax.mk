# @(#)vax.mk	6.1
all: /usr/include/vax/sys/buf.h \
/usr/include/vax/sys/callo.h \
/usr/include/vax/sys/clock.h \
/usr/include/vax/sys/conf.h \
/usr/include/vax/sys/cons.h \
/usr/include/vax/sys/dmcio.h \
/usr/include/vax/sys/dz.kmc.h \
/usr/include/vax/sys/filsys.h \
/usr/include/vax/sys/gdisk.h \
/usr/include/vax/sys/init.h \
/usr/include/vax/sys/inode.h \
/usr/include/vax/sys/io.h \
/usr/include/vax/sys/iobuf.h \
/usr/include/vax/sys/map.h \
/usr/include/vax/sys/mba.h \
/usr/include/vax/sys/mem.h \
/usr/include/vax/sys/mtpr.h \
/usr/include/vax/sys/nsc.h \
/usr/include/vax/sys/nscdev.h \
/usr/include/vax/sys/opt.h \
/usr/include/vax/sys/page.h \
/usr/include/vax/sys/param.h \
/usr/include/vax/sys/pcb.h \
/usr/include/vax/sys/proc.h \
/usr/include/vax/sys/psl.h \
/usr/include/vax/sys/reg.h \
/usr/include/vax/sys/seg.h \
/usr/include/vax/sys/shm.h \
/usr/include/vax/sys/space.h \
/usr/include/vax/sys/sysinfo.h \
/usr/include/vax/sys/systm.h \
/usr/include/vax/sys/text.h \
/usr/include/vax/sys/trap.h \
/usr/include/vax/sys/tty.h \
/usr/include/vax/sys/types.h \
/usr/include/vax/sys/uba.h \
/usr/include/vax/sys/user.h \
/usr/include/vax/sys/var.h \
/usr/include/vax/sys/buf.h: buf.h
	cp buf.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/buf.h; ln /usr/include/vax/sys/buf.h /usr/include/sys; fi
/usr/include/vax/sys/callo.h: callo.h
	cp callo.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/callo.h; ln /usr/include/vax/sys/callo.h /usr/include/sys; fi
/usr/include/vax/sys/clock.h: clock.h
	cp clock.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/clock.h; ln /usr/include/vax/sys/clock.h /usr/include/sys; fi
/usr/include/vax/sys/conf.h: conf.h
	cp conf.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/conf.h; ln /usr/include/vax/sys/conf.h /usr/include/sys; fi
/usr/include/vax/sys/cons.h: cons.h
	cp cons.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/cons.h; ln /usr/include/vax/sys/cons.h /usr/include/sys; fi
/usr/include/vax/sys/dmcio.h: dmcio.h
	cp dmcio.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/dmcio.h; ln /usr/include/vax/sys/dmcio.h /usr/include/sys; fi
/usr/include/vax/sys/dz.kmc.h: dz.kmc.h
	cp dz.kmc.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/dz.kmc.h; ln /usr/include/vax/sys/dz.kmc.h /usr/include/sys; fi
/usr/include/vax/sys/filsys.h: filsys.h
	cp filsys.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/filsys.h; ln /usr/include/vax/sys/filsys.h /usr/include/sys; fi
/usr/include/vax/sys/gdisk.h: gdisk.h
	cp gdisk.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/gdisk.h; ln /usr/include/vax/sys/gdisk.h /usr/include/sys; fi
/usr/include/vax/sys/init.h: init.h
	cp init.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/init.h; ln /usr/include/vax/sys/init.h /usr/include/sys; fi
/usr/include/vax/sys/inode.h: inode.h
	cp inode.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/inode.h; ln /usr/include/vax/sys/inode.h /usr/include/sys; fi
/usr/include/vax/sys/io.h: io.h
	cp io.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/io.h; ln /usr/include/vax/sys/io.h /usr/include/sys; fi
/usr/include/vax/sys/iobuf.h: iobuf.h
	cp iobuf.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/iobuf.h; ln /usr/include/vax/sys/iobuf.h /usr/include/sys; fi
/usr/include/vax/sys/map.h: map.h
	cp map.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/map.h; ln /usr/include/vax/sys/map.h /usr/include/sys; fi
/usr/include/vax/sys/mba.h: mba.h
	cp mba.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/mba.h; ln /usr/include/vax/sys/mba.h /usr/include/sys; fi
/usr/include/vax/sys/mem.h: mem.h
	cp mem.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/mem.h; ln /usr/include/vax/sys/mem.h /usr/include/sys; fi
/usr/include/vax/sys/mtpr.h: mtpr.h
	cp mtpr.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/mtpr.h; ln /usr/include/vax/sys/mtpr.h /usr/include/sys; fi
/usr/include/vax/sys/nsc.h: nsc.h
	cp nsc.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/nsc.h; ln /usr/include/vax/sys/nsc.h /usr/include/sys; fi
/usr/include/vax/sys/nscdev.h: nscdev.h
	cp nscdev.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/nscdev.h; ln /usr/include/vax/sys/nscdev.h /usr/include/sys; fi
/usr/include/vax/sys/opt.h: opt.h
	cp opt.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/opt.h; ln /usr/include/vax/sys/opt.h /usr/include/sys; fi
/usr/include/vax/sys/page.h: page.h
	cp page.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/page.h; ln /usr/include/vax/sys/page.h /usr/include/sys; fi
/usr/include/vax/sys/param.h: param.h
	cp param.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/param.h; ln /usr/include/vax/sys/param.h /usr/include/sys; fi
/usr/include/vax/sys/pcb.h: pcb.h
	cp pcb.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/pcb.h; ln /usr/include/vax/sys/pcb.h /usr/include/sys; fi
/usr/include/vax/sys/proc.h: proc.h
	cp proc.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/proc.h; ln /usr/include/vax/sys/proc.h /usr/include/sys; fi
/usr/include/vax/sys/psl.h: psl.h
	cp psl.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/psl.h; ln /usr/include/vax/sys/psl.h /usr/include/sys; fi
/usr/include/vax/sys/reg.h: reg.h
	cp reg.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/reg.h; ln /usr/include/vax/sys/reg.h /usr/include/sys; fi
/usr/include/vax/sys/seg.h: seg.h
	cp seg.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/seg.h; ln /usr/include/vax/sys/seg.h /usr/include/sys; fi
/usr/include/vax/sys/shm.h: shm.h
	cp shm.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/shm.h; ln /usr/include/vax/sys/shm.h /usr/include/sys; fi
/usr/include/vax/sys/space.h: space.h
	cp space.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/space.h; ln /usr/include/vax/sys/space.h /usr/include/sys; fi
/usr/include/vax/sys/sysinfo.h: sysinfo.h
	cp sysinfo.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/sysinfo.h; ln /usr/include/vax/sys/sysinfo.h /usr/include/sys; fi
/usr/include/vax/sys/systm.h: systm.h
	cp systm.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/systm.h; ln /usr/include/vax/sys/systm.h /usr/include/sys; fi
/usr/include/vax/sys/text.h: text.h
	cp text.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/text.h; ln /usr/include/vax/sys/text.h /usr/include/sys; fi
/usr/include/vax/sys/trap.h: trap.h
	cp trap.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/trap.h; ln /usr/include/vax/sys/trap.h /usr/include/sys; fi
/usr/include/vax/sys/tty.h: tty.h
	cp tty.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/tty.h; ln /usr/include/vax/sys/tty.h /usr/include/sys; fi
/usr/include/vax/sys/types.h: types.h
	cp types.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/types.h; ln /usr/include/vax/sys/types.h /usr/include/sys; fi
/usr/include/vax/sys/uba.h: uba.h
	cp uba.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/uba.h; ln /usr/include/vax/sys/uba.h /usr/include/sys; fi
/usr/include/vax/sys/user.h: user.h
	cp user.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/user.h; ln /usr/include/vax/sys/user.h /usr/include/sys; fi
/usr/include/vax/sys/var.h: var.h
	cp var.h /usr/include/vax/sys
	set +e; if vax; then rm -f /usr/include/sys/var.h; ln /usr/include/vax/sys/var.h /usr/include/sys; fi

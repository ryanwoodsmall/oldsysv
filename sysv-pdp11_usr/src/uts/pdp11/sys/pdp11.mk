all: /usr/include/pdp11/sys/buf.h \
/usr/include/pdp11/sys/callo.h \
/usr/include/pdp11/sys/conf.h \
/usr/include/pdp11/sys/csihdw.h \
/usr/include/pdp11/sys/da.h \
/usr/include/pdp11/sys/dmcio.h \
/usr/include/pdp11/sys/du.h \
/usr/include/pdp11/sys/dz.kmc.h \
/usr/include/pdp11/sys/filsys.h \
/usr/include/pdp11/sys/gdisk.h \
/usr/include/pdp11/sys/init.h \
/usr/include/pdp11/sys/inode.h \
/usr/include/pdp11/sys/io.h \
/usr/include/pdp11/sys/iobuf.h \
/usr/include/pdp11/sys/map.h \
/usr/include/pdp11/sys/maus.h \
/usr/include/pdp11/sys/nsc.h \
/usr/include/pdp11/sys/nscdev.h \
/usr/include/pdp11/sys/opt.h \
/usr/include/pdp11/sys/param.h \
/usr/include/pdp11/sys/proc.h \
/usr/include/pdp11/sys/psl.h \
/usr/include/pdp11/sys/reg.h \
/usr/include/pdp11/sys/seg.h \
/usr/include/pdp11/sys/shm.h \
/usr/include/pdp11/sys/space.h \
/usr/include/pdp11/sys/sysinfo.h \
/usr/include/pdp11/sys/systm.h \
/usr/include/pdp11/sys/text.h \
/usr/include/pdp11/sys/trap.h \
/usr/include/pdp11/sys/tty.h \
/usr/include/pdp11/sys/types.h \
/usr/include/pdp11/sys/uba.h \
/usr/include/pdp11/sys/user.h \
/usr/include/pdp11/sys/var.h
/usr/include/pdp11/sys/buf.h: buf.h
	cp buf.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/buf.h; ln /usr/include/pdp11/sys/buf.h /usr/include/sys; fi
/usr/include/pdp11/sys/callo.h: callo.h
	cp callo.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/callo.h; ln /usr/include/pdp11/sys/callo.h /usr/include/sys; fi
/usr/include/pdp11/sys/conf.h: conf.h
	cp conf.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/conf.h; ln /usr/include/pdp11/sys/conf.h /usr/include/sys; fi
/usr/include/pdp11/sys/csihdw.h: csihdw.h
	cp csihdw.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/csihdw.h; ln /usr/include/pdp11/sys/csihdw.h /usr/include/sys; fi
/usr/include/pdp11/sys/da.h: da.h
	cp da.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/da.h; ln /usr/include/pdp11/sys/da.h /usr/include/sys; fi
/usr/include/pdp11/sys/dmcio.h: dmcio.h
	cp dmcio.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/dmcio.h; ln /usr/include/pdp11/sys/dmcio.h /usr/include/sys; fi
/usr/include/pdp11/sys/du.h: du.h
	cp du.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/du.h; ln /usr/include/pdp11/sys/du.h /usr/include/sys; fi
/usr/include/pdp11/sys/dz.kmc.h: dz.kmc.h
	cp dz.kmc.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/dz.kmc.h; ln /usr/include/pdp11/sys/dz.kmc.h /usr/include/sys; fi
/usr/include/pdp11/sys/filsys.h: filsys.h
	cp filsys.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/filsys.h; ln /usr/include/pdp11/sys/filsys.h /usr/include/sys; fi
/usr/include/pdp11/sys/gdisk.h: gdisk.h
	cp gdisk.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/gdisk.h; ln /usr/include/pdp11/sys/gdisk.h /usr/include/sys; fi
/usr/include/pdp11/sys/init.h: init.h
	cp init.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/init.h; ln /usr/include/pdp11/sys/init.h /usr/include/sys; fi
/usr/include/pdp11/sys/inode.h: inode.h
	cp inode.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/inode.h; ln /usr/include/pdp11/sys/inode.h /usr/include/sys; fi
/usr/include/pdp11/sys/io.h: io.h
	cp io.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/io.h; ln /usr/include/pdp11/sys/io.h /usr/include/sys; fi
/usr/include/pdp11/sys/iobuf.h: iobuf.h
	cp iobuf.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/iobuf.h; ln /usr/include/pdp11/sys/iobuf.h /usr/include/sys; fi
/usr/include/pdp11/sys/map.h: map.h
	cp map.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/map.h; ln /usr/include/pdp11/sys/map.h /usr/include/sys; fi
/usr/include/pdp11/sys/maus.h: maus.h
	cp maus.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/maus.h; ln /usr/include/pdp11/sys/maus.h /usr/include/sys; fi
/usr/include/pdp11/sys/nsc.h: nsc.h
	cp nsc.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/nsc.h; ln /usr/include/pdp11/sys/nsc.h /usr/include/sys; fi
/usr/include/pdp11/sys/nscdev.h: nscdev.h
	cp nscdev.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/nscdev.h; ln /usr/include/pdp11/sys/nscdev.h /usr/include/sys; fi
/usr/include/pdp11/sys/opt.h: opt.h
	cp opt.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/opt.h; ln /usr/include/pdp11/sys/opt.h /usr/include/sys; fi
/usr/include/pdp11/sys/param.h: param.h
	cp param.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/param.h; ln /usr/include/pdp11/sys/param.h /usr/include/sys; fi
/usr/include/pdp11/sys/proc.h: proc.h
	cp proc.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/proc.h; ln /usr/include/pdp11/sys/proc.h /usr/include/sys; fi
/usr/include/pdp11/sys/psl.h: psl.h
	cp psl.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/psl.h; ln /usr/include/pdp11/sys/psl.h /usr/include/sys; fi
/usr/include/pdp11/sys/reg.h: reg.h
	cp reg.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/reg.h; ln /usr/include/pdp11/sys/reg.h /usr/include/sys; fi
/usr/include/pdp11/sys/seg.h: seg.h
	cp seg.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/seg.h; ln /usr/include/pdp11/sys/seg.h /usr/include/sys; fi
/usr/include/pdp11/sys/shm.h: shm.h
	cp shm.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/shm.h; ln /usr/include/pdp11/sys/shm.h /usr/include/sys; fi
/usr/include/pdp11/sys/space.h: space.h
	cp space.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/space.h; ln /usr/include/pdp11/sys/space.h /usr/include/sys; fi
/usr/include/pdp11/sys/sysinfo.h: sysinfo.h
	cp sysinfo.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/sysinfo.h; ln /usr/include/pdp11/sys/sysinfo.h /usr/include/sys; fi
/usr/include/pdp11/sys/systm.h: systm.h
	cp systm.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/systm.h; ln /usr/include/pdp11/sys/systm.h /usr/include/sys; fi
/usr/include/pdp11/sys/text.h: text.h
	cp text.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/text.h; ln /usr/include/pdp11/sys/text.h /usr/include/sys; fi
/usr/include/pdp11/sys/trap.h: trap.h
	cp trap.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/trap.h; ln /usr/include/pdp11/sys/trap.h /usr/include/sys; fi
/usr/include/pdp11/sys/tty.h: tty.h
	cp tty.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/tty.h; ln /usr/include/pdp11/sys/tty.h /usr/include/sys; fi
/usr/include/pdp11/sys/types.h: types.h
	cp types.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/types.h; ln /usr/include/pdp11/sys/types.h /usr/include/sys; fi
/usr/include/pdp11/sys/uba.h: uba.h
	cp uba.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/uba.h; ln /usr/include/pdp11/sys/uba.h /usr/include/sys; fi
/usr/include/pdp11/sys/user.h: user.h
	cp user.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/user.h; ln /usr/include/pdp11/sys/user.h /usr/include/sys; fi
/usr/include/pdp11/sys/var.h: var.h
	cp var.h /usr/include/pdp11/sys
	set +e; if pdp11; then rm -f /usr/include/sys/var.h; ln /usr/include/pdp11/sys/var.h /usr/include/sys; fi

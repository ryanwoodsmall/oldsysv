all:  xstr mkstr fold ctags
	
mkstr: mkstr.c
	cc mkstr.c
	mv a.out mkstr

xstr:  xstr.c
	cc xstr.c
	mv a.out xstr

fold:  fold.c
	cc fold.c
	mv a.out fold

ctags: ctags.c
	cc ctags.c
	mv a.out ctags

clean:
	rm -f a.out ctags xstr mkstr fold

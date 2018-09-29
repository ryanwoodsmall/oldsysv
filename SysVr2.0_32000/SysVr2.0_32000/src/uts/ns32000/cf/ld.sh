set -v
/bin/ld -o ../unix ld.file -e start -x ../locore.o conf.o low.o linesw.o ../lib? name.o

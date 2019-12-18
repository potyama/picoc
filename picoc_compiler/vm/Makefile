CC = cc
INCLUDEDIR = -I. -I..
CFLAGS = -O2  -g ${INCLUDEDIR}

CP = cp
RM = rm

HEADERS = ../define.h label_table.h vm.h

OBJECTS = label_table.o vm.o

BINDIR = ${HOME}/bin

.SUFFIXES: .c .o

.c.o:${HEADERS}
	${CC} ${CFLAGS} -c $<

all: test_vm

test_vm: ${OBJECTS} test_vm.o
	${CC} ${CFLAGS} test_vm.o ${OBJECTS} -o test_vm

install:
	mkdir -p ${BINDIR}
	cp test_vm ${BINDIR}

clean:
	${RM} -f *.o

hardclean:
	${RM} -f *.o Makefile~ *.*~ test_vm

tar: clean
	cd ..;tar cvzf `basename ${PWD}`.tgz `basename ${PWD}`

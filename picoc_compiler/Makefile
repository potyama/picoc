CC = cc
INCLUDEDIR = -I. 
CFLAGS = -O2  -g ${INCLUDEDIR}

CP = cp
RM = rm

HEADERS = define.h scanner.h parser.h id_table.h \
		id_table_stack.h id_info.h codegen.h

OBJECTS = scanner.o parser.o id_table.o id_table_stack.o codegen.o

BINDIR = ${HOME}/bin

.SUFFIXES: .c .o

all: test

.c.o:${HEADERS}
	${CC} ${CFLAGS} -c $<

test: test_scanner test_parser test_vm

test_scanner: ${OBJECTS} test_scanner.o
	${CC} ${CFLAGS} test_scanner.o ${OBJECTS} -o test_scanner

test_parser: ${OBJECTS} test_parser.o
	${CC} ${CFLAGS} test_parser.o ${OBJECTS} -o test_parser

test_vm:
	cd vm; make test_vm

install:
	mkdir -p ${BINDIR}
	cp test_parser picocc ${BINDIR} 
	chmod +x ${BINDIR}/picocc
	cd vm; make install

clean:
	${RM} -f *.o
	cd vm;make clean

hardclean:
	${RM} -f *.o Makefile~ *.*~ test_scanner test_parser
	cd vm;make hardclean

tar: clean
	cd ..;tar cvzf `basename ${PWD}`.tgz `basename ${PWD}`

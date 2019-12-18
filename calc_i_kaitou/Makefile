CC = cc
INCLUDEDIR = -I. 
CFLAGS = -O2  -g ${INCLUDEDIR}

CP = cp
RM = rm

HEADERS = define.h scanner.h parser.h error.h

.SUFFIXES: .c .o

all: expression_1 expression_2 expression_3 expression_4\
     expression_1i expression_2i expression_3i expression_4i

.c.o:${HEADERS}
	${CC} ${CFLAGS} -c $<

expression_1: expression.o parser_expression_1.o scanner.o error.o
	${CC} ${CFLAGS} -o $@ expression.o parser_expression_1.o scanner.o error.o

expression_2: expression.o parser_expression_2.o scanner.o error.o
	${CC} ${CFLAGS} -o $@ expression.o parser_expression_2.o scanner.o error.o

expression_3: expression.o parser_expression_3.o scanner.o error.o
	${CC} ${CFLAGS} -o $@ expression.o parser_expression_3.o scanner.o error.o

expression_4: expression.o parser_expression_4.o scanner.o error.o
	${CC} ${CFLAGS} -o $@ expression.o parser_expression_4.o scanner.o error.o

expression_1i: expression_i.o parser_expression_1i.o scanner.o error.o
	${CC} ${CFLAGS} -o $@ expression_i.o parser_expression_1i.o scanner.o error.o

expression_2i: expression_i.o parser_expression_2i.o scanner.o error.o
	${CC} ${CFLAGS} -o $@ expression_i.o parser_expression_2i.o scanner.o error.o

expression_3i: expression_i.o parser_expression_3i.o scanner.o error.o
	${CC} ${CFLAGS} -o $@ expression_i.o parser_expression_3i.o scanner.o error.o

expression_4i: expression_i.o parser_expression_4i.o scanner.o error.o
	${CC} ${CFLAGS} -o $@ expression_i.o parser_expression_4i.o scanner.o error.o

clean:
	${RM} -f *.o expression_1 expression_1i expression_2 expression_2i expression_3 expression_3i expression_4 expression_4i

tar: clean
	cd ..;tar cvzf `basename ${PWD}`.tgz `basename ${PWD}`

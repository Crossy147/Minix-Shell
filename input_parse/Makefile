INC=-I../include
CFLAGS=$(INC)
HDEPS=../include/siparse.h ../include/config.h siparseutils.h

INSTALL_DIR=../

CSRC=siparseutils.c

all: siparseutils.o y.tab.o lex.yy.o
	ar rcs $(INSTALL_DIR)siparse.a siparseutils.o lex.yy.o y.tab.o 

lex.yy.o: siparse.lex y.tab.o $(HDEPS)
	lex  siparse.lex
	cc $(CFLAGS) lex.yy.c -c 

y.tab.o: siparse.y  $(HDEPS)
	yacc -d siparse.y
	cc $(CFLAGS) y.tab.c -c 

siparseutils.o: siparseutils.c $(HDEPS)
	cc $(CFLAGS) -c $<

clean:
	rm -f lex.yy.c y.tab.c y.tab.h siparse.a  *.o 

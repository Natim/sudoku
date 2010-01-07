CPP=g++
CXXFLAGS=-O3 -g -Wall
LDFLAGS=-L/usr/X11R6/lib -Llib -lgraphlib -lX11

all: sudoku

sudoku: sudoku.o Bitmap.class.o Sudoku.struct.o BoiteDialogue.o
	${CPP} ${CXXFLAGS} -o sudoku $^ ${LDFLAGS}

%.o: %.cpp
	${CPP} ${CXXFLAGS} -c $^

clean:
	rm -fr *~
	rm -fr *.o

mrproper: clean
	rm -fr sudoku

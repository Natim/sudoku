# Dependances : les en-tetes de developpement X11 (paquet libx11-dev)

CXX=g++
CXXFLAGS=-O3 -g -Wall
LDFLAGS=-Llib
LDLIBS=-lgraphlib -lX11

OBJETS=sudoku.o Bitmap.class.o Sudoku.struct.o BoiteDialogue.o Fenetre.o

all: sudoku

sudoku: ${OBJETS}
	${CXX} ${CXXFLAGS} -o $@ $^ ${LDFLAGS} ${LDLIBS}

%.o: %.cpp
	${CXX} ${CXXFLAGS} -MMD -MP -c $<

-include ${OBJETS:.o=.d}

clean:
	rm -f *~
	rm -f *.o *.d

mrproper: clean
	rm -f sudoku

.PHONY: all clean mrproper

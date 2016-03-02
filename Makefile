CXX = g++
CXXFLAGS = -Wall
PROG = miny


ifeq ($(shell uname),Darwin)
	LDLIBS = -framework OpenGL -framework GLUT
else
	LDLIBS = -lglut -lGL -lGLU
endif

all: $(PROG)

$(PROG): Timer.o main.o Replay.o Field.o scores.o
	${CXX} -o $@ $^ ${LDLIBS}

main.o: main.cpp 

clean:
	rm -f $(PROG) *.o

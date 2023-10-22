CXX = g++
CXXFLAGS = -Wall -g
PROG = miny


ifeq ($(shell uname),Darwin)
	LDLIBS = -framework OpenGL -framework GLUT
else
	LDLIBS = -lglut -lGL -lGLU
endif

all: $(PROG)

$(PROG): Timer.o main.o Replay.o Field.o scores.o common.o Player.o Action.o ReadMe.o
	${CXX} -o $@ $^ ${LDLIBS}

main.o: main.cpp

stringgen: StringGen.cpp
	${CXX} -o $@ $^

ReadMe.cpp: README stringgen
	./stringgen readme < README > ReadMe.cpp

clean:
	rm -f $(PROG) *.o
	rm -f ReadMe.cpp

CC = g++
CFLAGS = -Wall
PROG = miny


ifeq ($(shell uname),Darwin)
	LIBS = -framework OpenGL -framework GLUT
else
	LIBS = -lglut -lGL -lGLU
endif

all: $(PROG)

$(PROG):	conly lonly

#$(SRCS)
#	$(CC) $(CFLAGS) -o $(PROG) *.cpp $(LIBS)

conly: 
	$(CC) $(CFLAGS) -c *.cpp


clean:
	rm -f $(PROG)

lonly:
	$(CC) $(CFLAGS) -o $(PROG) *.o $(LIBS)

CC=g++
INC=-I$(BOOSTINC)

CFLAGS=-c -Wall -std=c++0x -O2
LDFLAGS=-L $(BOOSTLIB) -lboost_program_options
<<<<<<< HEAD
LDLIBS=-L $(BOOSTLIB) -lboost_program_options
=======
>>>>>>> cb3350b8a1ee2cc041e843e0d19ff5fbb07790a7

SOURCES := $(wildcard src/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=faultsim

all: $(EXECUTABLE) doc

$(EXECUTABLE): $(OBJECTS)
<<<<<<< HEAD
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LDLIBS)
=======
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
>>>>>>> cb3350b8a1ee2cc041e843e0d19ff5fbb07790a7

.cpp.o:
	$(CC) $(CFLAGS) $(INC) $< -o $@

clean:
	rm -rf faultsim
	rm -rf src/*.o
	cd doc && make clean

doc:
	cd doc && make


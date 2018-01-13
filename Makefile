CC=g++
INC=-I$(BOOSTINC)

CFLAGS=-c -Wall -std=c++0x -O2
LDFLAGS=-L $(BOOSTLIB) -lboost_program_options
LDLIBS=-L $(BOOSTLIB) -lboost_program_options

SOURCES := $(wildcard src/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=faultsim

all: $(EXECUTABLE) doc

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LDLIBS)

.cpp.o:
	$(CC) $(CFLAGS) $(INC) $< -o $@

clean:
	rm -rf faultsim
	rm -rf src/*.o

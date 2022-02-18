CC=g++
#CFLAGS=-Wall -I. -I/usr/include -std=gnu++0x
CFLAGS=-Wall -I. -I/usr/include -std=c++17
DEPS=gridconv.h gridconv.hpp
OBJ=main.o grid.o gridconv.o map.o
LDDIRS=-L /usr/lib/x86_64-linux-gnu/
LDLIBS=-lstdc++fs -lgd

default: gridconv

%.o: exprtk.hpp %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LDDIRS) $(LDLIBS)

gridconv: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDDIRS) $(LDLIBS)

clean:
	-rm -f $(OBJ)

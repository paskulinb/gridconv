CC=g++
CFLAGS=-Wall -I. -I/usr/include -std=gnu++0x
DEPS=gridconv.h gridconv.hpp
OBJ=main.o grid.o gridconv.o map.o
LDDIRS=-L /usr/lib/i386-linux-gnu/
LDLIBS=-lgd

default: gridconv

%.o: exprtk.hpp %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(LDDIRS) $(LDLIBS)

gridconv: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDDIRS) $(LDLIBS)

clean:
	-rm -f $(OBJ)

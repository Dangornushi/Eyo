PRG    = ./build/eyo
OBJDIR = ./build/obj/
OBJ    = main.o move.o display.o visualMode.o util.o finder.o colorscheme.o

CFLAGS = -g -O2 -std=c++1z
LFLAGS = -lcurses
CC     =  g++ $(CFLAGS)

$(PRG) : $(OBJ)
	$(CC) -o $@ $^ $(LFLAGS)

.cpp.o:
	$(CC) -c $<

clean:
	rm -rf $(PRG) $(OBJDIR)/*.o

./build/colorscheme.o: colorscheme.cpp eyo.hpp
./build/display.o: display.cpp eyo.hpp move.hpp util.hpp finder.hpp
./build/finder.o: finder.cpp eyo.hpp
./build/main.o: main.cpp eyo.hpp move.hpp util.hpp finder.hpp visualMode.hpp
./build/move.o: move.cpp eyo.hpp
./build/util.o: util.cpp eyo.hpp
./build/visualMode.o: visualMode.cpp eyo.hpp

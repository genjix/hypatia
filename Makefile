CC = g++ -Wall -ansi
CSCF = /home/genjix/media/programs/crystalspace-src-1.2.1/

all: obj/engine.o engine.so
	rm obj/engine.o
engine.so: obj/engine.o obj/video.o obj/evensys.o obj/trackpar.o
	export CRYSTAL=$(CSCF)
	$(CC) obj/*.o -shared -lGL -lGLU `sdl-config --cflags --libs` -lGLEW -lavutil -lavformat -lavcodec -lz -lm -lswscale -g `$(CSCF)/cs-config --cflags --libs` -I/usr/include/python2.6 lib/libboost_python.a -o engine.so -lSDL_image lib/libboost_thread.a -llo
obj/engine.o:
	$(CC) -c hello.cpp `sdl-config --cflags` `$(CSCF)/cs-config --cflags` -I/usr/include/python2.6 -o obj/engine.o
obj/video.o: videoplayer.cpp videoplayer.h
	$(CC) -c videoplayer.cpp `sdl-config --cflags` `$(CSCF)/cs-config --cflags` -I/usr/include/python2.6 -o obj/video.o
obj/evensys.o: evensys.cpp evensys.h
	$(CC) -c evensys.cpp `sdl-config --cflags` `$(CSCF)/cs-config --cflags` -I/usr/include/python2.6 -o obj/evensys.o
obj/trackpar.o: trackpar.cpp trackpar.h
	$(CC) -c trackpar.cpp -I/usr/include/python2.6 `$(CSCF)/cs-config --cflags` -o obj/trackpar.o
clean:
	@echo Cleaning up...
	@rm lesson05
	@echo Done.

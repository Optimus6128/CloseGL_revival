CXXFLAGS_sys = `sdl-config --cflags | sed 's/include\/SDL/include/'`
LDFLAGS_sys = `sdl-config --libs` -lpthread

obj = src/Controls.o src/GridCalcs.o src/Parts.o src/Precalcs.o src/Script.o \
	  src/SdlOpenGL.o src/Sound.o src/TextureLoad.o src/VertexSend.o
bin = closegl

incdir = -Ilibs/mikmod/include -I/usr/local/include
libdir = -L/usr/local/lib
mikmod = libs/mikmod/libmikmod.a

CXXFLAGS = -O3 -g3 $(incdir) $(CXXFLAGS_sys)
LDFLAGS = $(libdir) $(mikmod) $(LDFLAGS_sys) -lGL -lGLU -lSDL -lm

$(bin): $(mikmod) $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

.SUFFIXES: .cpp

.cpp.o:
	$(CXX) -o $@ $(CXXFLAGS) -c $<

$(mikmod):
	cd libs/mikmod && $(MAKE)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleanlibs
cleanlibs:
	cd libs/mikmod && $(MAKE) clean

obj = Controls.o GridCalcs.o Parts.o Precalcs.o Script.o SdlOpenGL.o Sound.o \
	  TextureLoad.o VertexSend.o
bin = closegl

incdir = -I/usr/local/include -I/usr/nekoware/include -I/usr/tgcware/include
libdir = -L/usr/local/lib -L/usr/nekoware/lib -L/usr/tgcware/lib

CXXFLAGS = -O3 -g3 $(incdir)
LDFLAGS = $(libdir) -lGL -lGLU -lSDL -lm

$(bin): $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

.SUFFIXES: .cpp

.cpp.o:
	$(CXX) -o $@ $(CXXFLAGS) -c $<

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

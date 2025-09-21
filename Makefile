obj = Controls.o GridCalcs.o Parts.o Precalcs.o Script.o SdlOpenGL.o Sound.o \
	  TextureLoad.o VertexSend.o
bin = closegl

CXXFLAGS = -g
LDFLAGS = -lGL -lGLU -lSDL -lm

$(bin): $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

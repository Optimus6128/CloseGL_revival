obj = depackers/mmcmp.o depackers/pp20.o depackers/s404.o depackers/xpk.o \
	  drivers/drv_nos.o drivers/drv_sdl.o loaders/load_xm.o mmio/mmalloc.o \
	  mmio/mmerror.o mmio/mmio.o playercode/mdreg.o playercode/mdriver.o \
	  playercode/mdulaw.o playercode/mloader.o playercode/mlreg.o \
	  playercode/mlutil.o playercode/mplayer.o playercode/munitrk.o \
	  playercode/mwav.o playercode/npertab.o playercode/sloader.o \
	  playercode/virtch2.o playercode/virtch.o playercode/virtch_common.o \
	  posix/strccmp.o

alib = libmikmod.a

def = -DHAVE_CONFIG_H -DMIKMOD_BUILD -DNO_SDL_CONFIG
inc = -I. -Iinclude
opt = -O3
CFLAGS = $(opt) $(def) $(inc)

$(alib): $(obj)
	$(AR) rcs $@ $(obj)

.c.o:
	$(CC) -o $@ $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -f $(obj) $(alib)

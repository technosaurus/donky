AM_CFLAGS = -DLIBDIR=\"$(libdir)/donky\" \
            -DSYSCONFDIR=\"$(sysconfdir)/donky\" \
            $(LIBDL) -lpthread -lm -Wall
SUBDIRS = modules

bin_PROGRAMS = donky
donky_LDFLAGS = -export-dynamic
donky_SOURCES = \
	main.c main.h \
	cfg.c cfg.h \
	util.c util.h \
	module.c module.h \
	mem.c mem.h \
	lists.c lists.h \
	daemon.c daemon.h \
	protocol.c protocol.h \
	request.c request.h \
	c99.c c99.h \
	std/string.h \
	std/string/strlcpy.c std/string/strlcpy.h \
	std/string/strlcat.c std/string/strlcat.h
INCLUDES = $(DEPS_CFLAGS)
LIBS = $(DEPS_LIBS)

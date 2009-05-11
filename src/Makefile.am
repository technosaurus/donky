AM_CFLAGS = -DLIBDIR=\"$(libdir)/donky\" \
            -DSYSCONFDIR=\"$(sysconfdir)/donky\" \
            -ldl -lm -Wall
SUBDIRS = modules

bin_PROGRAMS = donky
donky_LDFLAGS = -export-dynamic
donky_SOURCES = \
	main.c \
	config.c config.h \
	util.c util.h \
	module.c module.h \
	mem.c mem.h \
	lists.c lists.h \
	daemon.c daemon.h \
	protocol.c protocol.h
INCLUDES = $(DEPS_CFLAGS)
LIBS = $(DEPS_LIBS)

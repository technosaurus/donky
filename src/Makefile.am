AM_CFLAGS = -DLIBDIR=\"$(libdir)/donky\" \
            -DSYSCONFDIR=\"$(sysconfdir)/donky\" \
            -ldl -lm -Wall -std=gnu99
SUBDIRS = modules

bin_PROGRAMS = donky
donky_LDFLAGS = -export-dynamic
donky_SOURCES = \
	main.c \
	text.c text.h \
	config.c config.h \
	util.c util.h \
	module.c module.h \
	x11.c x11.h \
	render.c render.h \
	mem.c mem.h \
	lists.c lists.h
INCLUDES = $(DEPS_CFLAGS)
LIBS = $(DEPS_LIBS)

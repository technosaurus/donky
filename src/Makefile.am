AM_CFLAGS = -DLIBDIR=\"$(libdir)/donky\" \
            -DSYSCONFDIR=\"$(sysconfdir)/donky\" \
            $(LIBDL) -lpthread -Wall --std=c89 -pedantic
SUBDIRS = modules

bin_PROGRAMS = donky
donky_LDFLAGS = -export-dynamic
donky_SOURCES = \
        main.c main.h \
        cfg.c cfg.h \
        util.c util.h \
        module.c module.h \
        mem.c mem.h \
        daemon.c daemon.h \
        protocol.c protocol.h \
        request.c request.h \
        net.c net.h
INCLUDES = $(DEPS_CFLAGS)
LIBS = $(DEPS_LIBS)

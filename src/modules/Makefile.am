moduledir = $(libdir)/donky

date_la_LDFLAGS    = -module -avoid-version
sysinfo_la_LDFLAGS = -module -avoid-version

module_LTLIBRARIES = \
	date.la \
	sysinfo.la

date_la_SOURCES    = date.c develop.h
sysinfo_la_SOURCES = sysinfo.c develop.h

date_la_LIBADD    = $(DEPS_LIBS)
sysinfo_la_LIBADD = $(DEPS_LIBS)

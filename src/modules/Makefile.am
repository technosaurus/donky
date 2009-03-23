moduledir = $(libdir)/donky

date_la_LDFLAGS    = -module -avoid-version
sysinfo_la_LDFLAGS = -module -avoid-version
cpuinfo_la_LDFLAGS = -module -avoid-version
moc_la_LDFLAGS     = -module -avoid-version
battery_la_LDFLAGS = -module -avoid-version

module_LTLIBRARIES = \
	date.la \
	sysinfo.la \
	cpuinfo.la \
	moc.la \
	battery.la

date_la_SOURCES    = date.c
sysinfo_la_SOURCES = sysinfo.c
cpuinfo_la_SOURCES = cpuinfo.c
moc_la_SOURCES     = moc.c
battery_la_SOURCES = battery.c

date_la_LIBADD    = $(DEPS_LIBS)
sysinfo_la_LIBADD = $(DEPS_LIBS)
cpuinfo_la_LIBADD = $(DEPS_LIBS)
moc_la_LIBADD     = $(DEPS_LIBS)
battery_la_LIBADD = $(DEPS_LIBS)

moduledir = $(libdir)/donky

date_la_LDFLAGS     = -module -avoid-version
sysinfo_la_LDFLAGS  = -module -avoid-version
pcpuinfo_la_LDFLAGS = -module -avoid-version
scpuinfo_la_LDFLAGS = -module -avoid-version
moc_la_LDFLAGS      = -module -avoid-version
battery_la_LDFLAGS  = -module -avoid-version
exec_la_LDFLAGS     = -module -avoid-version
mpd_la_LDFLAGS      = -module -avoid-version
volume_la_LDFLAGS   = -module -avoid-version
eeepc_la_LDFLAGS    = -module -avoid-version
diskio_la_LDFLAGS   = -module -avoid-version
wifi_la_LDFLAGS     = -module -avoid-version

module_LTLIBRARIES = \
	date.la \
	sysinfo.la \
	pcpuinfo.la \
	scpuinfo.la \
	moc.la \
	battery.la \
	exec.la \
	mpd.la \
	volume.la \
	eeepc.la \
	diskio.la \
	wifi.la

date_la_SOURCES     = date.c
sysinfo_la_SOURCES  = sysinfo.c
pcpuinfo_la_SOURCES = pcpuinfo.c
scpuinfo_la_SOURCES = scpuinfo.c
moc_la_SOURCES      = moc.c
battery_la_SOURCES  = battery.c
exec_la_SOURCES     = exec.c
mpd_la_SOURCES      = mpd.c
volume_la_SOURCES   = volume.c
eeepc_la_SOURCES    = eeepc.c
diskio_la_SOURCES   = diskio.c
wifi_la_SOURCES     = wifi.c

date_la_LIBADD     = $(DEPS_LIBS)
sysinfo_la_LIBADD  = $(DEPS_LIBS)
pcpuinfo_la_LIBADD = $(DEPS_LIBS)
scpuinfo_la_LIBADD = $(DEPS_LIBS)
moc_la_LIBADD      = $(DEPS_LIBS)
battery_la_LIBADD  = $(DEPS_LIBS)
exec_la_LIBADD     = $(DEPS_LIBS)
mpd_la_LIBADD      = $(DEPS_LIBS)
volume_la_LIBADD   = $(DEPS_LIBS) -lasound
eeepc_la_LIBADD    = $(DEPS_LIBS)
diskio_la_LIBADD   = $(DEPS_LIBS)
wifi_la_LIBADD     = $(DEPS_LIBS) -liw

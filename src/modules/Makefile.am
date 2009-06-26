moduledir = $(libdir)/donky

AM_CFLAGS = -Wall -pedantic
module_LTLIBRARIES = 

if ENABLE_DATE
date_la_LDFLAGS     = -module -avoid-version --std=c89
date_la_SOURCES     = date.c
date_la_LIBADD      = $(DEPS_LIBS)
module_LTLIBRARIES += date.la
endif

if ENABLE_SYSINFO
sysinfo_la_LDFLAGS  = -module -avoid-version --std=c89
sysinfo_la_SOURCES  = sysinfo.c
sysinfo_la_LIBADD   = $(DEPS_LIBS)
module_LTLIBRARIES += sysinfo.la
endif

if ENABLE_PCPUINFO
pcpuinfo_la_LDFLAGS = -module -avoid-version --std=c89
pcpuinfo_la_SOURCES = pcpuinfo.c
pcpuinfo_la_LIBADD  = $(DEPS_LIBS)
module_LTLIBRARIES += pcpuinfo.la
endif

if ENABLE_SCPUINFO
scpuinfo_la_LDFLAGS = -module -avoid-version --std=c89
scpuinfo_la_SOURCES = scpuinfo.c
scpuinfo_la_LIBADD  = $(DEPS_LIBS)
module_LTLIBRARIES += scpuinfo.la
endif

if ENABLE_MOC
moc_la_LDFLAGS      = -module -avoid-version --std=gnu89
moc_la_SOURCES      = moc.c
moc_la_LIBADD       = $(DEPS_LIBS)
module_LTLIBRARIES += moc.la
endif

if ENABLE_BATTERY
battery_la_LDFLAGS  = -module -avoid-version --std=c89
battery_la_SOURCES  = battery.c
battery_la_LIBADD   = $(DEPS_LIBS)
module_LTLIBRARIES += battery.la
endif

if ENABLE_EXEC
exec_la_LDFLAGS     = -module -avoid-version --std=gnu89
exec_la_SOURCES     = exec.c
exec_la_LIBADD      = $(DEPS_LIBS)
module_LTLIBRARIES += exec.la
endif

if ENABLE_MPD
mpd_la_LDFLAGS      = -module -avoid-version --std=c89
mpd_la_SOURCES      = mpd.c
if ENABLE_MPDSCROB
mpd_la_SOURCES     += mpdscrob.c extra/md5.c
endif
mpd_la_LIBADD       = $(DEPS_LIBS)
module_LTLIBRARIES += mpd.la
endif

if ENABLE_VOLUME
volume_la_LDFLAGS   = -module -avoid-version --std=c89
volume_la_SOURCES   = volume.c
volume_la_LIBADD    = $(DEPS_LIBS) -lasound
module_LTLIBRARIES += volume.la
endif

if ENABLE_EEEBL
eeebl_la_LDFLAGS    = -module -avoid-version --std=c89
eeebl_la_SOURCES    = eeebl.c
eeebl_la_LIBADD     = $(DEPS_LIBS)
module_LTLIBRARIES += eeebl.la
endif

if ENABLE_WIFI
wifi_la_LDFLAGS     = -module -avoid-version --std=gnu89
wifi_la_SOURCES     = wifi.c
wifi_la_LIBADD      = $(DEPS_LIBS) -liw
module_LTLIBRARIES += wifi.la
endif

if ENABLE_WEATHER
weather_la_LDFLAGS  = -module -avoid-version --std=gnu89
weather_la_SOURCES  = weather.c
weather_la_LIBADD   = $(DEPS_LIBS)
module_LTLIBRARIES += weather.la
endif

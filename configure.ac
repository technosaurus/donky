AC_PREREQ([2.63])
AC_INIT([donky], [2009.5], [lobo@ircage.org])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

dnl Check for programs
AM_INIT_AUTOMAKE
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

dnl Set macro directory
AC_CONFIG_MACRO_DIR([m4])

dnl Checks for libraries.
AC_CHECK_LIB([m], [pow])
AC_CHECK_LIB([pthread], [pthread_create])

dnl Took this from pidgin, I guess *bsd systems don't need -ldl.
AC_CHECK_FUNC(dlopen, LIBDL="", [AC_CHECK_LIB(dl, dlopen, LIBDL="-ldl")])
AC_SUBST(LIBDL)

dnl Checks for header files.
AC_CHECK_HEADERS([limits.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/param.h sys/socket.h sys/time.h unistd.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([floor gettimeofday memset pow socket strcasecmp strchr strndup strrchr strstr strtol sysinfo snprintf])

dnl Get unix name
uname=`uname`

case $uname in
        Linux*)
                AC_DEFINE([OS_LINUX], [1], [linux])
                ;;
        FreeBSD*)
                AC_DEFINE([OS_FREEBSD], [1], [freebsd])
                ;;
        OpenBSD*)
                AC_DEFINE([OS_OPENBSD], [1], [openbsd])
                ;;
        *)
                echo "Sorry, we do not currently support $uname!"
                exit 1
                ;;
esac

dnl Wifi support...
AC_MSG_CHECKING([whether to build the wifi module])
AC_ARG_ENABLE(wifi,
              AC_HELP_STRING([--enable-wifi],
                             [enable the wifi module (linux only, requires wireless-tools) [default=no]]), ,
              [enable_wifi=no])
if test "x$enable_wifi" = "xyes"; then
        if test "x$uname" != "xLinux"; then
                AC_MSG_RESULT([no])
                AC_MSG_NOTICE([wifi module not supported on $uname, disabling])
                enable_wifi="no"
        else
                AC_MSG_RESULT([yes])
                AC_CHECK_LIB([iw], [iw_sockets_open])
                MODULES="$MODULES wifi"
        fi
else
        AC_MSG_RESULT([no])
fi

dnl Battery support...
AC_MSG_CHECKING([whether to build the battery module])
AC_ARG_ENABLE(battery,
              AC_HELP_STRING([--enable-battery],
                             [enable the battery module (linux only) [default=no]]), ,
              [enable_battery=no])
if test "x$enable_battery" = "xyes"; then
        if test "x$uname" != "xLinux"; then
                AC_MSG_RESULT([no])
                AC_MSG_NOTICE([battery module not supported on $uname, disabling])
        else
                AC_MSG_RESULT([yes])
                MODULES="$MODULES battery"
        fi
else
        AC_MSG_RESULT([no])
fi

dnl Date support...
AC_MSG_CHECKING([whether to build the date module])
AC_ARG_ENABLE(date,
              AC_HELP_STRING([--enable-date],
                             [enable the date module [default=yes]]), ,
              [enable_date=yes])
if test "x$enable_date" = "xyes"; then
        AC_MSG_RESULT([yes])
        MODULES="$MODULES date"
else
        AC_MSG_RESULT([no])
fi

dnl Diskio support...
AC_MSG_CHECKING([whether to build the diskio module])
AC_ARG_ENABLE(diskio,
              AC_HELP_STRING([--enable-diskio],
                             [enable the diskio module [default=no]]), ,
              [enable_diskio=no])
if test "x$enable_diskio" = "xyes"; then
        AC_MSG_RESULT([yes])
        MODULES="$MODULES diskio"
else
        AC_MSG_RESULT([no])
fi

dnl Eee PC backlight support...
AC_MSG_CHECKING([whether to build the eeebl module])
AC_ARG_ENABLE(eeebl,
              AC_HELP_STRING([--enable-eeebl],
                             [enable the eeebl module [default=no]]), ,
              [enable_eeebl=no])
if test "x$enable_eeebl" = "xyes"; then
        AC_MSG_RESULT([yes])
        MODULES="$MODULES eeebl"
else
        AC_MSG_RESULT([no])
fi

dnl Exec support...
AC_MSG_CHECKING([whether to build the exec module])
AC_ARG_ENABLE(exec,
              AC_HELP_STRING([--enable-exec],
                             [enable the exec module [default=yes]]), ,
              [enable_exec=yes])
if test "x$enable_exec" = "xyes"; then
        AC_MSG_RESULT([yes])
        MODULES="$MODULES exec"
else
        AC_MSG_RESULT([no])
fi

dnl MOC supprt...
AC_MSG_CHECKING([whether to build the moc module])
AC_ARG_ENABLE(moc,
              AC_HELP_STRING([--enable-moc],
                             [enable the moc module [default=no]]), ,
              [enable_moc=no])
if test "x$enable_moc" = "xyes"; then
        AC_MSG_RESULT([yes])
        MODULES="$MODULES moc"
else
        AC_MSG_RESULT([no])
fi

dnl MPD support...
AC_MSG_CHECKING([whether to build the mpd module])
AC_ARG_ENABLE(mpd,
              AC_HELP_STRING([--enable-mpd],
                             [enable the mpd module [default=no]]), ,
              [enable_mpd=no])
if test "x$enable_mpd" = "xyes"; then
        AC_MSG_RESULT([yes])
        MODULES="$MODULES mpd"
else
        AC_MSG_RESULT([no])
fi

dnl pcpuinfo support...
AC_MSG_CHECKING([whether to build the pcpuinfo module])
AC_ARG_ENABLE(pcpuinfo,
              AC_HELP_STRING([--enable-pcpuinfo],
                             [enable the /proc cpuinfo module [default=no]]), ,
              [enable_pcpuinfo=no])
if test "x$enable_pcpuinfo" = "xyes"; then
        AC_MSG_RESULT([yes])
        MODULES="$MODULES pcpuinfo"
else
        AC_MSG_RESULT([no])
fi

dnl scpuinfo support...
AC_MSG_CHECKING([whether to build the scpuinfo module])
AC_ARG_ENABLE(scpuinfo,
              AC_HELP_STRING([--enable-scpuinfo],
                             [enable the /sys cpuinfo module [default=no]]), ,
              [enable_scpuinfo=no])
if test "x$enable_scpuinfo" = "xyes"; then
        AC_MSG_RESULT([yes])
        MODULES="$MODULES scpuinfo"
else
        AC_MSG_RESULT([no])
fi

dnl sysinfo support...
AC_MSG_CHECKING([whether to build the sysinfo module])
AC_ARG_ENABLE(sysinfo,
              AC_HELP_STRING([--enable-sysinfo],
                             [enable the sysinfo module [default=yes]]), ,
              [enable_sysinfo=yes])
if test "x$enable_sysinfo" = "xyes"; then
        AC_MSG_RESULT([yes])
        MODULES="$MODULES sysinfo"
else
        AC_MSG_RESULT([no])
fi

dnl alsa volume support...
AC_MSG_CHECKING([whether to build the volume module])
AC_ARG_ENABLE(volume,
              AC_HELP_STRING([--enable-volume],
                             [enable the alsa volume module [default=no]]), ,
              [enable_volume=no])
if test "x$enable_volume" = "xyes"; then
        AC_MSG_RESULT([yes])
        AC_CHECK_LIB([asound], [snd_mixer_open])
        MODULES="$MODULES volume"
else
        AC_MSG_RESULT([no])
fi

dnl Am conditionals
AM_CONDITIONAL(ENABLE_WIFI, test "x$enable_wifi" = "xyes")
AM_CONDITIONAL(ENABLE_BATTERY, test "x$enable_battery" = "xyes")
AM_CONDITIONAL(ENABLE_DATE, test "x$enable_date" = "xyes")
AM_CONDITIONAL(ENABLE_DISKIO, test "x$enable_diskio" = "xyes")
AM_CONDITIONAL(ENABLE_EEEBL, test "x$enable_eeebl" = "xyes")
AM_CONDITIONAL(ENABLE_EXEC, test "x$enable_exec" = "xyes")
AM_CONDITIONAL(ENABLE_MOC, test "x$enable_moc" = "xyes")
AM_CONDITIONAL(ENABLE_MPD, test "x$enable_mpd" = "xyes")
AM_CONDITIONAL(ENABLE_PCPUINFO, test "x$enable_pcpuinfo" = "xyes")
AM_CONDITIONAL(ENABLE_SCPUINFO, test "x$enable_scpuinfo" = "xyes")
AM_CONDITIONAL(ENABLE_SYSINFO, test "x$enable_sysinfo" = "xyes")
AM_CONDITIONAL(ENABLE_VOLUME, test "x$enable_volume" = "xyes")

dnl Our Makefiles.
AC_CONFIG_FILES([Makefile
                 data/Makefile
                 src/Makefile
                 src/modules/Makefile])
AC_OUTPUT

dnl Print results
AC_MSG_RESULT([>>])
AC_MSG_RESULT([>> $PACKAGE $VERSION configured successfully.])
AC_MSG_RESULT([>>])
AC_MSG_RESULT([>> PREFIX: $prefix])
AC_MSG_RESULT([>> MODULES: $MODULES])
AC_MSG_RESULT([>> CFLAGS: $CFLAGS])
AC_MSG_RESULT([>> LIBS: $LIBS])
AC_MSG_RESULT([>>])

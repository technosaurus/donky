#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([donky], [2009.5], [lobo@ircage.org])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# libtool
LT_INIT
LT_CONFIG_LTDL_DIR([libltdl])
AC_CONFIG_AUX_DIR([libltdl/config])
AC_CONFIG_MACRO_DIR([libltdl/m4])
AC_LIB_LTDL

# Checks for programs.
AM_INIT_AUTOMAKE
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_LIBTOOL

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lasound':
#AC_CHECK_LIB([asound], [main])
# FIXME: Replace `main' with a function in `-liw':
#AC_CHECK_LIB([iw], [main])

AC_CHECK_LIB([m], [pow])
AC_CHECK_LIB([pthread], [pthread_create])

# Checks for header files.
AC_CHECK_HEADERS([limits.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/param.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([floor gettimeofday memset pow socket strcasecmp strchr strndup strrchr strstr strtol sysinfo])

AC_CONFIG_FILES([Makefile
                 data/Makefile
                 src/Makefile
                 src/modules/Makefile])
AC_OUTPUT


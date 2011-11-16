#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([procmon], [0.8.2], [anders.lovgren@bmc.uu.se])
AM_INIT_AUTOMAKE([procmon], [0.8.2])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S

# Checks for libraries.
AC_CHECK_LIB([proc],[readproc])

# Checks for header files.
AC_CHECK_HEADERS([proc/readproc.h stdlib.h string.h syslog.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([memset select strchr strerror strstr])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT

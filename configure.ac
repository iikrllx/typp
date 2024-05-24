#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(typp, v1.0.0, krekhov.dev@mail.ru)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_INIT_AUTOMAKE([subdir-objects])

# Checks for pkg (list of modules).
PKG_CHECK_MODULES([ncursesw], [ncursesw])
PKG_CHECK_MODULES([formw], [formw])
PKG_CHECK_MODULES([menuw], [menuw])

# Checks for libraries.
AC_CHECK_LIB([ncurses], [initscr])
AC_CHECK_LIB([m], [round])

# Checks for header files.
AC_CHECK_HEADERS([unistd.h math.h time.h ctype.h wchar.h errno.h error.h \
                  signal.h menu.h form.h ncurses.h locale.h stdio.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([setlocale])

AC_CONFIG_FILES([Makefile src/Makefile texts/Makefile man/Makefile])
AC_OUTPUT

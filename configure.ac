AC_PREREQ([2.64])
AC_INIT([telecom_analyzer], [f2a1c26], [martin@martinjlowm.dk], [telecom])
AC_CONFIG_SRCDIR([src/telecom_analyzer.cc])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_HEADERS([config.h])

m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

CXXFLAGS="-g -O0"

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_RANLIB

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE

# Checks for library functions.
AC_FUNC_STRTOD
AC_CHECK_FUNCS([floor getpagesize memset sqrt strtoul strtol])

# Checks for libraries.
PKG_CHECK_MODULES(FFTW3, fftw3 >= 3.0)
AC_SUBST(FFTW3_LIBS)
AC_SUBST(FFTW3_CFLAGS)

# Checks for hackrf
PKG_CHECK_MODULES(LIBHACKRF, libhackrf)
AC_SUBST(LIBHACKRF_LIBS)
AC_SUBST(LIBHACKRF_CFLAGS)

# Checks for gnuradio
PKG_CHECK_MODULES(GNURADIO, gnuradio-runtime)
AC_SUBST(GNURADIO_LIBS)
AC_SUBST(GNURADIO_CFLAGS)

# OSX doesn't support System V shared memory
AC_CANONICAL_HOST
case "$host_os" in
	darwin*)
		AC_DEFINE([D_HOST_OSX], [1], [building for OSX])
		;;
esac

case "$host_cpu" in
	arm*)
		AC_DEFINE([D_HOST_OSX], [1], [building without shared memory])
		;;
esac

AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 src/Makefile])
AC_OUTPUT

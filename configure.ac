#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([hfm], [1.0], [phuong.do@igloo.com])
AC_CONFIG_SRCDIR([hfm.c])
AM_INIT_AUTOMAKE([subdir-objects])
#AC_CONFIG_HEADERS([hfm.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

## Checks for libraries.
## FIXME: Replace `main' with a function in `-lcrypto':
#AC_CHECK_LIB([crypto], [main])
## FIXME: Replace `main' with a function in `-lm':
#AC_CHECK_LIB([m], [main])
## FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [pthread_create])
## FIXME: Replace `main' with a function in `-lssl':
#AC_CHECK_LIB([ssl], [main])
## FIXME: Replace `main' with a function in `-lvmi':
AC_CHECK_LIB([vmi], [vmi_init])
## FIXME: Replace `main' with a function in `-lxenctrl':
#AC_CHECK_LIB([xenctrl], [main])
## FIXME: Replace `main' with a function in `-lxenlight':
#AC_CHECK_LIB([xenlight], [main])
## FIXME: Replace `main' with a function in `-lxentoollog':
#AC_CHECK_LIB([xentoollog], [main])

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([getcwd gettimeofday strdup strrchr strstr])

AC_CONFIG_FILES([Makefile
                 fcleaner/Makefile
                 fcleaner/tests/Makefile
                 monitors/Makefile
                 outputs/Makefile
                 tests/Makefile])
AC_OUTPUT

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
AC_SEARCH_LIBS([pthread_create], [pthread])
AC_SEARCH_LIBS([g_slist_alloc], [glib-2.0]) 
AC_SEARCH_LIBS([xc_interface_open], [xenctrl]) 
AC_SEARCH_LIBS([xtl_log], [xentoollog]) 
AC_SEARCH_LIBS([libxl_name_to_domid], [xenlight]) 
AC_SEARCH_LIBS([xc_domain_memory_mapping], [xenlight]) 
AC_SEARCH_LIBS([json_object_from_file], [json-c]) 
AC_SEARCH_LIBS([sqrt], [m]) 

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

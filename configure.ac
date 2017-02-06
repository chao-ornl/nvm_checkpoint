# -*- shell-script -*-
#
# Copyright 2017        UT-Battelle, LLC.
#                       All rights reserved.
# See COPYING in top-level directory.
# 
# Additional copyrights may follow
# 
# $HEADER$
#

AC_INIT([ornvcr], [0.1], [valleegr@ornl.gov])
AC_CONFIG_SRCDIR([include/ORNVCR.h])

AM_INIT_AUTOMAKE([dist-bzip2 subdir-objects foreign tar-ustar])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_MACRO_DIR([m4])

CFLAGS="$CFLAGS -std=c99"

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AM_PROG_CC_C_O
m4_pattern_allow([AM_PROG_AR], [AM_PROG_AR])
LT_INIT

# NVMALLOC_CHECK_WITHDIR(with_option_name, dir_value, file_in_dir)
# ----------------------------------------------------
AC_DEFUN([NVMALLOC_CHECK_WITHDIR],[
    AC_MSG_CHECKING([--with-$1 value])
    AS_IF([test "$2" = "yes" -o "$2" = "no" -o "x$2" = "x"],
          [AC_MSG_RESULT([simple ok (unspecified)])],
          [AS_IF([test ! -d "$2"],
                 [AC_MSG_RESULT([not found])
                  AC_MSG_WARN([Directory $2 not found])
                  AC_MSG_ERROR([Cannot continue])],
                 [AS_IF([test "x`ls $2/$3 2> /dev/null`" = "x"],
                        [AC_MSG_RESULT([not found])
                         AC_MSG_WARN([Expected file $2/$3 not found])
                         AC_MSG_ERROR([Cannot continue])],
                        [AC_MSG_RESULT([sanity check ok ($2)])]
                       )
                 ]
                )
          ]
         )
])dnl

AC_ARG_WITH([nvmalloc], [AS_HELP_STRING([--with-nvmalloc=DIR],
            [Specify the location of the NVmalloc installation.])])
CCI_CHECK_WITHDIR([nvmalloc],[$with_nvmalloc], [include/nvm.h])
AC_ARG_WITH([nvmalloc-libdir], [AC_HELP_STRING([--with-nvmalloc-libdir=DIR],
             [Search for NVmalloc libraries in DIR])])
CCI_CHECK_WITHDIR([nvmalloc-libdir],[$with_nvmalloc_libdir], [libnvm*])

AS_IF([test ! -z "$with_nvmalloc" -a "$with_nvmalloc" != "yes"],
      [nvmalloc_dir="$with_nvmalloc"])
AS_IF([test ! -z "$with_nvmalloc_libdir" -a "$with_nvmalloc_libdir" != "yes"],
      [nvmalloc_libdir="$with_nvmalloc_libdir"])

CPPFLAGS_save="$CPPFLAGS"
AS_IF([test ! -z "$nvmalloc_dir"],
      AC_SUBST([CPPFLAGS],["-I$nvmalloc_dir/include $CPPFLAGS_save"]))
LDFLAGS_save="$LDFLAGS"
AS_IF([test ! -z "$nvmalloc_libdir"],
      AC_SUBST([LDFLAGS],(["-L$nvmalloc_libdir -Wl,-rpath,$nvmalloc_libdir $LDFLAGS_save"])))
AS_IF([test ! -z "$nvmalloc_dir" -a -z "$nvmalloc_libdir"],
      AC_SUBST([LDFLAGS],(["-L$nvmalloc_dir/lib -Wl,-rpath,$nvmalloc_dir/lib $LDFLAGS_save"])))

AC_CHECK_LIB([nvmalloc], [nvm_init], , AC_MSG_ERROR([Unable to locate NVmalloc installation]))

CPPFLAGS_save="$CPPFLAGS"
top_srcdir=`pwd`
AC_SUBST([CPPFLAGS],["-I$top_srcdir/include $CPPFLAGS_save -std=c99"])

AC_CONFIG_FILES([Makefile include/Makefile src/Makefile])

AC_OUTPUT

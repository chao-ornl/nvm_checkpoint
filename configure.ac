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

# NVM_CHECK_WITHDIR(with_option_name, dir_value, file_in_dir)
# ----------------------------------------------------
AC_DEFUN([NVM_CHECK_WITHDIR],[
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

AC_ARG_WITH([nvm], [AS_HELP_STRING([--with-nvm=DIR],
            [Specify the location of the NVmalloc installation.])])
NVM_CHECK_WITHDIR([nvm],[$with_nvm], [include/nvm.h])
AS_IF([test ! -z "$with_nvm" -a "$with_nvm" != "yes"],
      [nvm_dir="$with_nvm"])

AC_ARG_WITH([nvm-libdir], [AC_HELP_STRING([--with-nvm-libdir=DIR],
             [Search for NVmalloc libraries in DIR])])
NVM_CHECK_WITHDIR([nvm-libdir],[$with_nvm_libdir], [libnvm*])
AS_IF([test ! -z "$with_nvm_libdir" -a "$with_nvm_libdir" != "yes"],
      [nvm_libdir="$with_nvm_libdir"])

AC_ARG_WITH([nvm-includedir], [AC_HELP_STRING([--with-nvm-includedir=DIR],
             [Search for NVmalloc headers in DIR])])
NVM_CHECK_WITHDIR([nvm-includedir],[$with_nvm_includedir], [nvm.h])
AS_IF([test ! -z "$with_nvm_includedir" -a "$with_nvm_includedir" != "yes"],
      [nvm_includedir="$with_nvm_includedir"])

CPPFLAGS_save="$CPPFLAGS"
AS_IF([test ! -z "$nvm_dir"],
      AC_SUBST([CPPFLAGS],["-I$nvm_dir/include $CPPFLAGS_save"]))

LDFLAGS_save="$LDFLAGS"
AS_IF([test ! -z "$nvm_libdir"],
      AC_SUBST([LDFLAGS],(["-L$nvm_libdir -Wl,-rpath,$nvm_libdir $LDFLAGS_save"])))

LDFLAGS_save="$LDFLAGS"
AS_IF([test ! -z "$nvm_includedir"],
      AC_SUBST([LDFLAGS],(["-I$nvm_includedir $LDFLAGS_save"])))

DFLAGS_save="$LDFLAGS"
AS_IF([test ! -z "$nvm_dir" -a -z "$nvm_libdir"],
      AC_SUBST([LDFLAGS],(["-L$nvm_dir/lib -Wl,-rpath,$nvm_dir/lib $LDFLAGS_save"])))

AC_CHECK_LIB([nvm], [nvm_init], , AC_MSG_ERROR([Unable to locate NVmalloc installation]))

CPPFLAGS_save="$CPPFLAGS"
top_srcdir=`pwd`
AC_SUBST([CPPFLAGS],["-I$top_srcdir/include $CPPFLAGS_save -std=c99"])

AC_CONFIG_FILES([Makefile include/Makefile src/Makefile])

AC_OUTPUT

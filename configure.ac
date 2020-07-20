#
# Copyright (c) 2012-2015 LAAS/CNRS
#
# GenoM3/bip template autoconf file
#
# heavily inspired by the genom3 pocolibs template, etc.
#

AC_PREREQ(2.62)
AC_INIT([genom3-bip],[1.0.0],[mfoughal@laas.fr])

AC_CONFIG_AUX_DIR([autoconf])
AC_CONFIG_MACRO_DIR([autoconf])
AM_INIT_AUTOMAKE([foreign no-define subdir-objects])
AC_PROG_INSTALL

# compilers
LT_INIT([disable-static])
AC_PROG_CC
AX_PTHREAD(, [AC_MSG_ERROR([pthread support not available])])

# external software
PKG_CHECK_MODULES(pocolibs, [pocolibs >= 2.11.99])
# PKG_CHECK_MODULES(genom3_c_client, [genom3-c-client >= 20140618])
PKG_CHECK_MODULES(genom3, [genom3 >= 2.99.26])
_PKG_CONFIG(genom3prefix, [variable=prefix], [genom3 >= 2.99.26])
_PKG_CONFIG(genom3tmpldir, [variable=tmpldir], [genom3 >= 2.99.26])
_PKG_CONFIG(genom3sitedir, [variable=sitetmpldir], [genom3 >= 2.99.26])
_PKG_CONFIG(genom3, [variable=GENOM_TOOL], [genom3 >= 2.99.26])
AC_SUBST(genom3tmpldir, [$pkg_cv_genom3tmpldir])
AC_SUBST(genom3sitedir, [${pkg_cv_genom3sitedir##$pkg_cv_genom3prefix/}])
AC_SUBST(genom3, [${pkg_cv_genom3}])

if test "x$prefix" = xNONE; then
  AC_MSG_CHECKING([default prefix...])
  prefix=$pkg_cv_genom3prefix
  AC_MSG_RESULT([$prefix])
fi

# output
AC_CONFIG_FILES([
	genom3-bip-pocolibs.pc
	Makefile
])
AC_OUTPUT

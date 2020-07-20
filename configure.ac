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
AC_PROG_CXX

AX_PTHREAD(, [AC_MSG_ERROR([pthread support not available])])

# boost
AX_BOOST_BASE([1.40],, [AC_MSG_ERROR([boost not available])])

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
dnl python
AC_ARG_VAR(PYTHON, [python interpreter])
AC_PATH_PROGS(PYTHON, [python python2.7 python2.6])
if ! test -x "$PYTHON"; then AC_MSG_FAILURE([python not found]); fi

dnl ROS
PKG_CHECK_MODULES(ROS, [roscpp >= 1.8.15])
AC_MSG_CHECKING([rosaction dependencies])
if $PKG_CONFIG --exists --print-errors '
    genpy >= 0.3.7
    roslib >= 1.8.10
    topic_tools >= 1.8.15
    rosgraph_msgs >= 1.8.15
    actionlib >= 1.8.7'; then
  AC_MSG_RESULT([ok])
else
  AC_MSG_RESULT([no])
  AC_MSG_FAILURE([missing ros dependencies])
fi

# - work out the rpath for -Ldirs found in ros .pc
# - clean up -l: syntax
ros_libs=
for arg in $ROS_LIBS; do
  case "$arg" in
    -l:/*/lib*.so)
      name=${arg##*/lib}; name=${name%.so}
      path=${arg#-l:}; path=${path%/lib*.so}
      arg="-L$path -R$path -l$name";;
    -l:*) AC_MSG_FAILURE([unsupported flag $arg in ROS .pc files]);;

    -L*) arg="$arg -R${arg#-L}";;
  esac
  for a in $arg; do
    case " $ros_libs " in
      *" $a "*) ;;
      *) ros_libs="$ros_libs $a";;
    esac
  done
done
ROS_LIBS=${ros_libs# }

# output
AC_CONFIG_FILES([
	genom3-bip-pocolibs.pc
	genom3-bip-ros.pc
	common-ros/msgmd5
	Makefile
])
AC_OUTPUT

#
# Copyright (c) 2012-2015 LAAS/CNRS
# All rights reserved.
#
# Permission to use, copy, modify, and distribute this software for any purpose
# with or without   fee is hereby granted, provided   that the above  copyright
# notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS  SOFTWARE INCLUDING ALL  IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR  BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR  ANY DAMAGES WHATSOEVER RESULTING  FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION,   ARISING OUT OF OR IN    CONNECTION WITH THE USE   OR
# PERFORMANCE OF THIS SOFTWARE.
#
#                                            Anthony Mallet on Tue Feb 27 2012
#
ACLOCAL_AMFLAGS=-I autoconf

bipdir=${prefix}/${genom3sitedir}/bip

# --- common pocolibs ---------------------------------------------------------------

commonpldir=${bipdir}
nobase_dist_commonpl_DATA=\
	common-pocolibs/autoconf/ax_pthread.m4	\
					\
	common-pocolibs/typeutil.tcl		\
					\
	common-pocolibs/msglib.h			\
	common-pocolibs/serialize.h		\
	common-pocolibs/typecopy.h

# --- BIP model -------------------------------------------------------------

bipmodeldir=${bipdir}
nobase_dist_bipmodel_DATA=\
	model/template.tcl		\
	model/model_execution.tcl			\
	model/model_verification.tcl			\
	model/model_verification_urgencies.tcl


# --- pkgconfig ---------------------------------------------------------------

pkgconfigdir=	$(libdir)/pkgconfig
pkgconfig_DATA=	genom3-bip-pocolibs.pc genom3-bip-ros.pc

# --- server PocoLibs ---------------------------------------------------------------

serverpocolibsdir=${bipdir}
nobase_dist_serverpocolibs_DATA=		\
	server-pocolibs/template.tcl		\
	server-pocolibs/metadata.gen		\
						\
	server-pocolibs/pocolibs.configure.ac	\
	server-pocolibs/pocolibs.Makefile.am	\
	server-pocolibs/pocolibs.pc.in		\
						\
	server-pocolibs/activity.h		\
	server-pocolibs/activity.c		\
	server-pocolibs/codels.mappings.h	\
	server-pocolibs/control_task.h		\
	server-pocolibs/control_task.c		\
	server-pocolibs/control_codels.c	\
	server-pocolibs/exception.c		\
	server-pocolibs/exec_task.c		\
	server-pocolibs/exec_codels.c		\
	server-pocolibs/main.c			\
	server-pocolibs/main.h			\
	server-pocolibs/msglib.c		\
	server-pocolibs/parameters.h		\
	server-pocolibs/portlib.h		\
	server-pocolibs/portlib.c		\
	server-pocolibs/remotelib.h		\
	server-pocolibs/remotelib.c		\
	server-pocolibs/genom3_external_for_bip.h\
	server-pocolibs/genom3_global_external_for_bip.h\
	server-pocolibs/main.cpp      \
	server-pocolibs/BIPGenoM3.cpp   	\
	server-pocolibs/BIPGenoM3.hpp      	\
	server-pocolibs/BIPGenoM3.bip		\
	server-pocolibs/global_functions.c			\
	server-pocolibs/global_functions.cpp			\
	server-pocolibs/global_functions.hpp			\
	server-pocolibs/genom3-external.h


# --- common ros ---------------------------------------------------------------

commonrdir=${bipdir}
nobase_dist_commonr_DATA=\
	common-ros/genrosmsg.tcl		\
	common-ros/ros.manifest.xml		\
					\
	common-ros/typecopy.h		\
	common-ros/genom-serialize.h	\
	common-ros/serialize.h		\
	common-ros/portlib.h		\
	common-ros/service.h		\
	common-ros/action.h

nobase_dist_commonr_SCRIPTS=\
	common-ros/msgmd5

commonautoconfdir=${bipdir}
nobase_dist_commonautoconf_DATA=\
	common-ros/autoconf/ax_pthread.m4		\
	common-ros/autoconf/ax_boost_base.m4	\
	common-ros/autoconf/ax_boost_thread.m4


#bin_SCRIPTS=	rosaction

# --- server ROS ---------------------------------------------------------------

serverrosdir=${bipdir}
nobase_dist_serverros_DATA=		\
	server-ros/template.tcl		\
					\
	server-ros/ros.configure.ac		\
	server-ros/ros.Makefile.am		\
					\
	server-ros/BIPEactivities.h		\
	server-ros/activities.h		\
	server-ros/codels.mappings.h	\
	server-ros/BIPEinternals.h		\
	server-ros/internals.h		\
	server-ros/locals.h			\
	server-ros/remote.h			\
	server-ros/port.h			\
					\
	server-ros/activities.cc		\
	server-ros/BIPEcontrol-task.cc		\
	server-ros/control-task.cc		\
	server-ros/control-codels.cc	\
	server-ros/exception.cc		\
	server-ros/exec-task.cc		\
	server-ros/main.cc			\
	server-ros/BIPEmain.cc			\
	server-ros/remote.cc		\
	server-ros/port.cc			\
	server-ros/json.cc			\
					\
	server-ros/BIPEgenom3_external_for_bip.h\
	server-ros/genom3_external_for_bip.h\
	server-ros/BIPmain.cpp      	\
	server-ros/BIPGenoM3.cpp      	\
	server-ros/BIPGenoM3.hpp      	\
	server-ros/BIPGenoM3.bip

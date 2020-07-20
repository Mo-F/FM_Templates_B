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

# --- common ---------------------------------------------------------------

commondir=${bipdir}
nobase_dist_common_DATA=\
	common/autoconf/ax_pthread.m4	\
					\
	common/typeutil.tcl		\
					\
	common/msglib.h			\
	common/serialize.h		\
	common/typecopy.h

# --- BIP model -------------------------------------------------------------

bipmodeldir=${bipdir}
nobase_dist_bipmodel_DATA=\
	model/template.tcl		\
	model/model_execution.tcl			\
	model/model_verification.tcl


# --- pkgconfig ---------------------------------------------------------------

pkgconfigdir=	$(libdir)/pkgconfig
pkgconfig_DATA=	genom3-bip-pocolibs.pc


# --- server ---------------------------------------------------------------

serverdir=${bipdir}
nobase_dist_server_DATA=\
	server/template.tcl		\
	server/metadata.gen		\
					\
	server/pocolibs.configure.ac	\
	server/pocolibs.Makefile.am	\
	server/pocolibs.pc.in		\
					\
	server/activity.h		\
	server/activity.c		\
	server/codels.mappings.h	\
	server/control_task.h		\
	server/control_task.c		\
	server/control_codels.c		\
	server/exception.c		\
	server/exec_task.c		\
	server/exec_codels.c		\
	server/main.c			\
	server/msglib.c			\
	server/parameters.h		\
	server/portlib.h		\
	server/portlib.c		\
	server/remotelib.h		\
	server/remotelib.c		\
	server/genom3_external_for_bip.h\
	server/BIPGenoM3.cpp      \
	server/BIPGenoM3.hpp      \
	server/BIPGenoM3.bip

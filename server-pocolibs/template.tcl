#
# Copyright (c) 2010-2015 LAAS/CNRS
# All rights reserved.
#
# Redistribution  and  use  in  source  and binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of  source  code must retain the  above copyright
#      notice and this list of conditions.
#   2. Redistributions in binary form must reproduce the above copyright
#      notice and  this list of  conditions in the  documentation and/or
#      other materials provided with the distribution.
#
# THE SOFTWARE  IS PROVIDED "AS IS"  AND THE AUTHOR  DISCLAIMS ALL WARRANTIES
# WITH  REGARD   TO  THIS  SOFTWARE  INCLUDING  ALL   IMPLIED  WARRANTIES  OF
# MERCHANTABILITY AND  FITNESS.  IN NO EVENT  SHALL THE AUTHOR  BE LIABLE FOR
# ANY  SPECIAL, DIRECT,  INDIRECT, OR  CONSEQUENTIAL DAMAGES  OR  ANY DAMAGES
# WHATSOEVER  RESULTING FROM  LOSS OF  USE, DATA  OR PROFITS,  WHETHER  IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR  OTHER TORTIOUS ACTION, ARISING OUT OF OR
# IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#                                           Anthony Mallet on Mon Feb  1 2010
#

# BIP Pocolibs component generation.

template usage {*}{
    "\n"
    "BIP Engine Pocolibs-based component generation.\n"
    "\n"
    "This template generates a GenoM component that links with the BIP engine and uses the pocolibs\n"
    "middleware.\n"
    "\n"
    "Supported options:\n"
    "  -C, --directory=dir\toutput files in dir\n"
    "  -p, --preserve\tdo not overwrite existing files\n"
    "  -s. --slow\t\tspecify the slow down factor\n"
    "  -h. --help\t\tprint usage summary (this text)"
}

# defaults

set slowdown 1
set odir bip-pocolibs
engine mode +overwrite +move-if-change

# require utility procs
template require ../common-pocolibs/typeutil.tcl

# parse options
template options {
    -C - --directory	{ set odir [template arg] }
    -p - --preserve	{ engine mode -overwrite }
    -s - --slow         { set slowdown [template arg] }
    -h - --help		{ puts [template usage]; exit 0 }
}
if {![llength $argv]} { puts [template usage]; exit 2 }

engine chdir $odir

# add common services to dotgen specification
dotgen parse file [file join [dotgen template builtindir] common/genom.gen]
dotgen parse file [file join [dotgen template dir] metadata.gen]

# parse input
set input [list]
foreach f $argv {
  dotgen parse file $f
  lappend input [file normalize $f]
}

# common header for all files (copyright info from .gen file)
set header {/* <"[--- Generated by [dotgen genom version]. Do not edit - ]"> */

<'if {![catch {dotgen input notice} notice]} {
  puts [lang c; comment $notice]
}'>
}

# server source files
set server_files {
  ../common-pocolibs/msglib.h
  exception.c
  parameters.h
  main.c
  msglib.c
  portlib.h
  portlib.c
  remotelib.h
  remotelib.c
  activity.h
  activity.c
  control_task.h
  control_task.c
  control_codels.c
  genom3_external_for_bip.h
}
set server_task_files {
  exec_task.c
  exec_codels.c
}
set bip_files {
  BIPGenoM3.bip
}
set bip_external_files {
  main.cpp
  BIPGenoM3.cpp
  BIPGenoM3.hpp
}

foreach c [dotgen components] {
  foreach f $server_files {
   template parse args $c string $header \
        file $f file src/[$c name]_[file tail $f]
  }
  foreach f $bip_files {
      template parse args $slowdown string $header \
        file $f file src/bip/[$c name]_[file tail $f]
  }
  foreach f $bip_external_files {
   template parse args $c string $header \
        file $f file src/bip/external/[$c name]_[file tail $f]
  }
  foreach t [$c tasks] {
    foreach f $server_task_files {
      template parse args [list $c $t] string $header \
          file $f file src/[$c name]_[$t name]_[file tail $f]
    }
  }
}

# C mappings for the server
foreach c [dotgen components] {
  template parse args [list $c c] string $header \
      file codels.mappings.h file src/[$c name]_c_types.h
}

# serialization and type manipulation procedures for all private types
set types [dict create]
foreach c [dotgen components] {
  foreach t [$c types private] {
    dict set types [$t mangle] $t
  }
}
set types [dict values $types]

template parse args [list $types] \
    string $header file ../common-pocolibs/serialize.h file src/serialize.h
template parse args [list $types] \
    string $header file ../common-pocolibs/typecopy.h file src/typecopy.h

# setup build environment
template parse args [list $input] file pocolibs.Makefile.am file Makefile.am
template parse file pocolibs.configure.ac file configure.ac
template parse file ../common-pocolibs/autoconf/ax_pthread.m4 file autoconf/ax_pthread.m4
foreach c [dotgen components] {
  template parse					\
      args [list $c] file pocolibs.pc.in		\
	file [$c name]-bip-pocolibs.pc.in
}

set deps [list]
foreach d [concat [dotgen input deps] [template deps]] {
  lappend deps "depend/input.d: $d"
  lappend deps "$d:"
}
engine mode +overwrite -move-if-change
template parse raw [join $deps "\n"]\n file depend/input.d

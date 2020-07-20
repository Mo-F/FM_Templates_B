<'
# Copyright (c) 2012-2013 LAAS/CNRS
# All rights reserved.
#
# Redistribution and use  in source  and binary  forms,  with or without
# modification, are permitted provided that the following conditions are
# met:
#
#   1. Redistributions of  source  code must retain the  above copyright
#      notice and this list of conditions.
#   2. Redistributions in binary form must reproduce the above copyright
#      notice and  this list of  conditions in the  documentation and/or
#      other materials provided with the distribution.
#
#                                      Anthony Mallet on Wed Aug  1 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]

lang c
'>
#include "autoconf/acheader.h"

#include <errno.h>
#include <stdio.h>

#include "posterLib.h"

#include "<"$comp">_control_task.h"
#include "serialize.h"


/* --- local data ---------------------------------------------------------- */



/* <"[--- genom_metadata_${comp}_fetch -------------------------------------]"> */


<'
# Copyright (c) 2011-2015 LAAS/CNRS
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
#                                      Anthony Mallet on Mon Mar  5 2012
#

if {[llength $argv] != 2} { error "expected arguments: component task" }
lassign $argv component task

# compute handy shortcuts
set comp [$component name]
set tname [$task name]

lang c
'>
#include "autoconf/acheader.h"

#include <sys/time.h>

#include <stdio.h>

#include "commonStructLib.h"
#include "h2evnLib.h"
#include "taskLib.h"

#include "<"$comp">_control_task.h"



/* <"[--- genom_${comp}_${tname}_exec_task ------------------------------]"> */

/* will drastically reduce (even remove) this function */

void *
genom_<"$comp">_<"$tname">_exec_task_init(void *data)
{
  struct genom_component_data *self = data;
  /* init */
  genom_<"$comp">_log_info("Inited task <"[$task name]">");

}


/* BIP function to return the exec task permanent activity */
genom_activity_ptr BIP_<"$comp">_<"$tname">_permanent_activity(){
  return <"$comp">_genom_component->tasks.<"$tname">.permanent;
}

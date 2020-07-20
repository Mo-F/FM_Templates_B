<'
# Copyright (c) 2012-2016 LAAS/CNRS
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
#                                      Anthony Mallet on Sat Jan 14 2012
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

#include "<"$comp">_internals.h"
#include "<"$comp">_genom3_external_for_bip.h"


/* === codels ============================================================== */

<'foreach obj [list $task {*}[$task services]] {'>

/* <"[--- [$obj class] [$obj name] --------------------------------------]"> */
<'  foreach state [$obj fsm] {'>
<'    set codel [$obj fsm $state]'>
<'    set func [$obj class]_[$obj name]_[$state name]'>

/* state <"[$state name]"> */
static inline genom_event
<"$comp">_codel_<"$func">(
  genom_<"$comp">_component_data *self, genom_<"$comp">_activity_<"[$obj class]_[$obj name]"> *a)
{
  genom_event s;
  
<'
    # build parameter list
    set plist [list]
    foreach p [$codel parameters] {
      switch -glob -- [$p src] {
        local	{
          switch -- [[$p param] dir] {
            local	{ set m "a->locals.[$p name]" }
            in		{ set m "a->in.[$p name]" }
            inout - out { set m "a->out.[$p name]" }
            default	{ error "unsupported local direction" }
          }
        }
        remote		{ set m "self->remotes.[[$p remote] name]_.handle" }
        port		{ set m "self->ports.[[$p port] name]_.handle" }
        ids		{ set m "self->ids[$p member]" }
        default		{ error "reached unreachable code" }
      }

      switch  -glob -- "[$p dir]/[$p src]" {
	in/* - */port {
          lappend plist [[$p type] pass value $m]
        }
	default {
	  lappend plist [[$p type] pass reference $m]
	}
      }
    }
    lappend plist "&self->tasks.[$task name]_.context"
'>
  s = <"[$codel invoke $plist]">;

  return s;
}


genom_event
<'  if {[$obj class] eq "task"} {'>
BIP_<"$comp">_codel_<"$func">(genom_<"$comp">_activity_ptr a)
<'  } else {'>
  BIP_<"$comp">_codel_<"$func">(genom_activity_<"$comp">_<"[$obj name]">_ptr a)
<'  }'>
{
  genom_<"$comp">_component_data *self=<"$comp">_genom_component; 
  genom_event s;
  genom_<"$comp">_log_info("Calling <"$comp">_codel_<"$func">  codel.");

  s = <"$comp">_codel_<"$func">(self, (genom_<"$comp">_activity_<"[$obj class]">_<"[$obj name]"> *) a);

  genom_<"$comp">_log_info("Returning %s.", s);
  ((genom_<"$comp">_activity_<"[$obj class]">_<"[$obj name]"> *)a)->state = s;

  return s;
}


<'  }'>

<'}'>

genom_<"$comp">_activity * BIP_<"$comp">_<"$tname">_permanent_activity()
{
  return <"$comp">_genom_component->tasks.<"$tname">_.permanent;
}

/*eof*/

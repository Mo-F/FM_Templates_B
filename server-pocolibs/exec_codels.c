<'
# Copyright (c) 2012-2015 LAAS/CNRS
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

if {[llength $argv] != 2} { error "expected arguments: component task" }
lassign $argv component task

# compute handy shortcuts
set comp [$component name]

lang c
'>
#include "autoconf/acheader.h"

#include "commonStructLib.h"

#include "<"$comp">_control_task.h"


/* <"[--- [$task class] [$task name] --------------------------------------]"> */
<'  foreach state [$task fsm] {'>
<'    set codel [$task fsm $state]'>
<'    set func [$task class]_[$task name]_[$state name]'>

/* state <"[$state name]"> */
genom_event
<"$comp">_codel_<"$func">(
  struct genom_component_data *self, struct genom_activity *a)
{
  genom_event s;

<'
    # build parameter list
    set plist [list]
    foreach p [$codel parameters] {
      switch -glob -- [$p src] {
        local {
          if {[$task class] eq "task"} {
            error "task codel with service parameter"
          }
          switch -- [[$p param] dir] {
            local	{ set m "a->locals.[$p name]" }
            in		{ set m "a->in.[$p name]" }
            inout - out { set m "a->out.[$p name]" }
            default	{ error "unsupported local direction" }
          }
        }
        remote	{ set m "self->remotes.[[$p remote] name].data" }
        port	{ set m "self->ports.[[$p port] name].handle" }
        ids	{ set m "self->ids[$p member]" }
        default	{ error "unsupported codel parameter source" }
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
    lappend plist "&self->tasks.[$task name].context"
'>
  s = <"[$codel invoke $plist]">;

  return s;
}

genom_event
BIP_<"$comp">_codel_<"$func">(struct genom_activity *a)
{
  genom_event s;
  struct genom_component_data *self = <"$comp">_genom_component;
  genom_<"$comp">_log_info("Calling <"$comp">_codel_<"$func">  codel.");

  s = <"$comp">_codel_<"$func">(self, a);

  a->state = s;

  return s;
}

<'}'>


<'foreach obj [$task services] {'>

/* <"[--- [$obj class] [$obj name] --------------------------------------]"> */
<'  foreach state [$obj fsm] {'>
<'    set codel [$obj fsm $state]'>
<'    set func [$obj class]_[$obj name]_[$state name]'>

/* state <"[$state name]"> */
genom_event
<"$comp">_codel_<"$func">(
  struct genom_component_data *self, struct genom_<"$comp">_<"[$obj name]">_activity *a)
{
  genom_event s;

<'
    # build parameter list
    set plist [list]
    foreach p [$codel parameters] {
      switch -glob -- [$p src] {
        local {
          if {[$obj class] eq "task"} {
            error "task codel with service parameter"
          }
          switch -- [[$p param] dir] {
            local	{ set m "a->locals.[$p name]" }
            in		{ set m "a->in.[$p name]" }
            inout - out { set m "a->out.[$p name]" }
            default	{ error "unsupported local direction" }
          }
        }
        remote	{ set m "self->remotes.[[$p remote] name].data" }
        port	{ set m "self->ports.[[$p port] name].handle" }
        ids	{ set m "self->ids[$p member]" }
        default	{ error "unsupported codel parameter source" }
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
    lappend plist "&self->tasks.[$task name].context"
'>
  s = <"[$codel invoke $plist]">;

  return s;
}

genom_event
BIP_<"$comp">_codel_<"$func">(struct genom_<"$comp">_<"[$obj name]">_activity *a)
{
  genom_event s;
  struct genom_component_data *self = <"$comp">_genom_component;

  genom_<"$comp">_log_info("Calling <"$comp">_codel_<"$func"> codel.");
  s = <"$comp">_codel_<"$func">(self, a);

  a->h.state = s;
  genom_<"$comp">_log_info("Exiting <"$comp">_codel_<"$func"> codel with %s.", s);
  return s;
}

<'  }'>
<'}'>


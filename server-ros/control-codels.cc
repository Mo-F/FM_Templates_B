<'
# Copyright (c) 2011-2016 LAAS/CNRS
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
#                                      Anthony Mallet on Mon Dec 19 2011
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]
if {[catch { $component version } version]} { set version {}}

lang c
'>
#include "autoconf/acheader.h"

#include "<"$comp">_internals.h"
#include "<"$comp">_typecopy.h"


/* --- genom_abort_activity_codel ------------------------------------------ */

genom_event
genom_abort_activity_codel(uint32_t activity, genom_context ctx)
{
<'foreach t [$component tasks] {'>
  /* task <"[$t name]"> */
  {
    genom_component_data *self = ctx->data->self;
    genom_activity *a;

    pthread_mutex_lock(&self->tasks.<"[$t name]">_.lock);
    for(size_t id = 0; id < genom_activities::MAX_ACTIVITIES; id++) {
      a = self->tasks.<"[$t name]">_.activities.a[id];
      if (!a) continue;

      switch(a->status) {
        case ACT_VOID: continue;

        case ACT_INIT: case ACT_RUN:
          if ((uint32_t/*oops*/)a->aid != activity) continue;
          a->stop = 1;
          a->interruptedby = "abort_activity";
          pthread_mutex_unlock(&self->tasks.<"[$t name]">_.lock);
          return genom_ok;

        case ACT_STOP: case ACT_ETHER:
          if ((uint32_t/*oops*/)a->aid != activity) continue;
          pthread_mutex_unlock(&self->tasks.<"[$t name]">_.lock);
          return genom_ok;
      }
    }
    pthread_mutex_unlock(&self->tasks.<"[$t name]">_.lock);
  }

<'}'>
  return genom_no_such_activity(ctx);
}


/* --- genom_connect_port_codel -------------------------------------------- */

/** Implement connect_port service codel */
genom_event
genom_connect_port_codel(const char local[128], const char remote[128],
                         genom_context ctx)
{
  genom_component_data *self = ctx->data->self;
  genom_event s;

  genom_take_resource(
    self,
<'foreach t [$component tasks] {'>
    self->resources.task_<"[$t name]"> ||
<'}'>
    self->resources.all,

    self->resources.all = 1);

<'foreach p [$component ports in simple] {'>
  if (!strcmp(local, "<"[$p name]">")) {
    if (self->ports.<"[$p name]">_.subscribe(self->control.node, remote))
      s = genom_no_such_outport(ctx);
    else
      s = genom_ok;
    goto done;
  }
<'}'>
<'foreach p [$component ports in multiple] {'>
  if (!strncmp(local, "<"[$p name]">/", sizeof("<"[$p name]">/")-1)) {
    if (self->ports.<"[$p name]">_.subscribe(
          self->control.node, local + sizeof("<"[$p name]">/")-1, remote))
      s = genom_no_such_outport(ctx);
    else
      s = genom_ok;
    goto done;
  }
<'}'>

  s = genom_no_such_inport(ctx);
  goto done;

done:
  genom_give_resource(self, self->resources.all = 0);
  return s;
}


/* --- genom_connect_remote_codel ------------------------------------------ */

genom_event
genom_connect_remote_codel(const char local[128], const char remote[128],
                           genom_context ctx)
{
  genom_component_data *self = ctx->data->self;
  genom_event s;

  genom_take_resource(
    self,
<'foreach t [$component tasks] {'>
    self->resources.task_<"[$t name]"> ||
<'}'>
    self->resources.all,

    self->resources.all = 1);

  /* find the remote */
<'foreach r [$component remotes] {'>
  if (!strcmp(local, "<"[$r name]">")) {
    s = self->data->self->remotes.<"[$r name]">_.connect(remote, ctx);
    goto done;
  }
<'}'>

  s = genom_no_such_remote(ctx);
  goto done;

done:
  genom_give_resource(self, self->resources.all = 0);
  return s;
}


/* --- genom_kill_codel ---------------------------------------------------- */

genom_event
genom_kill_codel(genom_context self)
{
  genom_shutdown = 1;
  return genom_ok;
}

<'foreach s [$component services] {'>


/* <"[--- Service [$s name] control codels ------------------------------]"> */

genom_event
genom_component_data::<"[$s name]">_controlcb(
  genom::srv_<"[$s name]">::locals &locals,
  genom::srv_<"[$s name]">::input &in,
  genom::srv_<"[$s name]">::output &out)
{
  genom_event s = genom_ok;

  /* check allowance (before/after statements) */
<'  foreach other [$s after] {'>
  if (!control.run_map[<"$COMP">_<"[$other name]">_RQSTID])
    return genom_disallowed(&control.context);
<'}'>
<'  foreach other [$s before] {'>
  if (control.run_map[<"$COMP">_<"[$other name]">_RQSTID])
    return genom_disallowed(&control.context);
<'}'>

  /* copy inout parameters to output */
<'  foreach p [$s parameters inout] {'>
  genom::ids::pcopy(out.<"[$p name]">, in.<"[$p name]">);
<'  }'>

  /* call validate codel */
<'  if {[llength [$s validate]]} {'>
<'    set validate [$s validate]'>
  genom_take_resource(
    this,
<'    foreach m [$validate mutex] {'>
<'      if {[$m class] ne "codel"} continue'>
<'      if {[catch {$m task} t]} continue'>
    resources.task_<"[$t name]"> == <"[$m cname]"> ||
<'      }'>
    resources.all,

    resources.control = (void *)<"[$validate cname]">);
<'    # build parameter list
    set plist [list]
    foreach p [[$s validate] parameters] {
      switch -- [$p src] {
        ids	{ set m "ids[$p member]" }
        local	{
          switch -- [[$p param] dir] {
            local	{ set m "locals.[$p name]" }
            in		{ set m "in.[$p name]" }
            inout - out { set m "out.[$p name]" }
            default	{ error "unsupported local direction" }
          }
        }
        default	{ error "unsupported codel parameter" }
      }
      switch -- [$p dir] {
	in		{ lappend plist [[$p type] pass value $m] }
	inout - out	{ lappend plist [[$p type] pass reference $m] }
        default		{ error "unsupported direction" }
      }
    }
    lappend plist &control.context
'>
  s = <"[$validate invoke $plist]">;
  genom_give_resource(this, resources.control = NULL);

  genom_log_debug("service <"[$s name]"> validation returned %s", s?s:"ok");
  if (s) return s;
<'  }'>

  /* copy ids parameters to ids (attributes) */
<'  foreach p [$s parameters] {'>
<'    if {[$p src] == "ids"} {'>
  genom_take_resource(
    this,
<'      foreach m [$s mutex] {'>
<'        if {[$m class] ne "codel"} continue'>
<'        if {[catch {$m task} t]} continue'>
    resources.task_<"[$t name]"> == <"[$m cname]"> ||
<'      }'>
    resources.all,

    resources.control = (void *)<"[$s name]_resource">);
<'      foreach p [$s parameters in inout] {'>
<'        if {[$p src] == "ids"} {'>
  genom::ids::pcopy(ids<"[$p member]">, in.<"[$p name]">);
<'        }'>
<'      }'>
<'      foreach p [$s parameters out inout] {'>
<'        if {[$p src] == "ids"} {'>
  genom::ids::pcopy(out.<"[$p name]">, ids<"[$p member]">);
<'        }'>
<'      }'>
  genom_give_resource(this, resources.control = NULL);
<'      break'>
<'    }'>
<'  }'>

  /* call simple codels */
<'foreach c [$s codels simple] {'>
  genom_take_resource(
    this,
<'  foreach m [$c mutex] {'>
<'    if {[$m class] ne "codel"} continue'>
<'    if {[catch {$m task} t]} continue'>
    resources.task_<"[$t name]"> == <"[$m cname]"> ||
<'  }'>
    resources.all,

    resources.control = (void *)<"[$c cname]">);
<'  # build parameter list
    set plist [list]
    foreach p [$c parameters] {
      switch -- [$p src] {
        ids	{ set m "ids[$p member]" }
        local	{
          switch -- [[$p param] dir] {
            local	{ set m "locals.[$p name]" }
            in		{ set m "in.[$p name][$p member]" }
            inout - out { set m "out.[$p name][$p member]" }
            default	{ error "unsupported local direction" }
          }
        }
        default	{ error "unsupported codel parameter" }
      }
      switch -- [$p dir] {
	in		{ lappend plist [[$p type] pass value $m] }
	inout - out	{ lappend plist [[$p type] pass reference $m] }
        default		{ error "unsupported direction" }
      }
    }
    lappend plist &control.context
'>
  s = <"[$c invoke $plist]">;
  genom_give_resource(this, resources.control = NULL);

  genom_log_debug("service %s codel %s returned %s",
                  "<"[$s name]">", "<"[$c name]">", s?s:"ok");
  if (s
<'  foreach t [$s throw] {'>
    && s != <"[$t cname]">_id
<'  }'>
    ) {
    genom_unkex_detail d;
    strncpy(d.what, s, sizeof(d.what)); d.what[sizeof(d.what)-1] = *"";
    genom_log_warn(
      "unknown exception %s for codel %s in service %s",
      s, "<"[$c name]">", "<"[$s name]">");
    s = genom_unkex(&d, &control.context);
  }
  if (s) return s;
<'  }'>

  return s;
}
<'}'>

/*eof*/

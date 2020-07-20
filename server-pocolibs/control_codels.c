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
#                                      Anthony Mallet on Fri Mar  2 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]

lang c
'>
#include "autoconf/acheader.h"

#include <signal.h>
#include <unistd.h>

#include "<"$comp">_c_types.h"
#include "typecopy.h"
#include "<"$comp">_parameters.h"
#include "<"$comp">_control_task.h"


/* --- genom_abort_activity_codel ------------------------------------------ */

genom_event
genom_abort_activity_codel(uint32_t activity, genom_context ctx)
{
<'foreach t [$component tasks] {'>
  /* task <"[$t name]"> */
  {
    struct genom_component_data *self = ctx->data->self;
    struct genom_activity *a;
    int id;

    for(id = 0; id < genom_max_activities(); id++) {
      a = &self->tasks.<"[$t name]">.activities.a[id].any;
      switch(a->status) {
        case ACT_VOID: continue;

        case ACT_ACT:
          if (a->aid != activity) continue;
          a->stop = 1;
          a->interruptedby = "abort_activity";
          return genom_ok;
      }
    }
  }

<'}'>
  return genom_no_such_activity(ctx);
}


/* --- genom_connect_port_codel -------------------------------------------- */

/* it will be much easier to connect all ports before starting the engine...*/

genom_event
genom_connect_port_codel(const char local[128], const char remote[128],
                         genom_context ctx)
{
  struct genom_component_data *self = ctx->data->self;
  genom_event s;

  /* find the inport */
<'foreach p [$component ports in] {'>
<'  if {"multiple" in [$p kind]} {'>
  if (!strncmp(local, "<"[$p name]">/", sizeof("<"[$p name]">/")-1)) {
    s = genom_<"$comp">_<"[$p name]">_connect(
      local + sizeof("<"[$p name]">/")-1, remote, ctx);
    goto done;
  }
<'  } else {'>
  if (!strcmp(local, "<"[$p name]">")) {
    s = genom_<"$comp">_<"[$p name]">_connect(remote, ctx);
    goto done;
  }
<'  }'>
<'}'>

  s = genom_no_such_inport(ctx);
  goto done;

done:
  return s;
}


/* --- genom_connect_remote_codel ------------------------------------------ */

genom_event
genom_connect_remote_codel(const char local[128], const char remote[128],
                           genom_context ctx)
{
  struct genom_component_data *self = ctx->data->self;
  genom_event s;


  /* find the remote */
<'foreach r [$component remotes] {'>
  if (!strcmp(local, "<"[$r name]">")) {
    s = genom_<"$comp">_<"[$r name]">_connect(remote, ctx);
    goto done;
  }
<'}'>

  s = genom_no_such_remote(ctx);
  goto done;

done:
  return s;
}


/* --- genom_kill_codel ---------------------------------------------------- */

genom_event			/* This has probably no meaning in the BIPE context. */
				/* indeed */
genom_kill_codel(genom_context self)
{
  kill(getpid(), SIGTERM);
  return genom_ok;
}

<'foreach s [$component services] {'>

/* <"[--- Service [$s name] validate codel ------------------------------]"> */

genom_event
genom_<"$comp">_<"[$s name]">_validatecb(
  struct genom_component_data *self,
  struct genom_<"$comp">_<"[$s name]">_activity *a)
{
  genom_event s = genom_ok;

  /* check allowance (before/after statements) */
  /* I believe this should be done by the BIP Model.. */
<'  foreach other [$s after] {'>
  if (!self->run_map[<"$COMP">_<"[$other name]">_RQSTID])
    return genom_disallowed(self);
<'  }'>
<'  foreach other [$s before] {'>
  if (self->run_map[<"$COMP">_<"[$other name]">_RQSTID])
    return genom_disallowed(self);
<'  }'>

  /* copy inout parameters to output */
<'  foreach p [$s parameters inout] {'>
  genom_tcopy_<"[[$p type] mangle]">(
    <"[[$p type] pass reference a->out.[$p name]]">,
    <"[[$p type] pass value a->in.[$p name]]">);
<'  }'>

  /* call validate codel */
<'  if {[llength [$s validate]]} {'>
<'    set validate [$s validate]'>
<'
    # build parameter list
    set plist [list]
    foreach p [[$s validate] parameters] {
      switch -- [$p src] {
        ids	{ set m "self->ids[$p member]" }
        local	{
          switch -- [[$p param] dir] {
            local	{ set m "a->locals.[$p name]" }
            in		{ set m "a->in.[$p name]" }
            inout - out { set m "a->out.[$p name]" }
            default	{ error "unsupported local direction" }
          }
        }
        default	{ error "unsupported codel parameter source" }
      }
      switch -- [$p dir] {
	in		{ lappend plist [[$p type] pass value $m] }
	inout - out	{ lappend plist [[$p type] pass reference $m] }
        default		{ error "unsupported direction" }
      }
    }
    lappend plist &self->control.context
'>
  s = <"[$validate invoke $plist]">;

  genom_log_debug("service <"[$s name]"> validation returned %s", s?s:"ok");
<'  }'>
  return s; 
}

/* <"[--- Service [$s name] control codels ------------------------------]"> */

genom_event
genom_<"$comp">_<"[$s name]">_controlcb(
  struct genom_component_data *self,
  struct genom_<"$comp">_<"[$s name]">_activity *a)
{
  genom_event s = genom_ok;

  /* copy ids parameters to ids (attributes) */
<'  foreach p [$s parameters] {'>
<'    if {[$p src] == "ids"} {'>
<'      foreach p [$s parameters in inout] {'>
<'        if {[$p src] == "ids"} {'>
  genom_tcopy_<"[[$p type] mangle]">(
    <"[[$p type] pass reference self->ids[$p member]]">,
    <"[[$p type] pass value a->in.[$p name]]">);
<'        }'>
<'      }'>
<'      foreach p [$s parameters out inout] {'>
<'        if {[$p src] == "ids"} {'>
  genom_tcopy_<"[[$p type] mangle]">(
    <"[[$p type] pass reference a->out.[$p name]]">,
    <"[[$p type] pass value self->ids[$p member]]">);
<'        }'>
<'      }'>
<'      break'>
<'    }'>
<'  }'>

  /* call simple codels */
<'  foreach c [$s codels simple] {'>
<'  # build parameter list
    set plist [list]
    foreach p [$c parameters] {
      switch -- [$p src] {
        ids	{ set m "self->ids[$p member]" }
        local	{
          switch -- [[$p param] dir] {
            local	{ set m "a->locals.[$p name]" }
            in		{ set m "a->in.[$p name]" }
            inout - out { set m "a->out.[$p name]" }
            default	{ error "unsupported local direction" }
          }
        }
        default		{ error "unsupported codel parameter" }
      }
      switch -- [$p dir] {
	in		{ lappend plist [[$p type] pass value $m] }
	inout - out	{ lappend plist [[$p type] pass reference $m] }
        default		{ error "unsupported direction" }
      }
    }
    lappend plist &self->control.context
'>
  s = <"[$c invoke $plist]">;

  genom_log_debug("service %s codel %s returned %s",
                  "<"[$s name]">", "<"[$c name]">", s?s:"ok");
  if (s
<'    foreach t [$s throw] {'>
      && s != <"[$t cname]">_id
<'    }'>
      ) {
    genom_unkex_detail d;
    strncpy(d.what, s, sizeof(d.what)); d.what[sizeof(d.what)-1] = *"";
    genom_log_warn(
      0, "unknown exception %s for codel %s in service %s",
      s, "<"[$c name]">", "<"[$s name]">");
    s = genom_unkex(&d, &self->control.context);
  }
  if (s) return s;
<'  }'>
  return s;
}

<'}'>
/* eof */

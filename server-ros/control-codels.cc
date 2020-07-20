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
#include "<"$comp">_genom3_external_for_bip.h"


/* --- genom_<"$comp">_abort_activity_codel ------------------------------------------ */

genom_event
genom_<"$comp">_abort_activity_codel(uint32_t activity, genom_context ctx)
{
<'foreach t [$component tasks] {'>
  /* task <"[$t name]"> */
  {
    genom_<"$comp">_component_data *self = ctx->data->self;
    genom_<"$comp">_activity *a;

    //    pthread_mutex_lock(&self->tasks.<"[$t name]">_.lock);
    for(size_t id = 0; id < genom_<"$comp">_activities::MAX_ACTIVITIES; id++) {
      a = self->tasks.<"[$t name]">_.activities.a[id];
      if (!a) continue;

      switch(a->status) {
        case ACT_VOID: continue;

        case ACT_INIT: case ACT_RUN:
          if ((uint32_t/*oops*/)a->aid != activity) continue;
          a->stop = 1;
          a->interruptedby = "abort_activity";
	  //          pthread_mutex_unlock(&self->tasks.<"[$t name]">_.lock);
          return genom_ok;

        case ACT_STOP: case ACT_ETHER:
          if ((uint32_t/*oops*/)a->aid != activity) continue;
	  //          pthread_mutex_unlock(&self->tasks.<"[$t name]">_.lock);
          return genom_ok;
      }
    }
    //    pthread_mutex_unlock(&self->tasks.<"[$t name]">_.lock);
  }

<'}'>
  return genom_no_such_activity(ctx);
}


/* --- genom_<"$comp">_connect_port_codel -------------------------------------------- */

/** Implement connect_port service codel */
genom_event
genom_<"$comp">_connect_port_codel(const char local[128], const char remote[128],
                         genom_context ctx)
{
  genom_<"$comp">_component_data *self = ctx->data->self;
  genom_event s;


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
  return s;
}


/* --- genom_<"$comp">_connect_remote_codel ------------------------------------------ */

genom_event
genom_<"$comp">_connect_remote_codel(const char local[128], const char remote[128],
                           genom_context ctx)
{
  genom_<"$comp">_component_data *self = ctx->data->self;
  genom_event s;


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
  return s;
}


/* --- genom_<"$comp">_kill_codel ---------------------------------------------------- */

genom_event
genom_<"$comp">_kill_codel(genom_context self)
{
  BIPE_genom_shutdown = 1;	// this is a global
  return genom_ok;
}

<'foreach s [$component services] {'>


/* <"[--- Service [$s name] control codels ------------------------------]"> */

genom_event
genom_<"$comp">_component_data::<"[$s name]">_validatecb(
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

  out.genom_success = s == genom_ok;
  if (!out.genom_success)
    genom_<"$comp">_encodex(out.genom_exdetail, s,
                   control.context.raised(NULL, &control.context));
  
  genom_<"$comp">_log_info("service <"[$s name]"> validation returned %s", s?s:"ok");

  return s;
<'  }'>
}


genom_event
genom_<"$comp">_component_data::<"[$s name]">_controlcb(
  genom::srv_<"[$s name]">::locals &locals,
  genom::srv_<"[$s name]">::input &in,
  genom::srv_<"[$s name]">::output &out)
{
  genom_event s = genom_ok;

  /* copy ids parameters to ids (attributes) */
<'  foreach p [$s parameters] {'>
<'    if {[$p src] == "ids"} {'>
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
<'      break'>
<'    }'>
<'  }'>

  /* call simple codels */
<'foreach c [$s codels simple] {'>
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

<' switch -- [$s name] {
   connect_service {'>
   s = genom_<"$comp">_connect_remote_codel(in.local, in.remote, &control.context);
<' }
   abort_activity {'>
   s = genom_<"$comp">_abort_activity_codel(in.activity, &control.context);
<' }
    connect_port {'>
   s = genom_<"$comp">_connect_port_codel(in.local, in.remote, &control.context);
<' } 
   kill { '>
   s = genom_<"$comp">_kill_codel(&control.context);
<' } 
   default {'>
   s = <"[$c invoke $plist]">;	/* normal */
<' }
 }'>

   //   s = <"[$c invoke $plist]">;

  genom_<"$comp">_log_debug("service %s codel %s returned %s",
                  "<"[$s name]">", "<"[$c name]">", s?s:"ok");


  if (s
<'  foreach t [$s throw] {'>
    && s != <"[$t cname]">_id
<'  }'>
    ) {
    genom_unkex_detail d;
    strncpy(d.what, s, sizeof(d.what)); d.what[sizeof(d.what)-1] = *"";
    genom_<"$comp">_log_warn(
      "unknown exception %s for codel %s in service %s",
      s, "<"[$c name]">", "<"[$s name]">");
    s = genom_unkex(&d, &control.context);
  }

  out.genom_success = s == genom_ok;
  if (!out.genom_success)
    genom_<"$comp">_encodex(out.genom_exdetail, s,
                   control.context.raised(NULL, &control.context));

  if (out.genom_success) {
    control.run_map[<"$COMP">_<"[$s name]">_RQSTID] = true;
    <"[$s name]">_interrupt_other(-1);
  }

  genom_<"$comp">_log_debug("service <"[$s name]"> control returned %s", s?s:"ok");
  
  if (s) return s;
<'  }'>

  return s;
}

<'}'>

<'foreach s [$component services] {'>

genom_event  BIP_<"$comp">_<"[$s name]">_validate(int rqid)
{
  struct genom_<"$comp">_component_data *self = <"$comp">_genom_component;
  genom_event s;

  genom_<"$comp">_log_info("BIP calling validate codel for <"$comp"> <"[$s name]">.");

  assert (rqid == self->control.current_rqst_type);
<'  if {[catch {$s task} t]} {'>
   s = self->genom_<"$comp">_component_data::<"[$s name]">_validatecb(self->services.<"[$s name]">_locals,
							    self->services.<"[$s name]">_in,
							    self->services.<"[$s name]">_out);
<'  } else {'>
   genom_<"$comp">_activity_service_<"[$s name]"> *a = (genom_<"$comp">_activity_service_<"[$s name]"> *)self->control.current_activity;
   s = self->genom_<"$comp">_component_data::<"[$s name]">_validatecb(a->locals,
							    a->in,
							    a->out);
<'  }'>
   self->control.current_rqst_return = s;
   return s;
}

<'  if {[catch {$s task} t]} {'>
genom_event  BIP_<"$comp">_<"[$s name]">_control(int rqid)
{
  struct genom_<"$comp">_component_data *self = <"$comp">_genom_component;
  genom_event s;

  genom_<"$comp">_log_info("BIP calling control codel for <"$comp"> <"[$s name]">.");

  assert (rqid == self->control.current_rqst_type);
  s = self->genom_<"$comp">_component_data::<"[$s name]">_controlcb(self->services.<"[$s name]">_locals,
							  self->services.<"[$s name]">_in,
							  self->services.<"[$s name]">_out);
   self->control.current_rqst_return = s;

   return s;
}
<'  }'>

<'if {[$s kind] == "activity"} {'>
void BIP_<"$comp">_<"[$s name]">_activity_report(genom_activity_<"$comp">_<"[$s name]">_ptr a)
{
  genom_<"$comp">_component_data *self = <"$comp">_genom_component;
  genom_<"$comp">_log_info("BIP finished and reporting for <"$comp"> <"[$s name]"> activity.");

  self->activity_report((genom_<"$comp">_activity *)a);

  return;
}
<'}'>

void BIP_<"$comp">_<"[$s name]">_rqst_report(int rqid)
{
  genom_<"$comp">_component_data *self = <"$comp">_genom_component;

  assert (rqid == self->control.current_rqst_type);
  
  genom_<"$comp">_log_info("BIP finished and reporting for <"$comp"> <"[$s name]"> rqst.");

  // This sequence will resume the ROS thread
  pthread_mutex_lock(&self->control.bip_proxi);
  self->control.bip_proxi_idle = true;
  pthread_cond_broadcast(&self->control.serv_bip_proxi);
  pthread_mutex_unlock(&self->control.bip_proxi);

  return;
}

<'if {[$s kind] == "activity"} {'>
genom_activity_<"$comp">_<"[$s name]">_ptr BIP_cast_activity_in_<"$comp">_<"[$s name]">_activity(int rqid)
{
  // This is really to grap the activity structure to be passed in the
  // BIP model.  We get it from the control structure because we have
  // stored it here, and we know there is only one rqst being handled
  // at once.
  genom_<"$comp">_component_data *self = <"$comp">_genom_component;

  assert (rqid == self->control.current_rqst_type);
  genom_activity_<"$comp">_<"[$s name]">_ptr a = self->control.current_activity;
  
  return a;
}
<'}'>

<'}'>

void BIP_<"$comp">_send_ir(int rqid)
{
  // send-ir is called when the whole "control" phase of the activity has gone thru successfully...
  // Nothing to do really... just restarting the control task... no?

  genom_<"$comp">_component_data *self = <"$comp">_genom_component;

  assert (rqid == self->control.current_rqst_type);
  
  genom_<"$comp">_log_info("BIP IR for <"$comp"> %d rqst", rqid);

  // This sequence will resume the ROS thread
  pthread_mutex_lock(&self->control.bip_proxi);
  self->control.bip_proxi_idle = true;
  pthread_cond_broadcast(&self->control.serv_bip_proxi);
  pthread_mutex_unlock(&self->control.bip_proxi);

  return; 
}


/*eof*/

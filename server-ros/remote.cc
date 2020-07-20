<'
#
# Copyright (c) 2012-2014 LAAS/CNRS
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
#                                      Anthony Mallet on Thu Sep 20 2012
#
#


if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]
'>
#include "autoconf/acheader.h"

#include "<"$comp">_internals.h"
#include "<"$comp">_typecopy.h"

<'foreach r [$component remotes] {'>

/* <"[--- Remote [$r name] ----------------------------------------------]"> */

<'
  set arg [list]
  foreach p [$r parameters] {
    switch -- [$p dir] {
      in		{ set a [[$p type] argument value [$p name]] }
      out - inout	{ set a [[$p type] argument reference [$p name]] }
      default		{ error "invalid parameter direction" }
    }
    lappend arg $a
  }
  lappend arg {genom_context self}
  set arg [join $arg {, }]
'>
extern "C" genom_event
genom_<"$comp">_remote_<"[$r name]">_call(<"$arg">)
{
<'
  set l [list]
  foreach p [$r parameters] { lappend l [$p name] }
  lappend l self
'>
  return self->data->self->remotes.<"[$r name]">.call(<"[join $l ,]">);
}

/* connect */
genom_event
genom::<"$comp">_remote::<"[$r name]">::connect(const char remote[128],
                          genom_context self)
{
  handle.call = NULL;
<'  switch -- "[$r kind]" {'>
<'    attribute - function {'>
  c = self->data->self->node->serviceClient<
    genom::srv_<"[$r name]">::input, genom::srv_<"[$r name]">::output
    >(remote, true);
  if (!c.exists()) return genom_no_such_remote(self);
<'    }'>
<'    activity {'>
  if (c) delete c;
  c = new client(*self->data->self->node, remote, &cbq);
  if (!c) return genom_no_such_remote(self);

  int count = 3;
  while(ros::ok() && !c->isServerConnected() && count-->0)
    cbq.callAvailable(ros::WallDuration(1.));
  if (!c->isServerConnected()) return genom_no_such_remote(self);
<'    }'>
<'  }'>
  handle.call = genom_<"$comp">_remote_<"[$r name]">_call;
  return genom_ok;
}

/* call */
genom_event
genom::<"$comp">_remote::<"[$r name]">::call(<"$arg">)
{
<'  foreach p [$r parameter in] {'>
  genom::ids::pcopy(in.<"[$p name]">, <"[[$p type] deref value [$p name]]">);
<'  }'>
<'  foreach p [$r parameter inout] {'>
  genom::ids::pcopy(in.<"[$p name]">, <"[[$p type] deref reference [$p name]]">);
<'  }'>
<'  switch -- "[$r kind]" {'>
<'    attribute - function {'>
  genom::srv_<"[$r name]">::output out;

  if (!c.call(in, out)) return genom_remote_io(self);

<'      foreach p [$r parameter inout out] {'>
  genom::ids::pcopy(<"[[$p type] deref reference [$p name]]">, out.<"[$p name]">);
<'      }'>
<'    }'>
<'    activity {'>
  boost::shared_ptr<const genom::srv_<"[$r name]">::output> out;
  client::GoalHandle gh;

  gh = c->sendGoal(in);
  while(ros::ok()) {
    cbq.callAvailable(ros::WallDuration(1.));
    if (gh.getCommState() == actionlib::CommState::DONE) break;
  }

  out = gh.getResult();
  if (!out) return genom_remote_io(self);

<'      foreach p [$r parameter inout out] {'>
  genom::ids::pcopy(<"[[$p type] deref reference [$p name]]">, out-><"[$p name]">);
<'      }'>
<'    }'>
<'  }'>
  if (out.genom_success) return genom_ok;

  return genom_<"$comp">_decodex(out.genom_exdetail.c_str(), self);
}

<'}'>

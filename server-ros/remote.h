<'
#
# Copyright (c) 2012,2014 LAAS/CNRS
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

lang c


if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]
'>
#ifndef H_GROS_<"$COMP">_REMOTE
#define H_GROS_<"$COMP">_REMOTE

#include "actionlib/client/simple_action_client.h"

#include "<"${comp}">_c_types.h"
#include "<"$comp">_service.h"
#include "<"$comp">_action.h"

namespace genom {
  namespace <"$comp">_remote {
<'foreach r [$component remotes] {'>

    /* <"[--- [$r name] -------------------------------------------------]"> */

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
    struct <"[$r name]"> {
      <"[[$r type] declarator handle]">;
      genom::srv_<"[$r name]">::input in;

<'  switch -- "[$r kind]" {'>
<'    attribute - function {'>
      typedef ros::ServiceClient client;
      client c;
<'    }'>
<'    activity {'>
      typedef
        actionlib::ActionClient< genom::action_<"[$r name]"> > client;
      client *c;
      ros::CallbackQueue cbq;
<'    }'>
<'    default { error "unsupported remote kind" }'>
<'  }'>

      <"[$r name]">() {
        handle.call = NULL;
<'  switch -- "[$r kind]" {'>
<'    attribute - function {'>

<'    }'>
<'    activity {'>
        c = NULL;
<'    }'>
<'  }'>
      }

      ~<"[$r name]">() {
<'  switch -- "[$r kind]" {'>
<'    attribute - function {'>

<'    }'>
<'    activity {'>
        if (c) delete c;
<'    }'>
<'  }'>
      }

      genom_event connect(const char remote[128], genom_context self);
      genom_event call(<"$arg">);
    };

<'}'>
  };
};

#endif /* H_GROS_<"$COMP">_REMOTE */

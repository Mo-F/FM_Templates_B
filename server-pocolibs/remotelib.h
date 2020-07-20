<'
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
#                                      Anthony Mallet on Mon Jul 30 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]

lang c
'>
#ifndef H_<"$COMP">_REMOTELIB
#define H_<"$COMP">_REMOTELIB

#include <stdint.h>

#include "csLib.h"

#include "<"$comp">_c_types.h"
#include "serialize.h"
#include "typecopy.h"


/* === Remote structures =================================================== */

struct genom_remote {
  pocolibs_metadata_component meta;
  CLIENT_ID csid;
  int id;
};

<'foreach r [$component remotes] {'>

/* <"[--- [$r name] -----------------------------------------------------]"> */

struct genom_<"$comp">_<"[$r name]">_remote {
  struct genom_remote h;
  <"[[$r type] declarator data]">;
};

/* initializer */
static __inline__ void
genom_tinit_<"$comp">_<"[$r name]">_remote(
  struct genom_<"$comp">_<"[$r name]">_remote *r)
{
  genom_tinit_<"[[dotgen types ::pocolibs::metadata::component] mangle]">(
    &r->h.meta);
  r->h.csid = NULL;
  r->h.id = 0;
  r->data.call = NULL;
}

/* finalizer */
static __inline__ void
genom_tfini_<"$comp">_<"[$r name]">_remote(
  struct genom_<"$comp">_<"[$r name]">_remote *r)
{
  if (r->h.csid) { csClientEnd(r->h.csid); r->h.csid = NULL; }
  genom_tfini_<"[[dotgen types ::pocolibs::metadata::component] mangle]">(
    &r->h.meta);
  r->data.call = NULL;
}

/* maximum request size */
static __inline__ size_t
genom_<"$comp">_<"[$r name]">_maxrqst_size(void)
{
  size_t s = 0;

  /* parameters */
<'  foreach p [$r parameters in inout] {'>
  s += genom_maxserialen_<"[[$p type] mangle]">();
<'  }'>
  return s;
}

/* maximum reply size */
static __inline__ size_t
genom_<"$comp">_<"[$r name]">_maxreply_size(void)
{
<'  if {[llength  [$r throw]]} {'>
  size_t s;
<'  }'>
  size_t m = 1 /* ok flag */;

  /* parameters */
<'  foreach p [$r parameters inout out] {'>
  m += genom_maxserialen_<"[[$p type] mangle]">();
<'  }'>

  /* throws */
<'  foreach t [$r throw] {'>
  s = sizeof(<"[$t cname]">);
  s += genom_maxserialen_<"[$t mangle]">();
  if (m < s) m = s;
<'  }'>

  return m;
}

genom_event	genom_<"$comp">_<"[$r name]">_connect(
          const char *name, genom_context self);

<'}'>

#endif /* H_<"$COMP">_REMOTELIB */

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
#                                      Anthony Mallet on Thu Mar  2 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]

lang c
'>
#ifndef H_<"$COMP">_ACTIVITY
#define H_<"$COMP">_ACTIVITY

#include "typecopy.h"
#include "<"$comp">_parameters.h"

/* forward declaration */
struct genom_component_data;


enum genom_activity_status {
  ACT_VOID,	/* nothing, available */
  ACT_ACT	/* active, not available */
};

struct genom_activity {
  int sid;	/* service number, -1 for permanent activity */
  int aid;	/* activity number */
  int rid;	/* csLib request id */

  enum genom_activity_status status;	/* current phase */
  int start, stop, pause;		/* status change request */
  genom_event state;			/* current FSM state */

  void *exdetail;			/* exception info, if any */
  const char *interruptedby;
};

typedef struct genom_activity *genom_activity_ptr;

/* === Service activity ==================================================== */

<'foreach s [$component services] {'>

/* <"[--- [$s name] -----------------------------------------------------]"> */

/* activity data */
struct genom_<"$comp">_<"[$s name]">_activity {
  struct genom_activity h;
  struct genom_<"$comp">_<"[$s name]">_input in;
  struct genom_<"$comp">_<"[$s name]">_output out;
<'  if {[llength [$s parameters local]]} {'>
  struct genom_<"$comp">_<"[$s name]">_locals locals;
<'  }'>
};

/* input/output initializer */
static __inline__ void
genom_tinit_<"$comp">_<"[$s name]">_activity(
  struct genom_<"$comp">_<"[$s name]">_activity *a)
{
<'  foreach p [$s parameters in inout] {'>
  genom_tinit_<"[[$p type] mangle]">(
    <"[[$p type] pass reference a->in.[$p name]]">);
<'  }'>
<'  foreach p [$s parameters inout out] {'>
  genom_tinit_<"[[$p type] mangle]">(
    <"[[$p type] pass reference a->out.[$p name]]">);
<'  }'>
<'  foreach p [$s parameters local] {'>
  genom_tinit_<"[[$p type] mangle]">(
    <"[[$p type] pass reference a->locals.[$p name]]">);
<'  }'>
}

/* input/output finalizer */
static __inline__ void
genom_tfini_<"$comp">_<"[$s name]">_activity(
  struct genom_<"$comp">_<"[$s name]">_activity *a)
{
<'  foreach p [$s parameters in inout] {'>
  genom_tfini_<"[[$p type] mangle]">(
    <"[[$p type] pass reference a->in.[$p name]]">);
<'  }'>
<'  foreach p [$s parameters inout out] {'>
  genom_tfini_<"[[$p type] mangle]">(
    <"[[$p type] pass reference a->out.[$p name]]">);
<'  }'>
<'  foreach p [$s parameters local] {'>
  genom_tfini_<"[[$p type] mangle]">(
    <"[[$p type] pass reference a->locals.[$p name]]">);
<'  }'>
}

<'}'>


/* === Activities ========================================================== */

union genom_any_activity {
  struct genom_activity any;
<'foreach s [$component services] {'>
  struct genom_<"$comp">_<"[$s name]">_activity s_<"[$s name]">;
<'}'>
};

struct genom_activities {
  union genom_any_activity a[16]; /* Un peu arbitraire, non? */
};

static inline size_t
genom_max_activities(void) {
  struct genom_activities *a;
  return sizeof(a->a)/sizeof(a->a[0]);
}

int	genom_<"$comp">_activity_alloc(struct genom_activities *activities);
void	genom_<"$comp">_activity_report(struct genom_component_data *self,
		struct genom_activity *a);
int	genom_<"$comp">_activity_encodex(char *buffer, int size, char *dst,
                int maxsize);
int	genom_<"$comp">_interrupt_reqd(struct genom_component_data *self,
		struct genom_activity *a);

#endif /* H_<"$COMP">_ACTIVITY */

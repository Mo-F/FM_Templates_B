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
#                                      Anthony Mallet on Wed Mar  7 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]

lang c
'>
#include "autoconf/acheader.h"

#include "<"$comp">_control_task.h"
#include "<"$comp">_activity.h"


/* <"[--- genom_${comp}_activity_alloc ----------------------------------]"> */

int
genom_<"$comp">_activity_alloc(struct genom_activities *activities)
{
  static int aid = 0;
  int id;

  /* look for the first free activity */ /* useless? */ /* not really, as this table store the
     real structure which we DO need */
  for(id = 0; id < genom_max_activities(); id++) {
    if (activities->a[id].any.status == ACT_VOID) { /* We should nake sure that this status is handled properly. */
      activities->a[id].any.aid = ++aid;
      activities->a[id].any.status = ACT_ACT;
      activities->a[id].any.start = 0;
      activities->a[id].any.stop = 0;
      activities->a[id].any.pause = 0;
      return id;
    }
  }

  return -1;
}


/* --- genom_<"$comp">_activity_encodex ------------------------------------ */

int
genom_<"$comp">_activity_encodex(char *buffer, int size, char *dst, int maxsize)
{
  struct genom_activity *a = (struct genom_activity *)buffer; /*casting*/

  if (a->state == genom_unkex_id)
    return genom_<"$comp">_genom_unkex_encodex(a->exdetail, size, dst, maxsize);
  else if (a->state == genom_syserr_id)
    return genom_<"$comp">_genom_syserr_encodex(a->exdetail, size, dst, maxsize);
<'foreach e [$component throws] {'>
  else if (a->state == <"[$e cname]">_id)
    return genom_<"$comp">_<"[$e cname]">_encodex(
      a->exdetail, size, dst, maxsize);
<'}'>

  assert(!"unknown genom exception");
  return 0;
}


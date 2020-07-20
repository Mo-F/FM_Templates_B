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
#                                      Anthony Mallet on Mon Jan 16 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]

lang c
'>
#include "autoconf/acheader.h"

#include <assert.h>
#include <err.h>

#include "<"$comp">_internals.h"

/* --- genom_<"$comp">_activities::alloc --------------------------------------------- */

genom_event
genom_<"$comp">_activities::alloc(genom_<"$comp">_activity *i)
{
  static int aid = 0;

  /* look for the first free activity */
  for(size_t id = 0; id < MAX_ACTIVITIES; id++) {
    if (a[id] == NULL) {
      i->iid = id;
      i->aid = ++aid;
      i->status = ACT_INIT;
      i->start = 0;
      i->stop = 0;
      i->pause = 0;
      a[id] = i;
      return genom_ok;
    };
  }

  return genom_too_many_activities(&i->self->control.context);
}


/* --- genom_<"$comp">_activities::bygid --------------------------------------------- */

genom_<"$comp">_activity *
genom_<"$comp">_activities::bygid(const actionlib_msgs::GoalID &gid)
{
  size_t id;

  /* iterate through allocated activities */
  for(id = 0; id < MAX_ACTIVITIES; id++) {
    if (a[id] == NULL) continue;
    if (a[id]->gid(gid)) return a[id];
  }

  return NULL;
}


/* --- genom_<"$comp">_activities::call ---------------------------------------------- */

/** activity report callback
 */
genom_<"$comp">_activity::CallResult
genom_<"$comp">_activity::call(void) {
  self->activity_report(this);
  return Success;
}


/* === Interrupt incompatible activities =================================== */

<'foreach s [$component services] {'>
<'  if {[$s kind] == "activity"} {'>
template<> int
genom_<"$comp">_activity_service_<"[$s name]">::interrupt_other(
  genom_<"$comp">_component_data *self) {
  return self-><"[$s name]">_interrupt_other(aid);
}

<'  }'>
int
genom_<"$comp">_component_data::<"[$s name]">_interrupt_other(int aid)
{
  int delay = 0;

<'  foreach task [$component tasks] {'>
<'    set clist [list]'>
<'    foreach c [$s interrupt] {'>
<'      if {[catch {$c task} t]} continue'>
<'      if {$t eq $task} { lappend clist $c }'>
<'    }'>
<'    if {[llength $clist] == 0} continue'>

  /* task <"[$task name]"> */
    //  pthread_mutex_lock(&tasks.<"[$task name]">_.lock);
  {
    for(size_t id = 0; id < genom_<"$comp">_activities::MAX_ACTIVITIES; id++) {
      genom_<"$comp">_activity *c = tasks.<"[$task name]">_.activities.a[id];
      if (!c) continue;
      if (c->status == ACT_VOID || c->status == ACT_ETHER) continue;
      if (c->aid == aid) continue;
      switch(c->sid) {
<'    foreach c $clist {'>
        case <"$COMP">_<"[$c name]">_RQSTID:
<'    }'>
          switch(c->status) {
            case ACT_RUN:
              delay = 1;
              if (!tasks.<"[$task name]">_.runnable) {
                tasks.<"[$task name]">_.runnable = 1;
		//                pthread_cond_broadcast(&tasks.<"[$task name]">_.sync);
              }
              /*FALLTHROUGH*/
            case ACT_INIT:
              c->stop = 1;
              c->interruptedby = "<"[$s name]">";
              break;

            case ACT_STOP: delay = 1; break;

            default: assert(!"code not reached");
          }
      }
    }
  }
  //  pthread_mutex_unlock(&tasks.<"[$task name]">_.lock);
<'  }'>

  return delay;
}

<'}'>

/*eof*/

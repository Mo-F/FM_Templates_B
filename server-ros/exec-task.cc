<'
# Copyright (c) 2012-2016 LAAS/CNRS
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
#                                      Anthony Mallet on Sat Jan 14 2012
#

if {[llength $argv] != 2} { error "expected arguments: component task" }
lassign $argv component task

# compute handy shortcuts
set comp [$component name]
set tname [$task name]

lang c
'>
#include "autoconf/acheader.h"

#include <sys/time.h>

#include "<"$comp">_internals.h"


/* <"[--- ${comp}_${tname}_task -----------------------------------------]"> */

/** <"$tname"> task of component <"$comp">
 */

void *
<"$comp">_<"$tname">_task(void *data)
{
  enum {
    GENOM_SCHED_ONE = 2,
    GENOM_SCHED_CONT = 4,
    GENOM_SCHED_WAKE = 8
  };
  genom_component_data *self = (genom_component_data *)data;
  genom_exec_task &task = self->tasks.<"$tname">_;
  genom_state_rusage *usage = &task.rusage;
  struct timeval tvs, tve;
  genom_activity *a;
  genom_activity_status e;
  int sched;
  size_t id;

  /* run */
  genom_log_info("spawned task <"[$task name]">");
  sched = GENOM_SCHED_CONT;
  gettimeofday(&tvs, NULL);
  tve = tvs;

  do {
    /* wait for next event */
    pthread_mutex_lock(&task.lock);
<'if {[catch {$task period}]} {'>
    if (!(sched & GENOM_SCHED_CONT)) {
      /* timeout after 1s to update rusage */
      struct timespec tout = { tvs.tv_sec + 1,  tvs.tv_usec * 1000 };
      while (!task.runnable)
        if (pthread_cond_timedwait(&task.sync, &task.lock, &tout))
          break;
      genom_log_debug("<"[$task name]"> tick");
    }
    gettimeofday(&tvs, NULL);
<'} else {'>
    if (!(sched & GENOM_SCHED_CONT)) {
      while (!task.wakeup) pthread_cond_wait(&task.sync, &task.lock);
      task.wakeup = 0;
      gettimeofday(&tvs, NULL);
      genom_log_debug("<"[$task name]"> tick");
    }
    /* paused activities are only awaken on a period tick */
    task.runnable = !(sched & GENOM_SCHED_CONT);
<'}'>
    sched = 0;

    if (task.shutdown) {
      pthread_mutex_unlock(&task.lock);
      break;
    }

    /* handle start/stop requests, wake up paused activities */
    for(id = 0; id < genom_activities::MAX_ACTIVITIES; id++) {
      a = task.activities.a[id];
      if (!a) continue;
      switch(a->status) {
        case ACT_VOID: case ACT_ETHER: continue;

        case ACT_INIT:
          if (a->start && task.runnable) {
            /* start request, only on a tick in periodic tasks */
            a->status = ACT_RUN;
            a->state = <"${comp}">_start;
            a->start = 0;
            a->pause = 0;
          }
          if (a->stop) {
            /* stop before start: raise 'interrupted' exception, except for
             * the permanent activity for which we still go to stop */
            if (a == task.permanent) {
              a->status = ACT_STOP;
              a->state = <"${comp}">_stop;
              a->stop = 0;
              a->pause = 0;
            } else {
              genom_interrupted_detail d;
              assert(a->interruptedby);
              strncpy(d.by, a->interruptedby, sizeof(d.by));
              d.by[sizeof(d.by)-1] = *"";
              genom_interrupted(&d, &task.context);

              a->status = ACT_ETHER;
              a->stop = 0;
              a->pause = 0;
              a->exdetail =
                (void *)task.context.raised(&a->state, &task.context);
              <"$comp">_cntrl_task_signal(a);
            }
          }
          break;

        case ACT_RUN:
          if (a->stop) {
            a->status = ACT_STOP;
            a->state = <"${comp}">_stop;
            a->stop = 0;
            a->pause = 0;
          }
          /*FALLTHROUGH*/
        case ACT_STOP:
          if (a->pause && task.runnable) {
            a->pause = 0;
          }
          break;
      }
    }
    task.runnable = 0;

    /* run activities */
    for(id = 0; id < genom_activities::MAX_ACTIVITIES; id++) {
      a = task.activities.a[id];
      if (!a) continue;
      if (a->status != ACT_RUN && a->status != ACT_STOP) continue;
      if (a->pause) continue;

      pthread_mutex_unlock(&task.lock);
      e = a->invoke();
      pthread_mutex_lock(&task.lock);
      switch(e) {
        case ACT_RUN:
          if (!a->pause)
            sched |= GENOM_SCHED_CONT | GENOM_SCHED_WAKE;
          break;

        case ACT_ETHER:
          if (a->state == NULL /* exception raised */)
            a->exdetail = (void *)self->tasks.<"$tname">_.context.raised(
              &a->state, &self->tasks.<"$tname">_.context);
          a->status = ACT_ETHER;
          <"$comp">_cntrl_task_signal(a);
          sched |= GENOM_SCHED_WAKE;
          break;

        case ACT_VOID: case ACT_INIT: case ACT_STOP:
          assert(!"not reached");
      }
      sched |= GENOM_SCHED_ONE;
    }

    if (sched & GENOM_SCHED_ONE) pthread_cond_broadcast(&task.sync);
    if (sched & GENOM_SCHED_WAKE) task.runnable = 1;
    pthread_mutex_unlock(&task.lock);

    /* send signals */
    if (sched & GENOM_SCHED_WAKE) {
<'foreach t [lsearch -inline -all -not [$component tasks] $task] {'>
      pthread_mutex_lock(&self->tasks.<"[$t name]">_.lock);
      if (!self->tasks.<"[$t name]">_.runnable) {
        self->tasks.<"[$t name]">_.runnable = 1;
        pthread_cond_broadcast(&self->tasks.<"[$t name]">_.sync);
      }
      pthread_mutex_unlock(&self->tasks.<"[$t name]">_.lock);
<'}'>
    }

    /* update rusage */
<'if {![catch {$task period}]} {'>
    if (sched & GENOM_SCHED_CONT) /* at each period only */ continue;
<'}'>
    {
      double last;
      double dt;
      genom_state_rusage u = *usage;

<'if {[catch {$task period} period]} {'>
      struct timeval tvp = tve;
      gettimeofday(&tve, NULL);
      dt = tve.tv_sec - tvp.tv_sec + (tve.tv_usec - tvp.tv_usec)*1e-6;
<'} else {'>
      gettimeofday(&tve, NULL);
      dt = <"[$period value]">;
<'}'>

      last = tve.tv_sec - tvs.tv_sec + (tve.tv_usec - tvs.tv_usec)*1e-6;
      if (sched & GENOM_SCHED_ONE) {
        u.cycles++;

        u.timings.last = last;
        u.load.last = 100. * last / dt;

        if (u.timings.max < u.timings.last) u.timings.max = u.timings.last;
        if (u.load.max < u.load.last) u.load.max = u.load.last;
      }

      if (dt > 10.) dt = 10.;
      u.timings.avg += (last - u.timings.avg) * dt / 10.;
      u.load.avg += (100. * last - dt * u.load.avg) / 10.;

      pthread_spin_lock(&task.rlock);
      *usage = u;
      pthread_spin_unlock(&task.rlock);
    }
  } while(1);

  genom_log_info("shutting down <"$tname"> task");

  pthread_mutex_lock(&task.lock);
  task.spawned = false;
  pthread_cond_broadcast(&task.sync);
  pthread_mutex_unlock(&task.lock);
  return NULL;
}


/* === codels ============================================================== */

<'foreach obj [list $task {*}[$task services]] {'>

/* <"[--- [$obj class] [$obj name] --------------------------------------]"> */
<'  foreach state [$obj fsm] {'>
<'    set codel [$obj fsm $state]'>
<'    set func [$obj class]_[$obj name]_[$state name]'>

/* state <"[$state name]"> */
static inline genom_event
<"$comp">_codel_<"$func">(
  genom_component_data *self, <"genom_activity_[$obj class]_[$obj name]"> *a)
{
  genom_event s;

  genom_take_resource(
    self,
<'    foreach m [$codel mutex] {'>
<'      switch [$m class] {'>
<'        codel { set rname [$m cname] }'>
<'        service { set rname genom_component_data::[$m name]_resource }'>
<'        default { error "unsupported object" }'>
<'      }'>
<'      if {[catch {$m task} t]} {'>
    self->resources.control == <"$rname"> ||
<'      } else {'>
    self->resources.task_<"[$t name]"> == <"$rname"> ||
<'      }'>
<'    }'>
    self->resources.all,

    self->resources.task_<"[$task name]"> = (void *)<"[$codel cname]">);
<'
    # build parameter list
    set plist [list]
    foreach p [$codel parameters] {
      switch -glob -- [$p src] {
        local	{
          switch -- [[$p param] dir] {
            local	{ set m "a->locals.[$p name]" }
            in		{ set m "a->in.[$p name]" }
            inout - out { set m "a->out.[$p name]" }
            default	{ error "unsupported local direction" }
          }
        }
        remote		{ set m "self->remotes.[[$p remote] name]_.handle" }
        port		{ set m "self->ports.[[$p port] name]_.handle" }
        ids		{ set m "self->ids[$p member]" }
        default		{ error "reached unreachable code" }
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
    lappend plist "&self->tasks.[$task name]_.context"
'>
  s = <"[$codel invoke $plist]">;
  genom_give_resource(self, self->resources.task_<"[$task name]"> = NULL);
  return s;
}

<'  }'>
/* invoke codels according to <"[$obj name]"> state */
template<>
genom_activity_status
<"genom_activity_[$obj class]_[$obj name]">::invoke()
{
  genom_event s;

  genom_log_debug(
    "<"task [$task name] invoking [$obj class] [$obj name] %s">",
    state);

<'  foreach state [$obj fsm] {'>
<'    set func [$obj class]_[$obj name]_[$state name]'>
  if (state == <"[$state cname]">) {
    s = <"$comp">_codel_<"$func">(self, this);
    genom_log_debug("<"[$obj class] [$obj name]"> yielded %s", s);
    if (
<'  foreach y [[$obj fsm $state] yield] {'>
<'    if {"pause" in [$y kind]} continue '>
<'    if {"ether" eq [$y name]} continue'>
      s == <"[$y cname]"> ||
<'  }'>
      0) {
      state = s;
      return ACT_RUN;
    }
    if (
<'  foreach y [[$obj fsm $state] yield] {'>
<'    if {"pause" in [$y kind]} {'>
      (s == <"[$y cname]"> && (s = <"$comp">_<"[$y name]">)) ||
<'    }'>
<'  }'>
      0) {
      state = s;
      pause = true;
      return ACT_RUN;
    }
    if (
<'  foreach y [[$obj fsm $state] yield] {'>
<'    if {"ether" eq [$y name]} {'>
      s == <"[$y cname]"> ||
<'    }'>
<'  }'>
      0) {
      state = <"$comp">_ether;
      return ACT_ETHER;
    }
    goto ex;
  }
<'  }'>

<'  if {[$obj class] eq "task"} {'>
  /* default start, if not already handled above */
  if (state == <"$comp">_start) {
    state = <"$comp">_ether;
    return ACT_ETHER;
  }
<'  }'>

  /* default stop, if not already handled above */
  if (state == <"$comp">_stop) {
    genom_interrupted_detail d;

    assert(interruptedby);
    strncpy(d.by, interruptedby, sizeof(d.by));
    d.by[sizeof(d.by)-1] = *"";
    s = genom_interrupted(&d, &task->context);
    goto ex;
  }

  /* this cannot happen by construction */
  genom_log_warn("bad state (%s) in service <"[$obj class]_[$obj name]">",
                 state?state : "genom_ok");
  abort();
  /*NOTREACHED*/

ex:
  if (
<'  foreach t [$obj throw] {'>
    s != <"[$t cname]">_id &&
<'  }'>
    1) {
    genom_bad_transition_detail d;
    strncpy(d.from, state, sizeof(d.from)); d.from[sizeof(d.from)-1] = *"";
    strncpy(d.to, s, sizeof(d.to)); d.to[sizeof(d.to)-1] = *"";
    genom_log_warn(
      "bad transition from %s to %s in <"[$obj class] [$obj name]">",
      state, s?s:"genom_ok");
    genom_bad_transition(&d, &task->context);
  }

  state = NULL;
  return ACT_ETHER;
}

<'}'>

/*eof*/

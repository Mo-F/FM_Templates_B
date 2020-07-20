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
#                                      Anthony Mallet on Tue Dec 13 2011
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]

lang c
'>
#include "autoconf/acheader.h"

#include <cstring>
#include <unistd.h>
#include <signal.h>

#include "ros/ros.h"
#include "ros/callback_queue.h"
#include "actionlib/server/action_server.h"

#include "<"$comp">_internals.h"
#include "<"$comp">_typecopy.h"


/* <"[--- ${comp}_cntrl_task --------------------------------------------]"> */

/** Control task of component <"$comp">
 *
 * This task waits for incoming messages or internal events. Upon reception
 * of a request, the corresponding callback function is invoked. Internal
 * events are handled and final replies are sent to the client.
 */

int
<"$comp">_cntrl_task(const char *instance)
{
  genom_component_data cids;
  int s;

  cids.control.context.raise = ros_server_raise;
  cids.control.context.raised = ros_server_raised;
  cids.control.context.data = &cids.control.context_data;

  cids.control.context_data.self = &cids;
  cids.control.context_data.ex = genom_ok;
  cids.control.context_data.exdetail = NULL;
  cids.control.context_data.exsize = 0;

  cids.control.node = new ros::NodeHandle(instance);
  cids.control.queue = ros::getGlobalCallbackQueue();

  memset(cids.control.run_map, 0, sizeof(cids.control.run_map));
  pthread_mutex_init(&cids.control.lock, NULL);
  pthread_cond_init(&cids.control.sync, NULL);

<'foreach s [$component services] {'>
<'  if {[$s kind] ne "activity"} continue'>
  cids.services.<"[$s name]">_ = NULL;
<'}'>

  cids.resources.all = 0;
  cids.resources.control = NULL;
<'foreach t [$component tasks] { '>
  cids.resources.task_<"[$t name]"> = NULL;
<'}'>
  cids.resources.q = 0;
  cids.resources.qnext = 0;
  pthread_mutex_init(&cids.resources.lock, NULL);
  pthread_cond_init(&cids.resources.sync, NULL);

  /* disable actionlib status updates when goal status does not change */
  cids.control.node->setParam("actionlib_status_frequency", 0);

<'if {![catch {$component ids}]} {'>
  genom::ids::pinit(cids.ids);
<'}'>

  /* advertise data outports */
  genom_log_info("advertising ports");
<'foreach p [$component ports] {'>
  s = cids.ports.<"[$p name]">_.init(cids.control.node);
  if (s) errx(2, "error while initializing <"[$p name]"> port");
<'}'>
  s = cids.genom_state_init();
  if (s) errx(2, "error while initializing state port");

  /* initilize tasks */
<'foreach t [$component tasks] {'>
  cids.tasks.<"[$t name]">_.context.raise = ros_server_raise;
  cids.tasks.<"[$t name]">_.context.raised = ros_server_raised;
  cids.tasks.<"[$t name]">_.context.data = &cids.tasks.<"[$t name]">_.context_data;

  cids.tasks.<"[$t name]">_.context_data.self = &cids;
  cids.tasks.<"[$t name]">_.context_data.ex = genom_ok;
  cids.tasks.<"[$t name]">_.context_data.exdetail = NULL;
  cids.tasks.<"[$t name]">_.context_data.exsize = 0;

  cids.tasks.<"[$t name]">_.rusage.cycles = 0;
  cids.tasks.<"[$t name]">_.rusage.timings.last = 0.;
  cids.tasks.<"[$t name]">_.rusage.timings.max = 0.;
  cids.tasks.<"[$t name]">_.rusage.timings.avg = 0.;
  cids.tasks.<"[$t name]">_.rusage.load.last = 0.;
  cids.tasks.<"[$t name]">_.rusage.load.max = 0.;
  cids.tasks.<"[$t name]">_.rusage.load.avg = 0.;
  pthread_spin_init(&cids.tasks.<"[$t name]">_.rlock, 0);

  cids.tasks.<"[$t name]">_.num = <"${comp}_[$t name]_TASKID">;
  cids.tasks.<"[$t name]">_.spawned = false;
  cids.tasks.<"[$t name]">_.wakeup = false;
  cids.tasks.<"[$t name]">_.runnable = true;
  cids.tasks.<"[$t name]">_.shutdown = false;

  for(size_t i = 0; i < genom_activities::MAX_ACTIVITIES; i++)
    cids.tasks.<"[$t name]">_.activities.a[i] = NULL;
  cids.tasks.<"[$t name]">_.permanent = new genom_activity_task_<"[$t name]">;
  if (!cids.tasks.<"[$t name]">_.permanent) err(2, "<"[$t name]">");
  cids.tasks.<"[$t name]">_.permanent->self = &cids;
  cids.tasks.<"[$t name]">_.permanent->task = &cids.tasks.<"[$t name]">_;
  cids.tasks.<"[$t name]">_.permanent->sid = -1;
  if (cids.tasks.<"[$t name]">_.activities.alloc(
        cids.tasks.<"[$t name]">_.permanent) != genom_ok)
    errx(2, "<"[$t name]">: permanent activity");

  pthread_mutex_init(&cids.tasks.<"[$t name]">_.lock, NULL);
  pthread_cond_init(&cids.tasks.<"[$t name]">_.sync, NULL);

<'}'>

<'if {![catch {$component clock-rate} rate]} {'>
  /* create timer */
  {
    struct itimerspec ts;
    if (timer_create(CLOCK_REALTIME, NULL, &cids.clock))
      err(2, "timer_create");

    if (pthread_create(&cids.clockthread, NULL,
                       genom_component_data::timercb, &cids))
      err(2, "timer thread create");
    pthread_detach(cids.clockthread);

<'  set rate [$rate value]'>
    ts.it_interval.tv_sec =
      <"[expr {int($rate)}]">;
    ts.it_interval.tv_nsec =
      <"[expr {int($rate*1000000000) % 1000000000}]">;
    ts.it_value = ts.it_interval;
    if (timer_settime(cids.clock, 0, &ts, NULL))
      err(2, "timer thread create");
  }
<'}'>

  /* spawn tasks */
<'foreach t [$component tasks] {'>
  cids.tasks.<"[$t name]">_.spawn(&cids,
                    "<"[$t name]">", <"$comp">_<"[$t name]">_task);
  if (!cids.tasks.<"[$t name]">_.spawned) goto shutdown;
<'}'>

  /* advertise services */
  genom_log_info("advertising services");
<'foreach s [$component services] {'>
<'  if {[catch {$s task} t]} {'>
  cids.services.<"[$s name]">_ =
    cids.control.node->advertiseService(
      "<"[$s name]">", &genom_component_data::<"[$s name]">_rqstcb, &cids);
<'  } else {'>
  cids.services.<"[$s name]">_ =
    new actionlib::ActionServer< genom::action_<"[$s name]"> >(
      *cids.control.node, "<"[$s name]">",
      boost::bind(&genom_component_data::<"[$s name]">_rqstcb, &cids, _1),
      boost::bind(&genom_component_data::<"[$s name]">_intercb, &cids, _1),
      false);
  cids.services.<"[$s name]">_->start();
<'  }'>
<'}'>

  {
    /*
     * If SIGUSR1 is set to SIG_IGN, it can be two things:
     *   - the parent really ignores it, so it won't care about it
     *   - the parent want to significate that it have a special handler to deal
     *   with it. In this case, it is valuable to send it. It is in particular
     *   true in the case of the daemon mode.
     */
    sig_t handler = signal(SIGUSR1, SIG_IGN);
    if (handler == SIG_IGN) kill(getppid(), SIGUSR1);
    signal(SIGUSR1, handler);
  }

  /* go */
  genom_log_info("control task initialized and running");
  while(ros::ok() && !genom_shutdown) {
    cids.control.queue->callAvailable(ros::WallDuration(1.));
    cids.genom_state_refresh();
  }
  goto shutdown;

shutdown:
  /* interrupt & wait for all regular activities */
<'foreach t [$component tasks] {'>
  if (cids.tasks.<"[$t name]">_.spawned) {
    int delay;

    genom_log_debug("interrupting task <"[$t name]"> activities");
    pthread_mutex_lock(&cids.tasks.<"[$t name]">_.lock);
    do {
      delay = 0;
      for(size_t id = 0; id < genom_activities::MAX_ACTIVITIES; id++) {
        genom_activity *a = cids.tasks.<"[$t name]">_.activities.a[id];
        if (!a) continue;
        if (a == cids.tasks.<"[$t name]">_.permanent) continue;

        if (a->status == ACT_INIT || a->status == ACT_RUN) {
          if (!a->stop) {
            a->stop = 1;
            a->interruptedby = "kill";
            if (!cids.tasks.<"[$t name]">_.runnable) {
              cids.tasks.<"[$t name]">_.runnable = 1;
              pthread_cond_broadcast(&cids.tasks.<"[$t name]">_.sync);
            }
          }
          delay = 1;
        } else if (a->status == ACT_STOP)
          delay = 1;
      }
      if (delay) {
        pthread_cond_wait(&cids.tasks.<"[$t name]">_.sync,
                          &cids.tasks.<"[$t name]">_.lock);
        pthread_mutex_unlock(&cids.tasks.<"[$t name]">_.lock);
        cids.control.queue->callAvailable();
        cids.genom_state_refresh();
        pthread_mutex_lock(&cids.tasks.<"[$t name]">_.lock);
      }
    } while(delay);
    pthread_mutex_unlock(&cids.tasks.<"[$t name]">_.lock);
  }

<'}'>
  /* interrupt permanent activities */
<'foreach t [lreverse [$component tasks]] {'>
  if (cids.tasks.<"[$t name]">_.spawned) {
    genom_log_debug("interrupting task <"[$t name]">");
    pthread_mutex_lock(&cids.tasks.<"[$t name]">_.lock);

    genom_activity *a = cids.tasks.<"[$t name]">_.permanent;
    if (!a) {
      /* activity was ether, realloc one */
      a = new genom_activity_task_<"[$t name]">;
      if (!a) err(2, "<"[$t name]">");
      a->self = &cids;
      a->task = &cids.tasks.<"[$t name]">_;
      a->sid = -1;
      if (cids.tasks.<"[$t name]">_.activities.alloc(a) != genom_ok)
        errx(2, "<"[$t name]">: permanent activity");
      cids.tasks.<"[$t name]">_.permanent = a;
    }
    if (a->status != ACT_STOP) {
      if (a->status == ACT_ETHER) a->status = ACT_INIT;
      a->stop = 1;
      a->interruptedby = "kill";
      cids.tasks.<"[$t name]">_.runnable = 1;
      pthread_cond_broadcast(&cids.tasks.<"[$t name]">_.sync);
    }

    while (cids.tasks.<"[$t name]">_.permanent &&
           cids.tasks.<"[$t name]">_.permanent->status != ACT_ETHER) {
      pthread_cond_wait(&cids.tasks.<"[$t name]">_.sync,
                        &cids.tasks.<"[$t name]">_.lock);
      pthread_mutex_unlock(&cids.tasks.<"[$t name]">_.lock);
      cids.control.queue->callAvailable();
      cids.genom_state_refresh();
      pthread_mutex_lock(&cids.tasks.<"[$t name]">_.lock);
    }
    pthread_mutex_unlock(&cids.tasks.<"[$t name]">_.lock);
  }

<'}'>
  /* wait for exec tasks */
<'foreach t [lreverse [$component tasks]] {'>
  if (cids.tasks.<"[$t name]">_.spawned) {
    genom_log_debug("waiting task <"[$t name]">");
    pthread_mutex_lock(&cids.tasks.<"[$t name]">_.lock);
    cids.tasks.<"[$t name]">_.shutdown = 1;
    cids.tasks.<"[$t name]">_.runnable = 1;
    pthread_cond_broadcast(&cids.tasks.<"[$t name]">_.sync);
    while (cids.tasks.<"[$t name]">_.spawned) {
      pthread_cond_wait(&cids.tasks.<"[$t name]">_.sync,
                        &cids.tasks.<"[$t name]">_.lock);
      pthread_mutex_unlock(&cids.tasks.<"[$t name]">_.lock);
      cids.control.queue->callAvailable();
      cids.genom_state_refresh();
      pthread_mutex_lock(&cids.tasks.<"[$t name]">_.lock);
    }
    pthread_join(cids.tasks.<"[$t name]">_.id, NULL);
    pthread_mutex_unlock(&cids.tasks.<"[$t name]">_.lock);
  }

<'}'>

  genom_log_info("shutting down services");
<'foreach s [$component services] {'>
<'  if {[catch {$s task} t]} {'>
  cids.services.<"[$s name]">_.shutdown();
<'  } else {'>
  if (cids.services.<"[$s name]">_)
    delete cids.services.<"[$s name]">_;
<'  }'>
<'}'>

  genom_log_info("shutting down ports");
<'foreach p [$component ports] {'>
  cids.ports.<"[$p name]">_.fini();
<'}'>

  genom_log_info("shutting down control task");
<'if {![catch {$component clock-rate} rate]} {'>
  pthread_cancel(cids.clockthread);
<'}'>
<'if {![catch {$component ids}]} {'>
  genom::ids::pfini(cids.ids);
<'}'>
  cids.control.node->shutdown();
  delete cids.control.node;

  genom_log_info("shutdown complete");
  return 0;
}


/* <"[--- genom_exec_task::spawn ----------------------------------------]"> */

void
genom_exec_task::spawn(genom_component_data *self,
                       const char *name, void *(*start)(void *))
{
  int s;

  pthread_mutex_lock(&lock);
  spawned = true;
  permanent->start = 1;

  s = pthread_create(&id, NULL, start, self);
  if (s) err(2, "spawning task %s", name);
  while (permanent->start)
    pthread_cond_wait(&sync, &lock);

  if (!spawned) {
    genom_log_warn("cannot spawn %s exec task", name);
    pthread_mutex_unlock(&lock);
    return;
  }
  if (permanent->status == ACT_ETHER && permanent->state != <"$comp">_ether) {
    genom_log_warn("%s exec task failed", name);
    pthread_mutex_unlock(&lock);
    spawned = false;
    return;
  }
  self->genom_state_update(this);
  pthread_mutex_unlock(&lock);

  self->genom_state_refresh();
}


/* <"[--- ${comp}_cntrl_task_signal -------------------------------------]"> */

/** \brief Signal the control task that an activity is terminated
 */

static const struct genom_activity_cbd_ {
  void operator()(genom_activity *) {}
} genom_activity_cbd = {};

void
<"$comp">_cntrl_task_signal(genom_activity *a)
{
  ros::CallbackInterfacePtr cb(a, genom_activity_cbd);
  a->self->control.queue->addCallback(cb);
}


<'if {![catch {$component clock-rate} rate]} {'>
/* --- Timer callback ------------------------------------------------------ */

void *
genom_component_data::timercb(void *data)
{
  genom_component_data *self = (genom_component_data *)data;
  uint64_t ticks;
  sigset_t sset;
  int sig;

  sigemptyset(&sset);
  sigaddset(&sset, SIGALRM);

  while (1) {
    do {
      sig = 0;
      sigwait(&sset, &sig);
    } while (sig != SIGALRM);

    ticks++;
<'  foreach task [$component tasks] {'>
<'    if {[catch {$task period} period]} continue'>
<'
    set ticks [tcl::mathfunc::int [expr {[$period value]/[$rate value]}]]
    if {![catch {$task delay} delay]} {
      set d [tcl::mathfunc::int [expr {[$delay value]/[$rate value]}]]
    } else {
      set d 0
    }
'>
    if (ticks % <"$ticks"> == <"$d">) {
      pthread_mutex_lock(&self->tasks.<"[$task name]">_.lock);
      self->tasks.<"[$task name]">_.wakeup = true;
      pthread_cond_broadcast(&self->tasks.<"[$task name]">_.sync);
      pthread_mutex_unlock(&self->tasks.<"[$task name]">_.lock);
    }
<'  }'>
  }
  return NULL;
}
<'}'>


<'foreach s [$component services] {'>


<'  if {[catch {$s task} task]} {'>
/* <"[--- Control service [$s name] ----------------------------------------]"> */

bool
genom_component_data::<"[$s name]">_rqstcb(
  genom::srv_<"[$s name]">::input &in, genom::srv_<"[$s name]">::output &out)
{
  genom::srv_<"[$s name]">::locals locals;
  genom_event s;

  genom_log_debug("handling request for <"[$s name]">");

  s = <"[$s name]">_controlcb(locals, in, out);

  /* wake up sleeping activities */
<'    foreach t [$component tasks] {'>
  pthread_mutex_lock(&tasks.<"[$t name]">_.lock);
  if (!tasks.<"[$t name]">_.runnable) {
    tasks.<"[$t name]">_.runnable = 1;
    pthread_cond_broadcast(&tasks.<"[$t name]">_.sync);
  }
  pthread_mutex_unlock(&tasks.<"[$t name]">_.lock);
<'    }'>

  out.genom_success = s == genom_ok;
  if (!out.genom_success)
    genom_<"$comp">_encodex(out.genom_exdetail, s,
                   control.context.raised(NULL, &control.context));

  if (out.genom_success) {
    control.run_map[<"$COMP">_<"[$s name]">_RQSTID] = true;
    <"[$s name]">_interrupt_other(-1);
  }

  genom_log_debug("done service %s", "<"[$s name]">");
  return true;
}
<'  } else {'>
/* <"[--- Execution service [$s name] --------------------------------------]"> */

void
genom_component_data::<"[$s name]">_rqstcb(
  actionlib::ServerGoalHandle< genom::action_<"[$s name]"> > rqst)
{
  genom_activity_service_<"[$s name]"> *a;
  genom_event s;

  genom_log_debug("handling request for <"[$s name]">");

  a = new genom_activity_service_<"[$s name]">();
  if (!a) { rqst.setRejected(); return; }

  a->self = this;
  a->task = &tasks.<"[$task name]">_;
  a->sid = <"$COMP">_<"[$s name]">_RQSTID;
  a->rqst = rqst;
  genom::ids::pcopy(a->in, *rqst.getGoal());

  pthread_mutex_lock(&tasks.<"[$task name]">_.lock);
  s = tasks.<"[$task name]">_.activities.alloc(a);
  pthread_mutex_unlock(&tasks.<"[$task name]">_.lock);
  if (s) { rqst.setRejected(); delete a; return; }

  rqst.setAccepted();

  s = <"[$s name]">_controlcb(a->locals, a->in, a->out);
  if (s) {
    /* failure in control codels */
    assert(a->status == ACT_INIT);
    a->exdetail = control.context.raised(&a->state, &control.context);
    assert(a->state != genom_ok);
    a->out.genom_success = false;
    genom_<"$comp">_encodex(a->out.genom_exdetail, a->state, a->exdetail);

    rqst.setSucceeded(a->out);
    a->status = ACT_VOID;
  } else if (<"[$s name]">_interrupt_other(a->aid)) {
    /* will be scheduled once conflicting activities have stopped */
  } else {
    /* start activity */
    pthread_mutex_lock(&tasks.<"[$task name]">_.lock);
    a->start = 1;
    genom_state_update(&tasks.<"[$task name]">_);
    pthread_mutex_unlock(&tasks.<"[$task name]">_.lock);
  }

  /* wake up sleeping activities */
<'    foreach t [$component tasks] {'>
  pthread_mutex_lock(&tasks.<"[$t name]">_.lock);
  if (!tasks.<"[$t name]">_.runnable) {
    tasks.<"[$t name]">_.runnable = 1;
    pthread_cond_broadcast(&tasks.<"[$t name]">_.sync);
  }
  pthread_mutex_unlock(&tasks.<"[$t name]">_.lock);
<'    }'>
}

void
genom_component_data::<"[$s name]">_intercb(
  actionlib::ServerGoalHandle< genom::action_<"[$s name]"> > rqst)
{
  genom_activity *a;

  pthread_mutex_lock(&tasks.<"[$task name]">_.lock);
  a = tasks.<"[$task name]">_.activities.bygid(rqst.getGoalID());
  if (a == NULL) {
    pthread_mutex_unlock(&tasks.<"[$task name]">_.lock);
    genom_log_warn(
      "cancel request for non-existent activity %s",
      rqst.getGoalID().id.c_str());
    return;
  }

  switch(a->status) {
    case ACT_VOID: case ACT_STOP: case ACT_ETHER: break;

    case ACT_INIT: case ACT_RUN:
      a->stop = 1;
      a->interruptedby = "client";
      if (!tasks.<"[$task name]">_.runnable) {
        tasks.<"[$task name]">_.runnable = 1;
        pthread_cond_broadcast(&tasks.<"[$task name]">_.sync);
      }
      break;
  }
  pthread_mutex_unlock(&tasks.<"[$task name]">_.lock);
}

<'  }'>
<'}'>


/* --- Activity callback --------------------------------------------------- */

void
genom_component_data::activity_report(genom_activity *a)
{
  assert(a->status == ACT_ETHER);

  /* success: update after/before array */
  if (a->state == <"$comp">_ether && a != a->task->permanent)
    control.run_map[a->sid] = true;

  /* reply */
  a->report();

  /* delete self */
  pthread_mutex_lock(&a->task->lock);
  assert(a->task->activities.a[a->iid] == a);
  a->task->activities.a[a->iid] = NULL;
  if (a == a->task->permanent) a->task->permanent = NULL;
  genom_state_update(a->task);
  pthread_mutex_unlock(&a->task->lock);

<'    foreach t [$component tasks] {'>
  /* check for pending activities */
  {
    genom_exec_task &task = a->self->tasks.<"[$t name]">_;

    pthread_mutex_lock(&task.lock);
    for(size_t i = 0; i < genom_activities::MAX_ACTIVITIES; i++) {
      genom_activity *c = task.activities.a[i];
      if (!c) continue;
      if (c->status != ACT_INIT) continue;

      pthread_mutex_unlock(&task.lock);
      int delay = c->interrupt_other(a->self);
      pthread_mutex_lock(&task.lock);

      if (!delay) {
        c->start = 1;
        if (!task.runnable) {
          task.runnable = 1;
          pthread_cond_broadcast(&task.sync);
        }
      }
    }
    pthread_mutex_unlock(&task.lock);
  }
<'    }'>
  delete a;
}

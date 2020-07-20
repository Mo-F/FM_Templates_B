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

#include "<"$comp">_genom3_external_for_bip.h"


void genom_<"$comp">_fini(void);

/* <"[--- ${comp}_cntrl_task --------------------------------------------]"> */

/** Control task of component <"$comp">
 *
 * This task waits for incoming messages or internal events. Upon reception
 * of a request, the corresponding callback function is invoked. Internal
 * events are handled and final replies are sent to the client.
 */
 //int
 //<"$comp">_cntrl_task(const char *instance)


static genom_<"$comp">_component_data cids;
struct genom_<"$comp">_component_data *<"$comp">_genom_component;

struct genom_<"$comp">_component_data *
genom_<"$comp">_init(const char *instance)
{
  int s;
  
  <"$comp">_genom_component= &cids;

  cids.bipe_comp = BIPE_genom_component;
  cids.bipe_comp->control.nthread++;
  cids.control.context.raise = ros_server_raise;
  cids.control.context.raised = ros_server_raised;
  cids.control.context.data = &cids.control.context_data;

  cids.control.context_data.self = &cids;
  cids.control.context_data.ex = genom_ok;
  cids.control.context_data.exdetail = NULL;
  cids.control.context_data.exsize = 0;

  cids.control.node = new ros::NodeHandle("<"$comp">");
  //  cids.control.queue = &(cids.control.queueStr);
  cids.control.queue =  new ros::CallbackQueue();
  cids.control.node->setCallbackQueue(cids.control.queue);

  pthread_mutex_init(&cids.control.lock, NULL);
  pthread_mutex_init(&cids.control.bip_proxi, NULL);
  pthread_cond_init(&cids.control.serv_bip_proxi, NULL);
  pthread_cond_init(&cids.control.sync, NULL);
  cids.control.bip_proxi_idle = false;
  cids.control.rqst_rcv = false;

  memset(cids.control.run_map, 0, sizeof(cids.control.run_map));

<'foreach s [$component services] {'>
<'  if {[$s kind] ne "activity"} continue'>
  cids.services.<"[$s name]">_ = NULL;
<'}'>

   
<'if {![catch {$component ids}]} {'>
  genom::ids::pinit(cids.ids);
<'}'>

  /* advertise data outports */
  genom_<"$comp">_log_info("advertising ports");
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
  //  pthread_spin_init(&cids.tasks.<"[$t name]">_.rlock, 0);

  cids.tasks.<"[$t name]">_.num = <"${comp}_[$t name]_TASKID">;
  cids.tasks.<"[$t name]">_.spawned = false;
  cids.tasks.<"[$t name]">_.wakeup = false;
  cids.tasks.<"[$t name]">_.runnable = true;
  cids.tasks.<"[$t name]">_.shutdown = false;

  for(size_t i = 0; i < genom_<"$comp">_activities::MAX_ACTIVITIES; i++)
    cids.tasks.<"[$t name]">_.activities.a[i] = NULL;
  cids.tasks.<"[$t name]">_.permanent = new genom_<"$comp">_activity_task_<"[$t name]">;
  if (!cids.tasks.<"[$t name]">_.permanent) err(2, "<"[$t name]">");
  cids.tasks.<"[$t name]">_.permanent->self = &cids;
  cids.tasks.<"[$t name]">_.permanent->task = &cids.tasks.<"[$t name]">_;
  cids.tasks.<"[$t name]">_.permanent->sid = -1;
  if (cids.tasks.<"[$t name]">_.activities.alloc(
        cids.tasks.<"[$t name]">_.permanent) != genom_ok)
    errx(2, "<"[$t name]">: permanent activity");

  //  pthread_mutex_init(&cids.tasks.<"[$t name]">_.lock, NULL);
  // pthread_cond_init(&cids.tasks.<"[$t name]">_.sync, NULL);

<'}'>

  /* advertise services */
  genom_<"$comp">_log_info("advertising services");
<'foreach s [$component services] {'>
<'  if {[catch {$s task} t]} {'>
  cids.services.<"[$s name]">_ =
    cids.control.node->advertiseService(
      "<"[$s name]">", &genom_<"$comp">_component_data::<"[$s name]">_rqstcb, &cids);
<'  } else {'>
  cids.services.<"[$s name]">_ =
    new actionlib::ActionServer< genom::action_<"[$s name]"> >(
      *cids.control.node, "<"[$s name]">",
      boost::bind(&genom_<"$comp">_component_data::<"[$s name]">_rqstcb, &cids, _1),
      boost::bind(&genom_<"$comp">_component_data::<"[$s name]">_intercb, &cids, _1),
      false);
  cids.services.<"[$s name]">_->start();
<'  }'>
<'}'>

  /* disable actionlib status updates when goal status does not change */
    //  cids.control.node->setParam("actionlib_status_frequency", 0); done by the BIPE comp.

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

  cids.spawn_control_task();	// This will start the ROS thread handling the queue.

  return &cids;
}

int
BIP_<"$comp">_init_mbox(void)
{
  return 1;			// This does not make sense in ROS, but we  define it to minimize the change in the BIP model.
}


void
BIP_<"$comp">_cntrl_task_check_event(void *data) /* This is the function which should be
						called by the event port */
{
  genom_<"$comp">_component_data *self = <"$comp">_genom_component;

  self->control.bip_event_port = data;	/* data is a pointer on the BIP port. No need to do it each time, but simpler to code... */

  genom_<"$comp">_log_info("BIP_<"$comp">_cntrl_task_check_event called and waiting for rqst");

  pthread_mutex_lock(&self->control.bip_proxi);

  while (!self->control.rqst_rcv)
    pthread_cond_wait(&self->control.serv_bip_proxi, &self->control.bip_proxi);

  self->control.rqst_rcv = false;

  pthread_mutex_unlock(&self->control.bip_proxi);
 
  genom_<"$comp">_log_info("BIP_<"$comp">_cntrl_task_check_event thread got control after request received...");

  call_<"$comp">BIPExternalPort_push(self->control.bip_event_port, self->control.current_rqst_type);

  return;
}

void
genom_<"$comp">_fini(void)
{

  genom_<"$comp">_log_info("shutting down services");
<'foreach s [$component services] {'>
<'  if {[catch {$s task} t]} {'>
  cids.services.<"[$s name]">_.shutdown();
<'  } else {'>
  if (cids.services.<"[$s name]">_)
    delete cids.services.<"[$s name]">_;
<'  }'>
<'}'>

  genom_<"$comp">_log_info("shutting down ports");
<'foreach p [$component ports] {'>
  cids.ports.<"[$p name]">_.fini();
<'}'>

  genom_<"$comp">_log_info("shutting down control task");
<'if {![catch {$component ids}]} {'>
  genom::ids::pfini(cids.ids);
<'}'>

  genom_<"$comp">_log_info("shutdown complete");
  return;
}


void *start_<"$comp">_control_task(void *v)
{
  genom_<"$comp">_component_data *cids= (genom_<"$comp">_component_data *)v;

  genom_<"$comp">_log_info("control task (ROS spin) spwaned and running");
  while(ros::ok() && !BIPE_genom_shutdown) {
    cids->control.queue->callAvailable(ros::WallDuration(1.));
    //    cids->genom_state_refresh();
  }
  genom_<"$comp">_log_info("control task (ROS spin) asked to shutdown... ");
  genom_<"$comp">_fini();

  if (!--(cids->bipe_comp->control.nthread))
    genom_BIPE_fini();
  else
    pthread_exit(NULL);
}


void
genom_<"$comp">_component_data::spawn_control_task(void)
{
  int s;
  s = pthread_create(&control.control_task_t, NULL, start_<"$comp">_control_task, this);
  if (s) err(2, "spawning ROS/Control task");
}

/* <"[--- ${comp}_cntrl_task_signal -------------------------------------]"> */

/** \brief Signal the control task that an activity is terminated
 */

static const struct genom_<"$comp">_activity_cbd_ {
  void operator()(genom_<"$comp">_activity *) {}
} genom_<"$comp">_activity_cbd = {};

void
BIPE_cntrl_task_signal(genom_<"$comp">_activity *a)
{
  ros::CallbackInterfacePtr cb(a, genom_<"$comp">_activity_cbd);
  a->self->control.queue->addCallback(cb);
}

<'foreach s [$component services] {'>


<'  if {[catch {$s task} task]} {'>
/* <"[--- Control service [$s name] ----------------------------------------]"> */

bool
genom_<"$comp">_component_data::<"[$s name]">_rqstcb(
  genom::srv_<"[$s name]">::input &in, genom::srv_<"[$s name]">::output &out)
{
  genom::srv_<"[$s name]">::locals locals;
  genom_event s;
  genom_<"$comp">_component_data *self = this;
   
  genom_<"$comp">_log_info("CT request <"[$s name]"> arrived");
  pthread_mutex_lock(&self->control.bip_proxi);
  self->control.rqst_rcv = true;
  self->control.current_rqst_type = <"$COMP">_<"[$s name]">_RQSTID; // this will tell which RQID we are currently handing (can only have one).
  self->services.<"[$s name]">_locals = locals; // saving locals 
  self->services.<"[$s name]">_in = in;		// saving in

  genom_<"$comp">_log_info("CT broadcasting to wake up BIP_<"$comp">_cntrl_task_check_event");
  pthread_cond_broadcast(&self->control.serv_bip_proxi);
  pthread_mutex_unlock(&self->control.bip_proxi);


  genom_<"$comp">_log_info("CT waiting for BIP to finish");
  pthread_mutex_lock(&self->control.bip_proxi);
  while (!self->control.bip_proxi_idle) // wait BIP_PROXI is done...
    pthread_cond_wait(&self->control.serv_bip_proxi, &self->control.bip_proxi);
  
  self->control.bip_proxi_idle = false;
  pthread_mutex_unlock(&self->control.bip_proxi);
  genom_<"$comp">_log_info("CT BIP has finished");
 
  genom_<"$comp">_log_info("Handling request for <"[$s name]">");

  s =  self->control.current_rqst_return;

  out = self->services.<"[$s name]">_out; // get the out from the rqst
  locals = self->services.<"[$s name]">_locals; // getting locals back, although I do not see the point for ctrl rqst

  out.genom_success = s == genom_ok;
  if (!out.genom_success)
    genom_<"$comp">_encodex(out.genom_exdetail, s,
                   control.context.raised(NULL, &control.context));

  if (out.genom_success) {
    control.run_map[<"$COMP">_<"[$s name]">_RQSTID] = true;
    <"[$s name]">_interrupt_other(-1);
  }

  genom_<"$comp">_log_info("Done with service %s", "<"[$s name]">");
  return true;
}
<'  } else {'>
/* <"[--- Execution service [$s name] --------------------------------------]"> */

void
genom_<"$comp">_component_data::<"[$s name]">_rqstcb(
  actionlib::ServerGoalHandle< genom::action_<"[$s name]"> > rqst)
{
  genom_<"$comp">_activity_service_<"[$s name]"> *a;
  genom_event s;
  genom_<"$comp">_component_data *self = this;

  genom_<"$comp">_log_info("CT request <"[$s name]"> arrived");
  pthread_mutex_lock(&self->control.bip_proxi);
  self->control.rqst_rcv = true;
  self->control.current_rqst_type = <"$COMP">_<"[$s name]">_RQSTID; // this will tell which RQID we are currently handing (can only have one).

  a = new genom_<"$comp">_activity_service_<"[$s name]">();
  if (!a) { rqst.setRejected(); return; }

  a->self = this;
  a->task = &tasks.<"[$task name]">_;
  a->sid = <"$COMP">_<"[$s name]">_RQSTID;
  a->rqst = rqst;
  genom::ids::pcopy(a->in, *rqst.getGoal());

  s = tasks.<"[$task name]">_.activities.alloc(a);
  if (s) {
    rqst.setRejected();
    delete a;
    return;
  }

  rqst.setAccepted();

  self->control.current_activity = a;

  genom_<"$comp">_log_info("CT broadcasting to wake up BIP_<"$comp">_cntrl_task_check_event");
  pthread_cond_broadcast(&self->control.serv_bip_proxi);
  pthread_mutex_unlock(&self->control.bip_proxi);


  genom_<"$comp">_log_info("CT waiting for BIP to finish");
  pthread_mutex_lock(&self->control.bip_proxi);
  while (!self->control.bip_proxi_idle) // wait BIP_PROXI is done...
    pthread_cond_wait(&self->control.serv_bip_proxi, &self->control.bip_proxi);

  // If we get here, it means that a rqst report (a failure) or the IR woke us up...
  self->control.bip_proxi_idle = false;
  pthread_mutex_unlock(&self->control.bip_proxi);
  genom_<"$comp">_log_info("CT BIP has finished");
 
  genom_<"$comp">_log_info("Handling request for <"[$s name]">");
  
  //  s = <"[$s name]">_controlcb(a->locals, a->in, a->out);
  s =  self->control.current_rqst_return;

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
    //    pthread_mutex_lock(&tasks.<"[$task name]">_.lock);
    a->start = 1;
    genom_state_update(&tasks.<"[$task name]">_);
    //    pthread_mutex_unlock(&tasks.<"[$task name]">_.lock);
  }
}

void
genom_<"$comp">_component_data::<"[$s name]">_intercb(
  actionlib::ServerGoalHandle< genom::action_<"[$s name]"> > rqst)
{
  // I leave this code here, but I do not see it being used within the BIPE
  genom_<"$comp">_activity *a;

  //  pthread_mutex_lock(&tasks.<"[$task name]">_.lock);
  a = tasks.<"[$task name]">_.activities.bygid(rqst.getGoalID());
  if (a == NULL) {
    //    pthread_mutex_unlock(&tasks.<"[$task name]">_.lock);
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
	//        pthread_cond_broadcast(&tasks.<"[$task name]">_.sync);
      }
      break;
  }
  //  pthread_mutex_unlock(&tasks.<"[$task name]">_.lock);
}

<'  }'>
<'}'>


/* --- Activity callback --------------------------------------------------- */

void
genom_<"$comp">_component_data::activity_report(genom_<"$comp">_activity *a)
{
  // assert(a->status == ACT_ETHER);

  // /* success: update after/before array */
  // if (a->state == <"$comp">_ether && a != a->task->permanent)
  //   control.run_map[a->sid] = true;

  /* reply */
  a->report();

  assert(a->task->activities.a[a->iid] == a);
  a->task->activities.a[a->iid] = NULL;
  if (a == a->task->permanent) a->task->permanent = NULL;
  genom_state_update(a->task);

  delete a;
}

/* BIP bool functions to test if an activity is of a specific RQSTID.  */
<'foreach s [$component services] {'>
bool BIP_<"$COMP">_<"[$s name]">_RQSTID_p(const int a)
{
  return (a == <"$COMP">_<"[$s name]">_RQSTID);
}

<'}'>

<'foreach e [dotgen types] {'>
<'  if {([$e kind] == "exception")} {'>
// bool BIP_<"[$e cname]">_p(const genom_event e)
// {
//   return (<"[$e cname]">_id == e);
// }

<'}'>
<'  if {([$e kind] == "event") || ([$e kind] == "pause event")} {'>
// bool BIP_<"[$e cname]">_p(const genom_event e)
// {
//   return (<"[$e cname]"> == e);
// }

<'}'>
<'}'>


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
#                                      Anthony Mallet on Wed Feb 29 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]

lang c
'>
#include "autoconf/acheader.h"

#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "commonStructLib.h"
#include "h2evnLib.h"
#include "taskLib.h"

#include "<"$comp">_parameters.h"
#include "<"$comp">_msglib.h"
#include "<"$comp">_activity.h"
#include "<"$comp">_control_task.h"
#include "typecopy.h"


/* --- local data ---------------------------------------------------------- */

#define CNTRL_TASK_MIN_STACK_SIZE 4096
#define EXEC_TASK_MIN_STACK_SIZE  4096

/* cslib server with context pointer, for reentrant csLib callbacks */
struct CS_SERV_IDS {
  CS_SERV csserv;
  struct genom_component_data *self;
};

/* These are BIPE calls. The rqstcb just parses the argument in the BIP version, and sets
   the activity in the pointer parameter. The validate and call to the "codel" is done in another function.*/
<'foreach s [$component services] {'>
static void	<"$comp">_<"[$s name]">_rqstcb(SERV_ID csserv, int sid);
<'}'>


/* <"[--- genom_${comp}_init --------------------------------------------]"> */

void *
genom_<"$comp">_init(void)
{
  struct genom_component_data *self;
  /* char tname[64]; */

  /* create internal data structure */
  self = malloc(sizeof(*self));
  if (!self) {
    genom_log_warn(1, "cannot create internal data structure");
    return NULL;
  }

<'if {![catch {$component ids} ids]} {'>
  genom_tinit_<"[$ids mangle]">(&self->ids);
<'}'>
<'foreach p [$component ports] {'>
  genom_tinit_<"$comp">_<"[$p name]">_port(&self->ports.<"[$p name]">);
<'}'>
   /* No remote in the BIP version for now. */
<'foreach r [$component remotes] {'>
   // genom_tinit_<"$comp">_<"[$r name]">_remote(&self->remotes.<"[$r name]">);
<'}'>
<'foreach t [$component tasks] { '>

  self->tasks.<"[$t name]">.context.raise = genom_pocolibs_raise;
  self->tasks.<"[$t name]">.context.raised = genom_pocolibs_raised;
  self->tasks.<"[$t name]">.context.data =
    &self->tasks.<"[$t name]">.context_data;

  self->tasks.<"[$t name]">.context_data.self = self;
  self->tasks.<"[$t name]">.context_data.ex = genom_ok;
  self->tasks.<"[$t name]">.context_data.exdetail = NULL;
  self->tasks.<"[$t name]">.context_data.exsize = 0;

  {
    int i;
    for(i = 0; i < genom_max_activities(); i++) /* same for operations in this loop */
      self->tasks.<"[$t name]">.activities.a[i].any.status = ACT_VOID;
    i = genom_<"$comp">_activity_alloc(&self->tasks.<"[$t name]">.activities);
    assert(i == 0);
    self->tasks.<"[$t name]">.permanent =
      &self->tasks.<"[$t name]">.activities.a[i].any;
  }
  self->tasks.<"[$t name]">.permanent->sid = -1;
  self->tasks.<"[$t name]">.permanent->rid = -1;
  self->tasks.<"[$t name]">.permanent->status = ACT_ACT;
  self->tasks.<"[$t name]">.permanent->start = 1;

<'}'>

  self->control.context.raise = genom_pocolibs_raise;
  self->control.context.raised = genom_pocolibs_raised;

  self->control.context.data = &self->control.context_data;
  self->control.context_data.self = self;
  self->control.context_data.ex = genom_ok;
  self->control.context_data.exdetail = NULL;
  self->control.context_data.exsize = 0;

  self->control.bip_event_port = NULL;	/* This is a pointer on the BIP event port. */

  snprintf(self->control.mbox_name, sizeof(self->control.mbox_name),
           "%s", genom_instance);
  self->control.csserv = NULL;

  int i;
  for(i = 0; i < genom_max_activities(); i++) /* same for operations in this loop */
    self->control.activities.a[i].any.status = ACT_VOID;

  memset(self->control.run_map, 0, sizeof(self->control.run_map));


  /* create posters */
  if (genom_metadata_<"$comp">_init(self)) goto error;
  if (genom_state_<"$comp">_init(self)) goto error;
<'foreach p [$component ports simple] {'>
  if (genom_<"$comp">_<"[$p name]">_open(&self->control.context)) goto error;
<'}'>
  if (genom_<"$comp">_genom_metadata_write(&self->control.context)) goto error;
  if (genom_<"$comp">_genom_state_write(&self->control.context)) goto error;

  genom_log_info("setup and running");

  <"$comp">_genom_component = self;

  return self;

error:
  genom_<"$comp">_fini(self);
  return NULL;
}


/* <"[--- genom_${comp}_fini --------------------------------------------]"> */

void
genom_<"$comp">_fini(void *data) /* this will be modeled in bip... */
{
  struct genom_component_data *self = data;

  /* clean up */
<'foreach p [$component ports] {'>
  genom_<"$comp">_<"[$p name]">_delete(&self->control.context);
<'}'>

<'foreach r [$component remotes] {'>
  genom_tfini_<"$comp">_<"[$r name]">_remote(&self->remotes.<"[$r name]">);
<'}'>
<'foreach p [$component ports] {'>
  genom_tfini_<"$comp">_<"[$p name]">_port(&self->ports.<"[$p name]">);
<'}'>
<'if {![catch {$component ids} ids]} {'>
  genom_tfini_<"[$ids mangle]">(&self->ids);
<'}'>
  free(self);
  genom_log_info("shutdown complete");
}


/* <"[--- ${comp}_cntrl_task(void) --------------------------------------]"> */

void
<"$comp">_cntrl_task_init(void *data)
{
  struct genom_component_data *self = data;
  SERV_ID serv_ids;
  char tname[64];

  /* make the thread a pocolibs task. This is required to use mailboxes from a thread. */
  snprintf(tname, sizeof(tname), "%s-bep", genom_instance); /* bep for BIP Event Port */
  genom_log_info("Calling taskFromThread from <"$comp">_cntrl_task_init (Event Port thread).");
  taskFromThread(tname);
  
  /* create requests reception mailbox */
  if (csMboxInit(self->control.mbox_name, <"$COMP">_MBOX_RQST_SIZE, 0) != OK) {
    genom_log_warn(1, "cannot create server mailbox");
    return;
  }

  /* configure as a server */
  if (csServInitN(
	<"$COMP">_MAX_RQST_SIZE,
	<"$COMP">_MAX_REPLY_SIZE >= <"$COMP">_MAX_INTERMED_REPLY_SIZE ?
	<"$COMP">_MAX_REPLY_SIZE : <"$COMP">_MAX_INTERMED_REPLY_SIZE,
	<"$COMP">_NRQSTID, &serv_ids) != OK) {
    genom_log_warn(1, "cannot initialize server mailbox");
    goto error;
  }
  /* fiddle with returned CS_SERV structure, to store ids for callbacks */
  serv_ids =
    realloc(serv_ids, sizeof(struct CS_SERV_IDS));
  ((struct CS_SERV_IDS *)serv_ids)->self = self;

  /* install services callbacks */
<'foreach s [$component services] {'>
  if (csServFuncInstall(serv_ids, <"$COMP">_<"[$s name]">_RQSTID,
                        (FUNCPTR)<"$comp">_<"[$s name]">_rqstcb) != OK) {
    genom_log_warn(1, "cannot serve service <"[$s name]">");
    goto error;
  }
<'}'>

  self->control.csserv = serv_ids; /* This is to make sure that all the initialization are done  */

  genom_log_info("inited control task");
  /* For BIP, at this point, the control task is ready to be run. */
  return;

  
error:
  if (self->control.csserv) csServEnd(self->control.csserv);
  csMboxEnd();

} 

void *
BIP_<"$comp">_cntrl_task_check_event(void *data) /* This is the function which should be
						called by the event port, and it takes the
						port as argument */
{
  struct genom_component_data *self = <"$comp">_genom_component;
  int e;

  if (! self) return NULL;	/* if the component has not been created yet, we just return. */
  if (! self->control.csserv) return NULL; /* the CS server has not been created yet. */

  genom_log_debug("Checking MBOX status from BIP.");

  self->control.bip_event_port = data;	/* data is a pointer on the BIP port. No need to do it each time, but simpler to code... */

  /* read reception mailbox status and sleep if there is no message */
  e = csMboxStatus(RCV_MBOX);
  genom_log_debug("csMboxStatus returned %d.", e);
  switch(e) {
  case 0: /* no message */
    /* update state port before sleeping */
    genom_log_debug("calling genom_state_<"$comp">_refresh.");
    genom_state_<"$comp">_refresh(self);

    genom_log_debug("calling h2evnSusp.");
    e = h2evnSusp(0);
    genom_log_debug("h2evnSusp returned %d.", e);
    if (e != TRUE) {
      genom_log_warn(1, "someone did something nasty");
      genom_log_warn(0, "aborting");
      abort();
    }
    genom_log_debug("control task wake up");

    /* update reception mailbox status */
    e = csMboxStatus(RCV_MBOX);
    if (e != RCV_MBOX) /* awoken by internal event */ break;

    /*FALLTHROUGH*/
  case RCV_MBOX: /* incoming request */
    genom_log_debug("csMboxStatus incoming request.");
    if (csServRqstExec(self->control.csserv) != OK) { /* This is how the requests are being parsed... and pushed in te BIP event FIFO. */
      genom_log_warn(1, "cannot read reception mailbox");
      genom_log_warn(0, "aborting");
      abort();
    }
    break;

    /* Not reachable in this function. */
  /* case ERROR: */
  /*   genom_log_warn(1, "cannot check reception mailbox"); */
  /*   genom_log_warn(0, "aborting"); */
  /*   abort(); */
  }
 
  return NULL;
}


/* --- Service callbacks --------------------------------------------------- */

<'foreach s [$component services] {'>
static void
<"$comp">_<"[$s name]">_rqstcb(SERV_ID csserv, int rid)
{
  struct genom_component_data *self = ((struct CS_SERV_IDS *)csserv)->self;
  struct genom_<"$comp">_<"[$s name]">_activity *a;
  STATUS s;

  genom_log_debug("handling request for <"[$s name]">");

  /* get an activity slot */
<'  if {[catch {$s task} t]} {'>
  int id = genom_<"$comp">_activity_alloc(&self->control.activities);/* We do need it FFI */
  if (id < 0) { /* same */
    s = csServReplySend(
      csserv, rid, FINAL_REPLY, ERROR, NULL, 0,
      genom_<"$comp">_genom_too_many_activities_encodex);
    if (s == ERROR) assert(!"unexpected failure");
    return;
  }
  a = &self->control.activities.a[id].s_<"[$s name]">;
<'  } else {'>
  int id = genom_<"$comp">_activity_alloc(&self->tasks.<"[$t name]">.activities);/* We do need it FFI */
  if (id < 0) { /* same */
    s = csServReplySend(
      csserv, rid, FINAL_REPLY, ERROR, NULL, 0,
      genom_<"$comp">_genom_too_many_activities_encodex);
    if (s == ERROR) assert(!"unexpected failure");
    return;
  }
  a = &self->tasks.<"[$t name]">.activities.a[id].s_<"[$s name]">;
<'  }'>
  a->h.sid = <"$COMP">_<"[$s name]">_RQSTID;
  a->h.rid = rid;
  genom_tinit_<"$comp">_<"[$s name]">_activity(a);

  /* decode input */
  s = csServRqstParamsGet(csserv, rid, (char *)&a->in, 0,
                          genom_<"$comp">_<"[$s name]">_decode);
  if (s == ERROR) {
    genom_log_warn(0, "invalid input for service %s", "<"[$s name]">");
    a->h.state = genom_serialization_id;
    a->h.exdetail = NULL;

    BIP_<"$comp">_<"[$s name]">_activity_report(a); /* BIP will not even see it here */


    assert(a->h.status == ACT_ACT); /* not needed.. */
    a->h.status = ACT_VOID;/* same */
  } else {
  /* This push the request in the FIFO of the event port. */
    call_<"$comp">BIPExternalPort_push(self->control.bip_event_port, &(a->h)); 
  }

  return;
}
<'}'>


/* This function will be used to send IR for accepted activity requests. */
void BIP_<"$comp">_send_ir(struct genom_activity *a)
{
  struct genom_component_data *self = <"$comp">_genom_component;
  STATUS s;
  SERV_ID csserv = self->control.csserv;
  
  genom_log_info("Sending IR for rid: %d, aid: %d.", a->rid, a->aid);

  /* send intermediate reply */
  s = csServReplySend(csserv, a->rid, INTERMED_REPLY, OK,
                      (char *)&a->aid, sizeof(int), NULL);
  if (s == ERROR) {
    genom_log_warn(1, "cannot acknowledge service");
  }
  
}

// validate and control codels
<'foreach s [$component services] {'>
genom_event  BIP_<"$comp">_<"[$s name]">_validate(genom_<"$comp">_<"[$s name]">_activity_ptr a)
{
  struct genom_component_data *self = <"$comp">_genom_component;
  genom_event e;

  genom_log_info("Calling <"$comp">_<"[$s name]"> validate codel.");

  a->h.state = e = genom_<"$comp">_<"[$s name]">_validatecb(self, a);

  genom_log_info("Exiting <"$comp">_<"[$s name]"> validate codel with %s.", (!e?"genom_ok":e));

  if (a->h.state) {		/* non nominal return, we fill the exception details, but we do not report the pb. BIP will... */
    a->h.exdetail = (void *)self->control.context.raised(
      NULL, &self->control.context);
    return e;
    /* genom_<"$comp">_activity_report(self, &a->h); */
    /* goto clean; */
  }

<'  if {![catch {$s task} t]} {'>
  /* update state port */
  genom_state_<"$comp">_update(
              self, &self->tasks.<"[$t name]">.activities,
              <"${comp}_[$t name]_TASKID">);
<'  }'>
  return e;
}

genom_event  
BIP_<"$comp">_<"[$s name]">_control(genom_<"$comp">_<"[$s name]">_activity_ptr a)
{
  struct genom_component_data *self = <"$comp">_genom_component;
  genom_event e;

  genom_log_info("Calling <"$comp">_<"[$s name]"> control codel.");

  a->h.state = e = genom_<"$comp">_<"[$s name]">_controlcb(self, a);

  /* check control codels */
  if (a->h.state) {
    a->h.exdetail = (void *)self->control.context.raised(
      NULL, &self->control.context);
    /* genom_<"$comp">_activity_report(self, &a->h); */ /*should be done by BIP. */
  } else {
    /* update after/before array */
    self->control.run_map[<"$COMP">_<"[$s name]">_RQSTID] = 1;
    
    a->h.state = <"$comp">_ether;
  }

  genom_log_info("Exiting <"$comp">_<"[$s name]"> control codel with %s.", (!e?"genom_ok":e));

  return e;
}

void
BIP_<"$comp">_<"[$s name]">_activity_report(genom_<"$comp">_<"[$s name]">_activity_ptr a)
{
  /* We need to decide if BIP relies on the a->h.state or its own variable (from the model). */
  struct genom_component_data *self = <"$comp">_genom_component;
  STATUS s;

  genom_log_info("Calling <"$comp">_<"[$s name]">_activity_report.");


  /* send final reply */
  if (a->h.state == <"$comp">_ether) {
    /* success: update after/before array */
    self->control.run_map[<"$COMP">_<"[$s name]">_RQSTID] = 1;

    s = csServReplySend(
      self->control.csserv, a->h.rid, FINAL_REPLY, OK,
      (char *)&a->out, 0, genom_<"$comp">_<"[$s name]">_encode);
  } else
    s = csServReplySend(
      self->control.csserv, a->h.rid, FINAL_REPLY, ERROR, (char *)&a->h, 0,
      genom_<"$comp">_activity_encodex);
  if (s == ERROR) {
    genom_log_warn(1, "could not send output for service %s", "<"[$s name]">");
    s = csServReplySend(
      self->control.csserv, a->h.rid, FINAL_REPLY, ERROR, NULL, 0,
      genom_<"$comp">_genom_serialization_encodex);
    if (s == ERROR) {
      genom_log_warn(1, "discarding output for service %s", "<"[$s name]">");
    } else
      genom_log_warn(0, "invalid output for service %s", "<"[$s name]">");
  }

  a->h.status = ACT_VOID; /* we need it to know that the slot is available for a new activity */

  /* cleanup */
  genom_tfini_<"$comp">_<"[$s name]">_activity(a); /* I am not sure we do this in BIP */
}

<'}'>

/* BIP bool functions to use as provided in BIP guard. */

/* BIP bool functions to test if an activity is of a specific RQSTID.  */
<'foreach s [$component services] {'>
int c_BIP_<"$COMP">_<"[$s name]">_RQSTID_p(const genom_activity_ptr a)
{
  return (a->sid == <"$COMP">_<"[$s name]">_RQSTID);
}

<'}'>


/* BIP  functions to cast an activity is of a specific activity.  */

<'foreach s [$component services] {'>
genom_<"$comp">_<"[$s name]">_activity_ptr BIP_cast_activity_in_<"$comp">_<"[$s name]">_activity(genom_activity_ptr a)
{
  return (genom_<"$comp">_<"[$s name]">_activity_ptr)a;
}

<'}'>

/* BIP  functions to cast a specific activity in a genom_activity.  */
<'foreach s [$component services] {'>
genom_activity_ptr BIP_cast_<"$comp">_<"[$s name]">_activity_inactivity(genom_<"$comp">_<"[$s name]">_activity_ptr a)
{
  return &(a->h);
}

<'}'>


/* BIP bool functions to test if a particular genom_event is of a specific type. */
int c_BIP_genom_ok_p(const genom_event e)
{
  return (e == genom_ok);
}

<'foreach e [dotgen types] {'>
<'  if {([$e kind] == "event") || ([$e kind] == "pause event")} {'>
int c_BIP_<"[$e cname]">_p(const genom_event e)
{
  return (e == <"[$e cname]">);
}

<'} elseif {([$e kind] == "exception")} {'>
int c_BIP_<"[$e cname]">_p(const genom_event e)
{
  return (e == <"[$e cname]">_id);
}

<'}'>
<'}'>


/* eof */

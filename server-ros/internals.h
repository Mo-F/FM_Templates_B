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
set COMP [string toupper [$component name]]

lang c
'>
#ifndef H_GROS_<"$COMP">_INTERNALS
#define H_GROS_<"$COMP">_INTERNALS

#include <pthread.h>

#include "boost/shared_ptr.hpp"

#include "ros/ros.h"
#include "actionlib/server/action_server.h"

/* service id */
<'set i 0; foreach s [$component services] {'>
#define <"$COMP">_<"[$s name]">_RQSTID	(<"$i">)
<' incr i '>
<'}'>
#define <"$COMP">_NRQSTID	(<"$i">)

/* task id */
<'set i 0; foreach t [$component tasks] {'>
#define <"$comp">_<"[$t name]">_TASKID	(<"$i">)
<' incr i '>
<'}'>
#define <"$comp">_NTASKID	(<"$i">)

#include "<"$comp">_port.h"
#include "<"$comp">_remote.h"
#include "<"$comp">_service.h"
#include "<"$comp">_action.h"
#include "<"$comp">_locals.h"
#include "<"$comp">_activities.h"


/* --- context ------------------------------------------------------------- */

struct genom_component_data;

struct genom_context_data {
  genom_component_data *self;

  genom_event ex;
  void *exdetail;
  size_t exsize;
};


/* --- tasks --------------------------------------------------------------- */

struct genom_exec_task {
  genom_context_iface context;
  genom_context_data context_data;
  pthread_t id;
  bool spawned;
  uint8_t num;

  genom_state_rusage rusage;
  pthread_spinlock_t rlock;

  bool wakeup; /* periodic tasks */
  bool runnable, shutdown;
  genom_activity *permanent;
  genom_activities activities;
  pthread_mutex_t lock;
  pthread_cond_t sync;

  void	spawn(genom_component_data *self,
              const char *name, void *(*start)(void *));
};

int	<"$comp">_cntrl_task(const char *instance);
<'foreach t [$component tasks] {'>
void *	<"$comp">_<"[$t name]">_task(void *data);
<'}'>


/* --- internal state ------------------------------------------------------ */

struct genom_component_data {
<'if {![catch {$component ids} ids]} {'>
  <"[$ids declarator ids]">;
<'}'>

  struct ports {
<'foreach p [$component ports] {'>
    genom_port_<"[$p name]"> <"[$p name]">_;
<'}'>

  } ports;

  int genom_state_init(void);
  int genom_state_update(genom_exec_task *task);
  int genom_state_refresh(void);


  /* exec tasks */
  struct {
<'foreach t [$component tasks] {'>
    genom_exec_task <"[$t name]">_;
<'}'>
  } tasks;

  /* control task */
  struct {
    genom_context_iface context;
    genom_context_data context_data;

    ros::NodeHandle *node;
    ros::CallbackQueue *queue;

    bool run_map[<"$COMP">_NRQSTID];
    pthread_mutex_t lock;
    pthread_cond_t sync;
  } control;

  /* mutual exclusion for resource access */
  struct {
    int all;				/* all resources locked */
    void *control;			/* control task codel/service */
<'foreach t [$component tasks] {'>
    void *task_<"[$t name]">;		/* <"[$t name]"> codel */
<'}'>

    unsigned int q, qnext;		/* fifo (ticket lock) */
    pthread_mutex_t lock;
    pthread_cond_t sync;
  } resources;

  /* remote services */
  struct {
<'foreach r [$component remotes] {'>
    genom::<"$comp">_remote::<"[$r name]"> <"[$r name]">_;
<'}'>
  } remotes;

  /* service callbacks */
  struct {
<'foreach s [$component services] {'>
<'  if {[catch {$s task} t]} {'>
    ros::ServiceServer <"[$s name]">_;
<'  } else {'>
    actionlib::ActionServer< genom::action_<"[$s name]"> > *<"[$s name]">_;
<'  }'>
<'}'>
  } services;

<'foreach s [$component services] {'>
  static void <"[$s name]">_resource(void) {}
  genom_event <"[$s name]">_controlcb(
    genom::srv_<"[$s name]">::locals &locals,
    genom::srv_<"[$s name]">::input &in,
    genom::srv_<"[$s name]">::output &out);
  int <"[$s name]">_interrupt_other(int aid);
<'  if {[catch {$s task} t]} {'>
  bool <"[$s name]">_rqstcb(
    genom::srv_<"[$s name]">::input &in,
    genom::srv_<"[$s name]">::output &out);
<'  } else {'>
  void <"[$s name]">_rqstcb(
    actionlib::ServerGoalHandle< genom::action_<"[$s name]"> > rqst);
  void <"[$s name]">_intercb(
    actionlib::ServerGoalHandle< genom::action_<"[$s name]"> > rqst);
<'  }'>

<'}'>

  void	activity_report(genom_activity *a);

<'if {![catch {$component clock-rate} rate]} {'>
  /* timer */
  timer_t clock;
  pthread_t clockthread;
  static void *timercb(void *data);
<'}'>
};

/* resource access */
#define genom_take_resource(self, wait, set) do {                       \
    pthread_mutex_lock(&(self)->resources.lock);                        \
    unsigned int q = (self)->resources.qnext++;                         \
    while(q != self->resources.q || (wait))                             \
      pthread_cond_wait(&(self)->resources.sync,                        \
                        &(self)->resources.lock);                       \
    set;                                                                \
    (self)->resources.q++;                                              \
    pthread_mutex_unlock(&self->resources.lock);                        \
  } while(0)

#define genom_give_resource(self, set) do {                             \
    pthread_mutex_lock(&(self)->resources.lock);                        \
    set;                                                                \
    pthread_cond_broadcast(&self->resources.sync);                      \
    pthread_mutex_unlock(&self->resources.lock);                        \
  } while(0)


/* global shutdown flag */
extern int genom_shutdown;

/* exceptions */
extern "C" {
genom_event	ros_server_raise(genom_event ex, void *detail,
                        size_t size, genom_context self);
const void *	ros_server_raised(genom_event *ex,
                        genom_context self);
}

/* log functions */
void	genom_log_info(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));
void	genom_log_warn(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));
void	genom_log_debug(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));

#endif /* H_GROS_<"$COMP">_INTERNALS */

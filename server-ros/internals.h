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

#include "genom3/c/context.h"
#include "BIPEinternals.h"

#include "<"$comp">_port.h"
#include "<"$comp">_remote.h"
#include "<"$comp">_service.h"
#include "<"$comp">_action.h"
#include "<"$comp">_locals.h"
#include "<"$comp">_activities.h"


struct genom_context_data {
  genom_<"$comp">_component_data *self;

  genom_event ex;
  void *exdetail;
  size_t exsize;
};


/* --- tasks --------------------------------------------------------------- */

struct genom_<"$comp">_exec_task {
  genom_context_iface context;
  genom_context_data context_data;
  pthread_t id;
  bool spawned;
  uint8_t num;

  genom_state_rusage rusage;
  // I comment part of the structure which is probably not needed anymore.
  //  pthread_spinlock_t rlock;

  bool wakeup; /* periodic tasks */
  bool runnable, shutdown;
  genom_<"$comp">_activity *permanent;
  genom_<"$comp">_activities activities;
  //  pthread_mutex_t lock;
  //  pthread_cond_t sync;

  void	spawn(genom_<"$comp">_component_data *self,
	      const char *name, void *(*start)(void *));
};

int	<"$comp">_cntrl_task(const char *instance);
<'foreach t [$component tasks] {'>
void *	<"$comp">_<"[$t name]">_task(void *data);
<'}'>


/* --- internal state ------------------------------------------------------ */

struct genom_<"$comp">_component_data {

    genom_component_data *bipe_comp;

<'if {![catch {$component ids} ids]} {'>
  <"[$ids declarator ids]">;
<'}'>

  struct ports {
<'foreach p [$component ports] {'>
    genom_port_<"[$p name]"> <"[$p name]">_;
<'}'>

  } ports;

  int genom_state_init(void);
  int genom_state_update(genom_<"$comp">_exec_task *task);
  int genom_state_refresh(void);
  void spawn_bip_proxi(void);


  /* exec tasks */
  struct {
<'foreach t [$component tasks] {'>
    genom_<"$comp">_exec_task <"[$t name]">_;
<'}'>
  } tasks;

  /* control task */
  struct {
    genom_context_iface context;
    genom_context_data context_data;

    ros::NodeHandle *node;	/* This is a pointer on the global node */
    //    ros::CallbackQueue queueStr; /* The queue for this component. */
    ros::CallbackQueue *queue;	/* A pointer to the above. */
    
    bool rqst_rcv,bip_proxi_idle;		

    void *bip_event_port; /* This is the BIP port which get the request. */

    int current_rqst_type;	/* with ROS, only one of the these at once. */
    void *current_activity;	/* with ROS, only one handle at once. */
    genom_event current_rqst_return;	/* with ROS, only one of the these at once. */
    
    pthread_mutex_t bip_proxi;
    pthread_t bip_proxi_t;
    pthread_t control_task_t;
    pthread_mutex_t lock;
    pthread_cond_t serv_bip_proxi;
    pthread_cond_t sync;

    bool run_map[<"$COMP">_NRQSTID];
  } control;

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
    ros::ServiceServer <"[$s name]">_; /* This is where I save the args and locals for BIP to pick them up */
    genom::srv_<"[$s name]">::locals <"[$s name]">_locals;
    genom::srv_<"[$s name]">::input <"[$s name]">_in;
    genom::srv_<"[$s name]">::output <"[$s name]">_out;
<'  } else {'>
    actionlib::ActionServer< genom::action_<"[$s name]"> > *<"[$s name]">_;
<'  }'>
<'}'>
  } services;

<'foreach s [$component services] {'>
  genom_event <"[$s name]">_controlcb(
    genom::srv_<"[$s name]">::locals &locals,
    genom::srv_<"[$s name]">::input &in,
    genom::srv_<"[$s name]">::output &out);
  genom_event <"[$s name]">_validatecb(
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
   void  spawn_control_task(void);
  void	activity_report(genom_<"$comp">_activity *a);
};


/* global component */
extern struct genom_<"$comp">_component_data *<"$comp">_genom_component;

/* global shutdown flag */
//extern int <"$comp">_genom_shutdown;

/* some function declaration... */
struct genom_<"$comp">_component_data *genom_<"$comp">_init(const char *instance);


/* exceptions */
extern "C" {
genom_event	ros_server_raise(genom_event ex, void *detail,
                        size_t size, genom_context self);
const void *	ros_server_raised(genom_event *ex,
                        genom_context self);
}

/* log functions */
void	genom_<"$comp">_log_info(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));
void	genom_<"$comp">_log_warn(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));
void	genom_<"$comp">_log_debug(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));

#endif /* H_GROS_<"$COMP">_INTERNALS */

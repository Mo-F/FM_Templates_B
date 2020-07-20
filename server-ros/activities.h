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
#                                      Anthony Mallet on Wed Jan 25 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]
if {[catch { $component version } version]} { set version {}}

lang c
'>
#ifndef H_GROS_<"$COMP">_ACTIVITIES
#define H_GROS_<"$COMP">_ACTIVITIES

#include <err.h>
#include <pthread.h>

#include <string>

#include "ros/ros.h"
#include "ros/callback_queue_interface.h"
#include "actionlib/server/action_server.h"

#include "BIPEactivities.h"


/* exception encoding/decoding */
void		genom_<"$comp">_encodex(std::string &json, genom_event ex,
			const void *exdetail);
genom_event	genom_<"$comp">_decodex(const char *json, genom_context self);


/* --- activities ---------------------------------------------------------- */

struct genom_<"$comp">_component_data;
struct genom_<"$comp">_exec_task;

struct genom_<"$comp">_activity : public ros::CallbackInterface {
  genom_<"$comp">_component_data *self;	/* component data */
  genom_<"$comp">_exec_task *task;	/* the task for this activity */

  int iid;			/* index number */
  int sid;			/* service number, -1 for permanent activity */
  int aid;			/* activity number */

  genom_activity_status status;	/* current phase */
  bool start, stop, pause;	/* status change request */
  genom_event state;		/* current FSM state */

  const void *exdetail;		/* exception info, if any */
  const char *interruptedby;

  virtual bool gid(const actionlib_msgs::GoalID &gid) const = 0;

  //  virtual genom_activity_status invoke(void) = 0;
  virtual void report(void) = 0;
  virtual int interrupt_other(genom_<"$comp">_component_data *self) = 0;

  /* signalled by task at ETHER, invoked by ros global callback queue */
  CallResult call(void);
};

void	<"$comp">_cntrl_task_signal(genom_<"$comp">_activity *a);

template<int taskid /* unused, but here for symmetry with services */>
struct genom_<"$comp">_activity_perm_data : genom_<"$comp">_activity {
  bool gid(const actionlib_msgs::GoalID &gid) const { return false; }
  //  genom_activity_status invoke(void);
  void report(void) {}
  int interrupt_other(genom_<"$comp">_component_data *self) { return 0; }
};

template<class ActionSpec, class localstype>
struct genom_<"$comp">_activity_data : genom_<"$comp">_activity {
  typedef typename ActionSpec::_action_goal_type::_goal_type intype;
  typedef typename ActionSpec::_action_result_type::_result_type outype;

  actionlib::ServerGoalHandle<ActionSpec> rqst;
  intype in;
  outype out;
  localstype locals;

  bool gid(const actionlib_msgs::GoalID &gid) const {
    return rqst.getGoalID().id == gid.id;
  }

  //  genom_activity_status invoke(void);

  void report(void) {
    if (state == <"$comp">_ether) {
      out.genom_success = true;
    } else {
      out.genom_success = false;
      genom_<"$comp">_encodex(out.genom_exdetail, state, exdetail);
    }
    rqst.setSucceeded(out);
  }

  int interrupt_other(genom_<"$comp">_component_data *self);
};

<'foreach s [$component services] {'>
<'  if {[$s kind] == "activity"} {'>
typedef genom_<"$comp">_activity_data<
  genom::action_<"[$s name]">,
  genom::srv_<"[$s name]">::locals> genom_<"$comp">_activity_service_<"[$s name]">;
<'  }'>
<'}'>

<'foreach t [$component tasks] {'>
typedef genom_<"$comp">_activity_perm_data<<"${comp}_[$t name]_TASKID">>
  genom_<"$comp">_activity_task_<"[$t name]">;
<'}'>


struct genom_<"$comp">_activities {
  static const size_t MAX_ACTIVITIES = 32;
  genom_<"$comp">_activity *a[MAX_ACTIVITIES];

  genom_event alloc(genom_<"$comp">_activity *i);
  genom_<"$comp">_activity *bygid(const actionlib_msgs::GoalID &gid);
};

#endif /* H_GROS_<"$COMP">_ACTIVITIES */

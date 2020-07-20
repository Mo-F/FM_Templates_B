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
#                                      Anthony Mallet on Wed Feb 29 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]

lang c
'>
#ifndef H_<"$COMP">_CONTROL_TASK
#define H_<"$COMP">_CONTROL_TASK

#include <assert.h>
#include <pthread.h>
#include <stdarg.h>

#include "csLib.h"
#include "semLib.h"

#include "<"$comp">_c_types.h"
#include "<"$comp">_parameters.h"
#include "<"$comp">_msglib.h"
#include "<"$comp">_portlib.h"
#include "<"$comp">_remotelib.h"
#include "<"$comp">_activity.h"

/* This include is here just to enforce consistency... */
#include "<"$comp">_genom3_external_for_bip.h"



/* Had to make global here also... */
/* I know, this is bad to make it a global, but I have no other way to
   link genom component and BIP port for now. Note that this is only a problem if we have
   more than one instance of the same module... unlikely for now.*/
extern struct genom_component_data *<"$comp">_genom_component;

/* --- internal state ------------------------------------------------------ */

extern const char *genom_instance;

struct genom_component_data;

struct genom_context_data {
  struct genom_component_data *self;

  genom_event ex;
  void *exdetail;
  size_t exsize;
};

struct genom_component_data {
<'if {![catch {$component ids} ids]} {'>
  <"[$ids declarator ids]">;
<'}'>

  struct {
<'foreach p [$component ports] {'>
    struct genom_<"$comp">_<"[$p name]">_port <"[$p name]">;
<'}'>
  } ports;

  struct {
<'foreach t [$component tasks] {'>
    struct {
      struct genom_context_iface context;
      struct genom_context_data context_data;
      long taskid;

      struct genom_activity *permanent; /* I need to make this a global for BIP to grab? or I can get it from here... */
      struct genom_activities activities;
    } <"[$t name]">;
<'}'>
  } tasks;

  /* control task */
  struct {
    struct genom_context_iface context;
    struct genom_context_data context_data;
    long taskid;

    char mbox_name[64];
    SERV_ID csserv;
    void *bip_event_port;			/* This is the BIP port which get the request. */

    struct genom_activities activities; /* Now we use a table */
    //    union genom_any_activity activity;
    int run_map[<"$COMP">_NRQSTID]; /* before/after constraints */
  } control;

};


/* control task */
void *	genom_<"$comp">_init(void);
void	genom_<"$comp">_fini(void *data);
/* void	genom_<"$comp">_schedule_cntrl(struct genom_component_data *self); */

void <"$comp">_cntrl_task_init(void *); /* This is a BIPE call */

<'foreach s [$component services] {'>
genom_event genom_<"$comp">_<"[$s name]">_controlcb(
   struct genom_component_data *,
   struct genom_<"$comp">_<"[$s name]">_activity *);
genom_event genom_<"$comp">_<"[$s name]">_validatecb(
   struct genom_component_data *,
   struct genom_<"$comp">_<"[$s name]">_activity *);

genom_event <"$comp">_<"[$s name]">_call_validate(SERV_ID csserv, struct genom_<"$comp">_<"[$s name]">_activity *a);
genom_event <"$comp">_<"[$s name]">_call_control(SERV_ID csserv, struct genom_<"$comp">_<"[$s name]">_activity *a);

<'}'>


/* exec tasks */
<'foreach t [$component tasks] {'>
void *		genom_<"$comp">_<"[$t name]">_exec_task(void *data);
void		genom_<"$comp">_<"[$t name]">_invoke_perm(
			struct genom_component_data *);
enum genom_activity_status
		genom_<"$comp">_<"[$t name]">_invoke(
			struct genom_component_data *,
			struct genom_activity *);

/* <"[--- [$t class] [$t name] --------------------------------------]"> */
<'  foreach state [$t fsm] {'>
<'    set codel [$t fsm $state]'>
<'    set func [$t class]_[$t name]_[$state name]'>

genom_event <"$comp">_codel_<"$func">(struct genom_component_data *, struct genom_activity *);
<'}'>

<'foreach obj [$t services] {'>
/* <"[--- [$obj class] [$obj name] --------------------------------------]"> */
<'  foreach state [$obj fsm] {'>
<'    set codel [$obj fsm $state]'>
<'    set func [$obj class]_[$obj name]_[$state name]'>

genom_event <"$comp">_codel_<"$func">(struct genom_component_data *, struct genom_<"$comp">_<"[$obj name]">_activity *);
<'}'>
<'}'>
<'}'>


STATUS <"$comp">_send_ir(struct genom_component_data *self, struct genom_activity *a);

<'foreach s [$component services] {'>
/* <"[$s name]"> */
void genom_<"$comp">_<"[$s name]">_activity_report(struct genom_component_data *self,
						   struct genom_<"$comp">_<"[$s name]">_activity *a);
<'}'>

/* exceptions */
genom_event	genom_pocolibs_raise(genom_event ex, void *detail, size_t size,
                        genom_context self);
const void *	genom_pocolibs_raised(genom_event *ex, genom_context self);


/* log functions */
void	genom_log_info(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));
void	genom_log_warn(int h2error, const char *format, ...)
  __attribute__ ((format (printf, 2, 3)));
void	genom_log_debug(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));


#endif /* H_<"$COMP">_CONTROL_TASK */

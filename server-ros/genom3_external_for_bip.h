<'
# Copyright (c) 2016 LAAS/CNRS
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
#                   

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]

lang c
'>

/* this file contains all the structure and proto the BIP Engine must know of... */

#ifndef H_<"$COMP">_GENOM3_EXTERNAL_FOR_BIP
#define H_<"$COMP">_GENOM3_EXTERNAL_FOR_BIP

 // GenoM3 types which need to be known by BIP
typedef struct genom_<"$comp">_activity *genom_<"$comp">_activity_ptr;
struct genom_<"$comp">_component_data;


#include "BIPEgenom3_external_for_bip.h"


// BIP defined functions called by GenoM3
void call_<"$comp">BIPExternalPort_push(void* arg, 
					int a); // wrapper function to call the push from C code.


/* BIP bool functions to test if an activity is of a specific RQSTID.  */
<'foreach s [$component services] {'>
bool BIP_<"$COMP">_<"[$s name]">_RQSTID_p(const int);
<'}'>


<'foreach e [dotgen types] {'>
<'  if {([$e kind] == "exception") || ([$e kind] == "event") || ([$e kind] == "pause event")} {'>
bool BIP_<"[$e cname]">_p(const genom_event);
<'}'>
<'}'>




// GenoM3 functions which are called by BIP

void   BIP_<"$comp">_init_genom(void); /* Initialize the module */
int    BIP_<"$comp">_init_mbox(void) ; /* Initialize the MBOX */
void   BIP_<"$comp">_cntrl_task_check_event(void *); /* This is the "blocking" function called by the event port
							    monitor. */
void   BIP_<"$comp">_receive_rqst(genom_<"$comp">_activity *);	 /* This is called by the event port transition.  */



// "control tasks" codels
<'foreach s [$component services] {'>
typedef void *genom_activity_<"$comp">_<"[$s name]">_ptr;
genom_event  BIP_<"$comp">_<"[$s name]">_validate(int);
genom_event  BIP_<"$comp">_<"[$s name]">_control(int);
<'	if {[$s kind] == "activity"} {'>
void BIP_<"$comp">_<"[$s name]">_activity_report(genom_activity_<"$comp">_<"[$s name]">_ptr);
genom_activity_<"$comp">_<"[$s name]">_ptr BIP_cast_activity_in_<"$comp">_<"[$s name]">_activity(int);
<'	}'>
void BIP_<"$comp">_<"[$s name]">_rqst_report(int);

  //genom_activity * BIP_cast_<"$comp">_<"[$s name]">_activity_in_activity(genom_activity_<"$comp">_<"[$s name]"> a);

<'}'>
void BIP_<"$comp">_send_ir(int);

/* casting and comparison (BIP can not handle comparison of genom_event variables) */

int BIP_compare_<"$comp">_validate(genom_event);

// "exec tasks" codels
<'foreach t [$component tasks] {'>
/* <"[--- [$t class] [$t name] --------------------------------------]"> */
<'  foreach state [$t fsm] {'>
<'    set func [$t class]_[$t name]_[$state name]'>
/* --- no specific activity structure for permantent activity ---  */
genom_event BIP_<"$comp">_codel_<"$func">(//genom_component_data *,
					  genom_<"$comp">_activity *);
<'}'>
<'foreach obj [$t services] {'>
/* <"[--- [$obj class] [$obj name] --------------------------------------]"> */
<'  foreach state [$obj fsm] {'>
<'    set func [$obj class]_[$obj name]_[$state name]'>
genom_event BIP_<"$comp">_codel_<"$func">(//genom_component_data *,
					  genom_activity_<"$comp">_<"[$obj name]">_ptr);
<'}'>
<'}'>

genom_<"$comp">_activity *BIP_<"$comp">_<"[$t name]">_permanent_activity();
<'}'>


#endif /* H_<"$COMP">_GENOM3_EXTERNAL_FOR_BIP */

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
/* this file contains all the structure and proto the BIP Engine must know of... and they
   are all in C. */

#ifdef  __cplusplus
extern "C"  {
#endif

#ifndef H_<"$COMP">_GENOM3_EXTERNAL_FOR_BIP
#define H_<"$COMP">_GENOM3_EXTERNAL_FOR_BIP

#include "genom3_global_external_for_bip.h"

// GenoM3 types which need to be known by BIP

typedef const char *genom_event;
typedef struct genom_activity *genom_activity_ptr;

// BIP defined functions called by GenoM3
void call_<"$comp">BIPExternalPort_push(void* arg, 
					struct genom_activity *a); // wrapper function to call the push from C code.


// GenoM3 functions which are called by BIP

void   BIP_<"$comp">_init_genom(void); /* Initialize the module */
int    BIP_<"$comp">_init_mbox(void) ; /* Initialize the MBOX */
void  *BIP_<"$comp">_cntrl_task_check_event(void *data); /* This is the "blocking" function called by the event port
							    monitor. */
void   BIP_<"$comp">_receive_rqst(genom_activity_ptr);	 /* This is called by the event port transition.  */


// "control tasks" codels
<'foreach s [$component services] {'>
typedef struct genom_<"$comp">_<"[$s name]">_activity * genom_<"$comp">_<"[$s name]">_activity_ptr;
genom_event  BIP_<"$comp">_<"[$s name]">_validate(genom_<"$comp">_<"[$s name]">_activity_ptr);
genom_event  BIP_<"$comp">_<"[$s name]">_control( genom_<"$comp">_<"[$s name]">_activity_ptr);
void BIP_<"$comp">_<"[$s name]">_activity_report(genom_<"$comp">_<"[$s name]">_activity_ptr);
void BIP_<"$comp">_send_ir(genom_activity_ptr);
genom_<"$comp">_<"[$s name]">_activity_ptr BIP_cast_activity_in_<"$comp">_<"[$s name]">_activity(genom_activity_ptr);
genom_activity_ptr BIP_cast_<"$comp">_<"[$s name]">_activity_inactivity(genom_<"$comp">_<"[$s name]">_activity_ptr a);

<'}'>

/* casting and comparison (BIP can not handle comparison of genom_event variables) */

int BIP_compare_<"$comp">_validate(genom_event);

// "exec tasks" codels
<'foreach t [$component tasks] {'>
/* <"[--- [$t class] [$t name] --------------------------------------]"> */
<'  foreach state [$t fsm] {'>
<'    set func [$t class]_[$t name]_[$state name]'>
/* --- no specific activity structure for permantent activity ---  */
genom_event BIP_<"$comp">_codel_<"$func">(genom_activity_ptr);
<'}'>
<'foreach obj [$t services] {'>
/* <"[--- [$obj class] [$obj name] --------------------------------------]"> */
<'  foreach state [$obj fsm] {'>
<'    set func [$obj class]_[$obj name]_[$state name]'>
genom_event BIP_<"$comp">_codel_<"$func">(genom_<"$comp">_<"[$obj name]">_activity_ptr);
<'}'>
<'}'>
<'}'>

/* BIP bool functions to test if an activity is of a specific RQSTID.  */
<'foreach s [$component services] {'>
int c_BIP_<"$COMP">_<"[$s name]">_RQSTID_p(const genom_activity_ptr);
<'}'>


/* BIP function to return an exec task permanent activity */
<'foreach t [$component tasks] {'>
genom_activity_ptr BIP_<"$comp">_<"[$t name]">_permanent_activity();
<'}'>


#endif /* H_<"$COMP">_GENOM3_EXTERNAL_FOR_BIP */

#ifdef __cplusplus
}
#endif

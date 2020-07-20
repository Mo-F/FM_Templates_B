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
set COMP [string toupper [$component name]]

# this shoul be c++ but I need to synthesize functions which look like C function, sorry
lang c
'>
#ifndef _<"$COMP">_BIP_EXTERNAL_PORT_
#define _<"$COMP">_BIP_EXTERNAL_PORT_

#include <AtomExternalPort.hpp>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <signal.h>

#include "<"$comp">_genom3_external_for_bip.h"

#define BUFFER_SIZE 64		// If we really get more than 64 request... we are in deep s...



class <"$comp">BIPExternalPort : public AtomExternalPort {
 public:
  <"$comp">BIPExternalPort(const string &name, const EventConsumptionPolicy &policy);
  virtual ~<"$comp">BIPExternalPort();

  virtual void initialize();
  virtual bool hasEvent() const;
  virtual void popEvent();
  virtual void purgeEvents(); 

  virtual TimeValue eventTime() const;
  virtual genom_activity_ptr event_get_activity();

  void push(genom_activity_ptr a); // had to move it out of the protected to call it from my C function...

 protected:
  static void *spinning(void *arg);

  genom_activity_ptr rqst[BUFFER_SIZE];
  TimeValue mTime[BUFFER_SIZE];

  int readIndex;
  int writeIndex;

  mutable pthread_mutex_t mutex;
  pthread_t pid;
};


/* BIP bool functions to test if an activity is of a specific RQSTID.  */
<'foreach s [$component services] {'>
bool BIP_<"$COMP">_<"[$s name]">_RQSTID_p(const genom_activity_ptr);
<'}'>


bool BIP_genom_ok_p(const genom_event);
<'foreach e [dotgen types] {'>
<'  if {([$e kind] == "exception") || ([$e kind] == "event") || ([$e kind] == "pause event")} {'>
bool BIP_<"[$e cname]">_p(const genom_event);
<'}'>
<'}'>


#endif


/* eof */

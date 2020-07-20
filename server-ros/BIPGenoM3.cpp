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

# this shoul be c++ but I need to synthesize functions which look like C function, sorry
lang c
'>
#include "<"$comp">_BIPGenoM3.hpp"

/* BIP bool functions to test if an activity is of a specific RQSTID.  */
<'foreach s [$component services] {'>
bool BIP_<"$COMP">_<"[$s name]">_RQSTID_p(const genom_activity_ptr a)
{
  return (c_BIP_<"$COMP">_<"[$s name]">_RQSTID_p(a) == 1);
}

<'}'>


/* BIP bool functions to test if a particular genom_event is of a specific type.  */
bool BIP_genom_ok_p(const genom_event e)
{
  return (c_BIP_genom_ok_p(e) == 1);
}

<'foreach e [dotgen types] {'>
<'  if {([$e kind] == "exception") || ([$e kind] == "event") || ([$e kind] == "pause event")} {'>
bool BIP_<"[$e cname]">_p(const genom_event e)
{
  return (c_BIP_<"[$e cname]">_p(e) == 1);
}

<'}'>
<'}'>


<"$comp">BIPExternalPort::<"$comp">BIPExternalPort(const string &name, const EventConsumptionPolicy &policy) :
  AtomExternalPort(name, policy),
  readIndex(0),
  writeIndex(0) {
}

<"$comp">BIPExternalPort::~<"$comp">BIPExternalPort() {
}

void <"$comp">BIPExternalPort::initialize() {
  pthread_create(&pid, NULL, spinning, this);
}

bool <"$comp">BIPExternalPort::hasEvent() const {
  pthread_mutex_lock(&mutex);

  bool ret = readIndex != writeIndex;

  pthread_mutex_unlock(&mutex);

  return ret;
}

void <"$comp">BIPExternalPort::popEvent() {
  pthread_mutex_lock(&mutex);

  readIndex = (readIndex + 1) % BUFFER_SIZE;

  pthread_mutex_unlock(&mutex);
}

void <"$comp">BIPExternalPort::purgeEvents() {
  pthread_mutex_lock(&mutex);

  readIndex = writeIndex;

  pthread_mutex_unlock(&mutex);
}


TimeValue <"$comp">BIPExternalPort::eventTime() const {
  pthread_mutex_lock(&mutex);

  TimeValue ret = mTime[readIndex];

  pthread_mutex_unlock(&mutex);

  return ret;
}

genom_activity_ptr <"$comp">BIPExternalPort::event_get_activity() {
  pthread_mutex_lock(&mutex);

  genom_activity_ptr ret = rqst[readIndex];

  pthread_mutex_unlock(&mutex);

  return ret;
}


void *<"$comp">BIPExternalPort::spinning(void *arg) { // why do not you use this? because is a static method.
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1);
  sigaddset(&mask, SIGALRM);
    
  // mask signals USR1 and ALARM
  // for this thread (they are used by
  // the Engine's thread for notification
  // mechanisms)
  sigprocmask(SIG_BLOCK, &mask, NULL);

  <"$comp">BIPExternalPort *port = (<"$comp">BIPExternalPort *) arg; // why the arg? not self?

  //  We need to pass a structure which also containts the genom_self component...

  while (! BIP_<"$comp">_init_mbox());

  while (true) {

    // _self need to be passed by pthread_create
    BIP_<"$comp">_cntrl_task_check_event(port); // This will do the push... 
  }

  return NULL;
}

void <"$comp">BIPExternalPort::push(genom_activity_ptr a) {
  // current value of the Engine real-time clock
  TimeValue currentTime = time();

  pthread_mutex_lock(&mutex);

  rqst[writeIndex] = a;
  mTime[writeIndex] = currentTime;
  // We should check that the fifo is not full...
  writeIndex = (writeIndex + 1) % BUFFER_SIZE;

  pthread_mutex_unlock(&mutex);

  notify();
}

extern "C" 
void call_<"$comp">BIPExternalPort_push(void* arg, genom_activity_ptr a) // wrapper function to call the push from C code.
{
  <"$comp">BIPExternalPort *port = (<"$comp">BIPExternalPort *) arg; // why the arg? not self?
  
  return port->push(a);
}

/* eof */

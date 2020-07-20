<'
# Copyright (c) 2011-2013,2015 LAAS/CNRS
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
#                                      Anthony Mallet on Thu Mar  1 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]

lang c
'>
#ifndef H_<"$COMP">_MSGLIB
#define H_<"$COMP">_MSGLIB

#include <stddef.h>

#include "portLib.h"
#include "csLib.h"

#include "serialize.h"


/* --- Service/task id ----------------------------------------------------- */

enum {
<'foreach s [$component services] {'>
  <"$COMP">_<"[$s name]">_RQSTID,
<'}'>

  /* total */
  <"$COMP">_NRQSTID
};

enum {
<'foreach t [$component tasks] {'>
  <"$comp">_<"[$t name]">_TASKID,
<'}'>

  /* total */
  <"$comp">_NTASKID
};


/* --- Mailboxes ----------------------------------------------------------- */

/* mailbox buffer size for a message of size "size" */
#define MBOX_BUF_SIZE(size)                             \
  ((size) + 4 - ((size) & 3) + sizeof(LETTER_HDR) + 8)

/* Maximum request size - compilers should optimize this away */
static __inline__ size_t
genom_<"$comp">_max_rqst_size(void)
{
  size_t m = 0, s;

<'foreach s [$component services] {'>
  /* input of <"[$s name]"> */
  s = 0;
<'  foreach p [$s parameters in inout] {'>
  s += genom_maxserialen_<"[[$p type] mangle]">();
<'  }'>
  if (m < s) m = s;

<'}'>
  return m;
}
#define  <"$COMP">_MAX_RQST_SIZE	genom_<"$comp">_max_rqst_size()

/* Size of reception mailbox */
#define  <"$COMP">_MBOX_RQST_SIZE                                \
  (MBOX_BUF_SIZE(<"$COMP">_MAX_RQST_SIZE) * SERV_NMAX_RQST_ID)


/* Maximum reply size */
static __inline__ size_t
genom_<"$comp">_max_reply_size()
{
  size_t m = 0, s;

<'foreach s [$component services] {'>
  /* output of <"[$s name]"> */
  s = 1; /* ok flag */
<'  foreach p [$s parameters inout out] {'>
  s += genom_maxserialen_<"[[$p type] mangle]">();
<'  }'>
  if (m < s) m = s;

  /* throws of <"[$s name]"> */
<'  foreach t [$s throw] {'>
  s = sizeof(<"[$t cname]">_id);
  s += genom_maxserialen_<"[$t mangle]">();
  if (m < s) m = s;
<'  }'>

<'}'>

  s = sizeof(genom_unkex_id) + sizeof(genom_unkex_detail);
  if (m < s) m = s;
  s = sizeof(genom_syserr_id) + sizeof(genom_syserr_detail);
  if (m < s) m = s;
  return m;
}
#define  <"$COMP">_MAX_REPLY_SIZE		genom_<"$comp">_max_reply_size()
#define  <"$COMP">_MAX_INTERMED_REPLY_SIZE	sizeof(int)

/* Size of reply mbox (clients) */
#define  <"$COMP">_CLIENT_MBOX_REPLY_SIZE        	\
  ((MBOX_BUF_SIZE(<"$COMP">_MAX_REPLY_SIZE) +		\
    MBOX_BUF_SIZE(<"$COMP">_MAX_INTERMED_REPLY_SIZE))	\
   * CLIENT_NMAX_RQST_ID)

#endif /* H_<"$COMP">_MSGLIB */

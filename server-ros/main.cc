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
#                                      Anthony Mallet on Fri Dec  9 2011
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]
if {[catch { $component version } version]} { set version {}}

lang c
'>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cstdio>
#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <stdlib.h>

#include "ros/ros.h"

#include "<"$comp">_genom3_external_for_bip.h"

#include "BIPEinternals.h"
#include "<"$comp">_internals.h"


/* --- log ----------------------------------------------------------------- */

void
genom_<"$comp">_log_info(const char *format, ...)
{
  va_list va;

  printf("\t[GenoM3] <"$comp"> ");
  va_start(va, format);
  vprintf(format, va);
  va_end(va);
  printf("\n");
}

void
genom_<"$comp">_log_warn(const char *format, ...)
{
  va_list va;

  fprintf(stderr, "\t[GenoM3] <"$comp"> ");
  va_start(va, format);
  vfprintf(stderr, format, va);
  va_end(va);
  fprintf(stderr, "\n");
}

void
genom_<"$comp">_log_debug(const char *format, ...)
{
  struct timeval tv;
  struct tm tm;
  va_list va;
  if (!debug) return;

  fprintf(stderr, "\t[GenoM3] <"$comp"> ");
  gettimeofday(&tv, NULL);
  localtime_r(&tv.tv_sec, &tm);
  fprintf(stderr, "%02d:%02d:%02d.%06d: ",
          tm.tm_hour, tm.tm_min, tm.tm_sec, (unsigned)tv.tv_usec);
  va_start(va, format);
  vfprintf(stderr, format, va);
  va_end(va);
  fprintf(stderr, "\n");
}

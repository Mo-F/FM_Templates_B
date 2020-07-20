<'
# Copyright (c) 2011-2014 LAAS/CNRS
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
#                                      Anthony Mallet on Thu Mar 15 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]

lang c
'>
#ifndef H_<"${COMP}">_PARAMETERS
#define H_<"${COMP}">_PARAMETERS

#include "<"$comp">_c_types.h"


/* --- Service input and output structures --------------------------------- */

<'foreach s [$component services] {'>

/* input of <"[$s name]"> */
struct genom_<"$comp">_<"[$s name]">_input {
<'  foreach p [$s parameters in inout] {'>
  <"[[$p type] declarator [$p name]]">;
<'  }'>
};

/* output of <"[$s name]"> */
struct genom_<"$comp">_<"[$s name]">_output {
<'  foreach p [$s parameters out inout] {'>
  <"[[$p type] declarator [$p name]]">;
<'  }'>
};

<'  if {[llength [$s parameters local]]} {'>
/* locals of <"[$s name]"> */
struct genom_<"$comp">_<"[$s name]">_locals {
<'    foreach p [$s parameters local] {'>
  <"[[$p type] declarator [$p name]]">;
<'    }'>
};
<'  }'>

int	genom_<"$comp">_<"[$s name]">_encode(char *buffer, int size,
		char *dst, int maxsize);
int	genom_<"$comp">_<"[$s name]">_decode(char *buffer, int size,
		char *dst, int maxsize);
<'}'>


/* --- Remote input and output structures ---------------------------------- */

<'foreach r [$component remotes] {'>

/* input of <"[$r name]"> */
struct genom_<"$comp">_<"[$r name]">_input {
<'  foreach p [$r parameters in inout] {'>
  <"[[$p type] declarator (*[$p name])]">;
<'  }'>
};

/* output of <"[$r name]"> */
struct genom_<"$comp">_<"[$r name]">_output {
<'  foreach p [$r parameters out inout] {'>
  <"[[$p type] declarator [$p name]]">;
<'  }'>
};

<'}'>


/* --- Exceptions ---------------------------------------------------------- */

<'foreach e [$component throws] {'>
int	genom_<"$comp">_<"[$e cname]">_encodex(char *buffer, int size,
                char *dst, int maxsize);
<'}'>
int	genom_<"$comp">_genom_unkex_encodex(char *buffer, int size, char *dst,
		int maxsize);
int	genom_<"$comp">_genom_syserr_encodex(char *buffer, int size, char *dst,
		int maxsize);

#endif /* H_<"${COMP}">_PARAMETERS */

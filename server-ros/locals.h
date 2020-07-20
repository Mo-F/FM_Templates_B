<'
# Copyright (c) 2013 LAAS/CNRS
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
#                                      Anthony Mallet on Wed Feb 20 2013
#
#

# check arguments
if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

lang c

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]
'>
#ifndef H_GROS_<"$COMP">_LOCALS
#define H_GROS_<"$COMP">_LOCALS

#include "<"$comp">_typecopy.h"

namespace genom {
<'foreach s [$component services] {'>

  /* <"[--- Service [$s name] -----------------------------------------]"> */

  namespace srv_<"[$s name]"> {
    struct locals {
<'  foreach p [$s parameters local] {'>
      <"[[$p type] declarator [$p name]]">;
<'  }'>

      locals() {
<'  foreach p [$s parameters local] {'>
        genom::ids::pinit(<"[$p name]">);
<'  }'>
      }

      ~locals() {
<'  foreach p [$s parameters local] {'>
        genom::ids::pfini(<"[$p name]">);
<'  }'>
      }
    };
  };

<'}'>
};

#endif /* H_GROS_<"$COMP">_LOCALS */

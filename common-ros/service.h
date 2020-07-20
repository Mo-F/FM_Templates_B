<'
# Copyright (c) 2011-2013 LAAS/CNRS
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
#                                      Anthony Mallet on Tue Nov 29 2011
#
#

# check arguments
if {[llength $argv] != 1} { error "expected arguments: services" }
lassign $argv services

lang c

# compute handy shortcuts
set component [dotgen component]
set comp [$component name]
set COMP [string toupper [$component name]]
'>

#ifndef H_GROS_<"$COMP">_SERVICE
#define H_GROS_<"$COMP">_SERVICE

#include "ros/serialization.h"

#include "<"$comp">_serialize.h"
#include "<"$comp">_typecopy.h"

<'foreach s $services {'>

/* <"[--- Service [$s name] ------------------------------------------------]"> */

<'  if {[$s kind] != "activity"} {'>
#include "<"$comp">/srv/<"[$s name]">.srv.md5"
<'  } else {'>
#include "<"$comp">/msg/<"[$s name]">ActionGoal.msg.md5"
#include "<"$comp">/msg/<"[$s name]">ActionResult.msg.md5"
<'  }'>

namespace genom {
  namespace srv_<"[$s name]"> {
    struct input {
<'  foreach p [$s parameters in inout] {'>
      <"[[$p type] declarator [$p name]]">;
<'  }'>

      boost::shared_ptr<std::map<std::string, std::string> > __connection_header;

      input() {
<'  foreach p [$s parameters in inout] {'>
        genom::ids::pinit(<"[$p name]">);
<'  }'>
      }

      ~input() {
<'  foreach p [$s parameters in inout] {'>
        genom::ids::pfini(<"[$p name]">);
<'  }'>
      }

      struct input &operator=(input &src) {
<'  foreach p [$s parameters in inout] {'>
        genom::ids::pinit(<"[$p name]">);
        genom::ids::pcopy(<"[$p name]">, src.<"[$p name]">);
<'  }'>
        return *this;
      }

      struct input &operator=(const input &src) {
<'  foreach p [$s parameters in inout] {'>
        genom::ids::pinit(<"[$p name]">);
        genom::ids::pcopy(<"[$p name]">, src.<"[$p name]">);
<'  }'>
        return *this;
      }
    };

    struct output {
      uint8_t genom_success;
      std::string genom_exdetail;
<'  foreach p [$s parameters inout out] {'>
      <"[[$p type] declarator [$p name]]">;
<'  }'>

      boost::shared_ptr<std::map<std::string, std::string> > __connection_header;

      output():genom_success(1),genom_exdetail() {
<'  foreach p [$s parameters inout out] {'>
        genom::ids::pinit(<"[$p name]">);
<'  }'>
      }

      ~output() {
<'  foreach p [$s parameters inout out] {'>
        genom::ids::pfini(<"[$p name]">);
<'  }'>
      }

      struct output &operator=(const output &src) {
        genom_success = src.genom_success;
        genom_exdetail = src.genom_exdetail;
<'  foreach p [$s parameters inout out] {'>
        genom::ids::pinit(<"[$p name]">);
        genom::ids::pcopy(<"[$p name]">, src.<"[$p name]">);
<'  }'>
        return *this;
      }
    };
  };
};

namespace ros {
  namespace message_traits {
    template<>
      struct IsMessage< genom::srv_<"[$s name]">::input >: TrueType {};
    template<>
      struct IsMessage< genom::srv_<"[$s name]">::input const>: TrueType {};
    template<>
      struct IsMessage< genom::srv_<"[$s name]">::output >: TrueType {};
    template<>
      struct IsMessage< genom::srv_<"[$s name]">::output const>: TrueType {};

    template<>
      struct IsFixedSize< genom::srv_<"[$s name]">::input >: TrueType {};
    template<>
      struct IsFixedSize< genom::srv_<"[$s name]">::output >: TrueType {};

    template<> struct MD5Sum< genom::srv_<"[$s name]">::input > {
      static const char* value()
      {
<'  if {[$s kind] != "activity"} {'>
        return genom_md5_<"[$s name]">;
<'  } else {'>
        return genom_md5_<"[$s name]">ActionGoal;
<'  }'>
      }

      static const char* value(const genom::srv_<"[$s name]">::input &) {
        return value();
      }
    };

    template<> struct MD5Sum< genom::srv_<"[$s name]">::output > {
      static const char* value()
      {
<'  if {[$s kind] != "activity"} {'>
        return genom_md5_<"[$s name]">;
<'  } else {'>
        return genom_md5_<"[$s name]">ActionResult;
<'  }'>
      }

      static const char* value(const genom::srv_<"[$s name]">::output &) {
        return value();
      }
    };

    template<> struct DataType< genom::srv_<"[$s name]">::input > {
      static const char* value() {
<'  if {[$s kind] != "activity"} {'>
        return "<"$comp">/<"[$s name]">";
<'  } else {'>
        return "<"$comp">/<"[$s name]">ActionGoal";
<'  }'>
      }

      static const char* value(const genom::srv_<"[$s name]">::input &) {
        return value();
      }
    };

    template<> struct DataType< genom::srv_<"[$s name]">::output > {
      static const char* value() {
<'  if {[$s kind] != "activity"} {'>
        return "<"$comp">/<"[$s name]">";
<'  } else {'>
        return "<"$comp">/<"[$s name]">ActionResult";
<'  }'>
      }

      static const char* value(const genom::srv_<"[$s name]">::output &) {
        return value();
      }
    };

    template<> struct Definition< genom::srv_<"[$s name]">::input > {
      static const char* value()
      {
        return "<"[string map {\n \\n\\\n} [rossrv $s]]">";
      }

      static const char* value(const genom::srv_<"[$s name]">::input &) {
        return value();
      }
    };

    template<> struct Definition< genom::srv_<"[$s name]">::output > {
      static const char* value()
      {
        return "<"[string map {\n \\n\\\n} [rossrv $s]]">";
      }

      static const char* value(const genom::srv_<"[$s name]">::output &) {
        return value();
      }
    };
  };

<'  if {[$s kind] != "activity"} {'>
  namespace service_traits {
    template<> struct MD5Sum< genom::srv_<"[$s name]">::input > {
      static const char* value()
      {
        return genom_md5_<"[$s name]">;
      }

      static const char* value(const genom::srv_<"[$s name]">::input &) {
        return value();
      }
    };

    template<> struct MD5Sum< genom::srv_<"[$s name]">::output > {
      static const char* value()
      {
        return genom_md5_<"[$s name]">;
      }

      static const char* value(const genom::srv_<"[$s name]">::output &) {
        return value();
      }
    };

    template<> struct DataType< genom::srv_<"[$s name]">::input > {
      static const char* value() {
        return "<"$comp">/<"[$s name]">";
      }

      static const char* value(const genom::srv_<"[$s name]">::input &) {
        return value();
      }
    };

    template<> struct DataType< genom::srv_<"[$s name]">::output > {
      static const char* value() {
        return "<"$comp">/<"[$s name]">";
      }

      static const char* value(const genom::srv_<"[$s name]">::output &) {
        return value();
      }
    };
  };
<'  }'>

  namespace serialization {
    template<> struct Serializer< genom::srv_<"[$s name]">::input > {
      template<typename Stream, typename T>
      inline static void allInOne(Stream& stream, T m) {
<'  foreach p [$s parameters in inout] {
    switch -- [[$p type] kind] {
      array {
        set l [[$p type] length]
'>
        for(uint32_t i = 0; i < <"$l">; i++) {
          stream.next(m.<"[$p name]">[i]);
        }
<'      }
      default {'>
        stream.next(m.<"[$p name]">);
<'      }
    }
  }'>
      }

      ROS_DECLARE_ALLINONE_SERIALIZER;
    };

    template<> struct Serializer< genom::srv_<"[$s name]">::output > {
      template<typename Stream, typename T>
      inline static void allInOne(Stream& stream, T m) {
        stream.next(m.genom_success);
        stream.next(m.genom_exdetail);

<'  foreach p [$s parameters inout out] {
    switch -- [[$p type] kind] {
      array {
        set l [[$p type] length]
'>
        for(uint32_t i = 0; i < <"$l">; i++) {
          stream.next(m.<"[$p name]">[i]);
        }
<'      }
      default {'>
        stream.next(m.<"[$p name]">);
<'      }
    }
  }'>
      }

      ROS_DECLARE_ALLINONE_SERIALIZER;
    };
  };
};

<'}'>

#endif /* H_GROS_<"$COMP">_SERVICE */

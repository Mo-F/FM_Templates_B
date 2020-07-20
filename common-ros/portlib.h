<'
#
# Copyright (c) 2011-2012 LAAS/CNRS
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
#                                      Anthony Mallet on Thu Dec 15 2011
#
#

lang c

# compute handy shortcuts
set component [dotgen component]
set comp [$component name]
set roscomp genom_$comp
set COMP [string toupper [$component name]]
'>
#ifndef H_GROS_<"$COMP">_PORTLIB
#define H_GROS_<"$COMP">_PORTLIB

#include <pthread.h>

#include <cstdlib>
#include <cstring>
#include <map>

#include "ros/ros.h"
#include "ros/serialization.h"

#include "<"$comp">_serialize.h"
#include "<"$comp">_typecopy.h"

<'foreach p [$component ports] {'>

/* <"[--- Port [$p name] ---------------------------------------------------]"> */

<'  if {[catch {[$p datatype] masquerade ros} masquerade]} {'>
#include "<"$comp">/msg/<"[rosmsg_cname [$p datatype]]">.msg.md5"
<'  } else {'>
#include "<"$masquerade">.h"
<'  }'>

namespace genom {
  struct port_<"[$p name]"> {
    ::<"[[$p datatype] declarator v]">;
    boost::shared_ptr<std::map<std::string, std::string> > __connection_header;

    port_<"[$p name]">() {
      genom::ids::pinit(v);
    }

    ~port_<"[$p name]">() {
      genom::ids::pfini(v);
    }
  };
};

namespace ros {
  namespace message_traits {
    template<>
      struct IsMessage< genom::port_<"[$p name]"> > : TrueType {};
    template<>
      struct IsMessage< genom::port_<"[$p name]"> const > : TrueType {};

    template<>
      struct MD5Sum< genom::port_<"[$p name]"> > {
      static const char *value() {
<'  if {[catch {[$p datatype] masquerade ros} masquerade]} {'>
        return genom_md5_<"[rosmsg_cname [$p datatype]]">;
<'  } else {'>
        return MD5Sum< <"[string map {/ ::} $masquerade]"> >::value();
<'  }'>
      }

      static const char* value(const genom::port_<"[$p name]"> &) {
        return value();
      }
    };

    template<> struct DataType< genom::port_<"[$p name]"> > {
      static const char* value() {
<'  if {[catch {[$p datatype] masquerade ros} masquerade]} {'>
        return "<"$comp">/<"[rosmsg_cname [$p datatype]]">";
<'  } else {'>
        return DataType< <"[string map {/ ::} $masquerade]"> >::value();
<'  }'>
      }

      static const char* value(const genom::port_<"[$p name]"> &) {
        return value();
      }
    };

    template<> struct Definition< genom::port_<"[$p name]"> > {
      static const char* value()
      {
<'  if {[catch {[$p datatype] masquerade ros} masquerade]} {'>
        return "<"[string map {\n \\n\\\n} [rosmsg [$p datatype]]]">";
<'  } else {'>
        return Definition< <"[string map {/ ::} $masquerade]"> >::value();
<'  }'>
      }

      static const char* value(const genom::port_<"[$p name]"> &) {
        return value();
      }
    };
  };

  namespace serialization {
    template<> struct Serializer< genom::port_<"[$p name]"> > {
      template<typename Stream, typename T>
        inline static void allInOne(Stream& stream, T m) {
        stream.next(m.v);
      }
      ROS_DECLARE_ALLINONE_SERIALIZER;
    };
  };
};

<'}'>

#endif /* H_GROS_<"$COMP">_PORTLIB */

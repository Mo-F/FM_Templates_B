<'
#
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
#                                      Anthony Mallet on Thu Dec 15 2011
#
#

lang c


if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]
'>
#ifndef H_GROS_<"$COMP">_PORT
#define H_GROS_<"$COMP">_PORT

#include <map>

#include "ros/master.h"

#include "<"${comp}">_c_types.h"
#include "<"${comp}">_portlib.h"

/* forward declaration */
struct genom_component_data;


/* === Handle functions ==================================================== */

extern "C" {
<'foreach p [$component ports] {'>
<'  switch -- [$p kind] {'>
<'    simple {'>
  <"[[$p datatype] argument reference]">
		genom_<"$comp">_<"[$p name]">_data(genom_context self);
<'      switch -- [$p dir] {'>
<'        in {'>
  genom_event	genom_<"$comp">_<"[$p name]">_read(genom_context self);
<'        }'>
<'        out {'>
  genom_event	genom_<"$comp">_<"[$p name]">_write(genom_context self);
<'        }'>
<'      }'>
<'    }'>
<'    multiple {'>
  <"[[$p datatype] argument reference]">
		genom_<"$comp">_<"[$p name]">_data(const char *id,
                             genom_context self);
<'      switch -- [$p dir] {'>
<'        in {'>
  genom_event	genom_<"$comp">_<"[$p name]">_read(const char *id,
                             genom_context self);
<'        }'>
<'        out {'>
  genom_event	genom_<"$comp">_<"[$p name]">_write(const char *id,
                             genom_context self);
  genom_event	genom_<"$comp">_<"[$p name]">_open(const char *id,
                             genom_context self);
  genom_event	genom_<"$comp">_<"[$p name]">_close(const char *id,
                             genom_context self);
<'        }'>
<'      }'>
<'    }'>
<'    default { error "unknown port kind" }'>
<'  }'>
<'}'>

}


/* --- input ports --------------------------------------------------------- */

<'foreach p [$component ports in] {'>
struct genom_port_<"[$p name]">_base {
  pthread_spinlock_t lock;
  boost::shared_ptr<genom::port_<"[$p name]"> const> data, latest;
  ros::Subscriber sub;

  void datacb(const boost::shared_ptr<genom::port_<"[$p name]"> const> &d);

  int subscribe(ros::NodeHandle *node, const char *name)
  {
    ros::master::V_TopicInfo topics;
    ros::master::V_TopicInfo::iterator i;
    std::string absname = ros::names::resolve(name);

    if (data) data.reset();
    if (sub) sub.shutdown();

    if (!ros::master::getTopics(topics)) return 1;
    for(i = topics.begin(); i != topics.end(); i++)
      if (!i->name.compare(absname)) break;
    if (i == topics.end()) return 1;

    sub = node->subscribe(
      absname, 1, &genom_port_<"[$p name]">_base::datacb, this,
      ros::TransportHints().tcpNoDelay());
    return !sub;
  }

  void fini(void)
  {
    if (data) data.reset();
    if (sub) sub.shutdown();
    pthread_spin_destroy(&lock);
  }
};

<'}'>
<'foreach p [$component ports in simple] {'>
struct genom_port_<"[$p name]"> : genom_port_<"[$p name]">_base {
  <"[[$p type] declarator handle]">;

  int init(ros::NodeHandle *node)
  {
    if (pthread_spin_init(&lock, 0)) return errno;
    handle.data = genom_<"$comp">_<"[$p name]">_data;
    handle.read = genom_<"$comp">_<"[$p name]">_read;
    return 0;
  }
};

<'}'>
<'foreach p [$component ports in multiple] {'>
struct genom_port_<"[$p name]"> {
  <"[[$p type] declarator handle]">;

  typedef std::map<std::string, genom_port_<"[$p name]">_base> items_s;
  items_s items;

  int init(ros::NodeHandle *node)
  {
    handle.data = genom_<"$comp">_<"[$p name]">_data;
    handle.read = genom_<"$comp">_<"[$p name]">_read;
    return 0;
  }

  inline int subscribe(ros::NodeHandle *node, const char *id, const char *name)
  {
    int s;

    items_s::iterator i = items.find(id);
    if (i == items.end()) {
      s = open(node, id);
      if (s) return s;
      i = items.find(id);
      if (i == items.end()) return ENOENT;
    }

    return i->second.subscribe(node, name);
  }

  inline genom_port_<"[$p name]">_base *find(const char *id)
  {
    items_s::iterator i = items.find(id);
    if (i == items.end()) return NULL;

    return &i->second;
  }

  inline int open(ros::NodeHandle *node, const char *id)
  {
    std::pair<items_s::iterator, bool> i;
    genom_port_<"[$p name]">_base p;

    i = items.insert(items_s::value_type(id, p));
    if (!i.second) return EEXIST;

    if (pthread_spin_init(&i.first->second.lock, 0)) {
      items.erase(i.first);
      return errno;
    }
    return 0;
  }

  inline void close(const char *id)
  {
    items_s::iterator i = items.find(id);
    if (i == items.end()) return;

    i->second.fini();
    items.erase(i);
  }

  inline void fini(void)
  {
    for(items_s::iterator i = items.begin(); i != items.end(); i++) {
      i->second.fini();
    }
    items.clear();
  }
};
<'}'>

/* --- output ports -------------------------------------------------------- */

<'foreach p [$component ports out] {'>
struct genom_port_<"[$p name]">_base {
  <"[[$p type] declarator handle]">;
  genom::port_<"[$p name]"> *data;
  ros::Publisher pub;

  void publish(void) { pub.publish(*data); }
  void fini(void) { pub.shutdown(); delete data; }
};
<'}'>

<'foreach p [$component ports out simple] {'>
struct genom_port_<"[$p name]"> : genom_port_<"[$p name]">_base {
  <"[[$p type] declarator handle]">;

  int init(ros::NodeHandle *node)
  {
    handle.data = genom_<"$comp">_<"[$p name]">_data;
    handle.write = genom_<"$comp">_<"[$p name]">_write;

    data = new genom::port_<"[$p name]">;
    if (!data) return ENOMEM;

    pub = node->advertise<genom::port_<"[$p name]"> >(
      "<"[$p name]">", 1, true);
    return !pub;
  }
};
<'}'>
<'foreach p [$component ports out multiple] {'>
struct genom_port_<"[$p name]"> {
  <"[[$p type] declarator handle]">;

  typedef std::map<std::string, genom_port_<"[$p name]">_base> items_s;
  items_s items;

  int init(ros::NodeHandle *node)
  {
    handle.data = genom_<"$comp">_<"[$p name]">_data;
    handle.write = genom_<"$comp">_<"[$p name]">_write;
    handle.open = genom_<"$comp">_<"[$p name]">_open;
    handle.close = genom_<"$comp">_<"[$p name]">_close;
    return 0;
  }

  void
  publish(const char *id)
  {
    items_s::iterator i = items.find(id);
    if (i == items.end()) return;

    i->second.publish();
  }

  void
  fini()
  {
    for(items_s::iterator i = items.begin(); i != items.end(); i++) {
      i->second.fini();
    }
    items.clear();
  }

  genom_port_<"[$p name]">_base *
  find(const char *id)
  {
    items_s::iterator i = items.find(id);
    if (i == items.end()) return NULL;

    return &i->second;
  }

  inline int
  open(ros::NodeHandle *node, const char *id)
  {
    std::pair<items_s::iterator, bool> i;
    i.first = items.find(id);
    if (i.first != items.end()) return EEXIST;

    genom_port_<"[$p name]">_base p;
    std::string n = std::string("<"[$p name]">/") + id;

    p.data = new genom::port_<"[$p name]">;
    if (!p.data) return ENOMEM;
    p.pub = node->advertise<genom::port_<"[$p name]"> >(n, 1, true);
    if (!p.pub) { delete p.data; return EINVAL; }

    i = items.insert(items_s::value_type(id, p));
    return !i.second;
  }

  inline void
  close(const char *id)
  {
    items_s::iterator i = items.find(id);
    if (i == items.end()) return;

    i->second.fini();
    items.erase(i);
  }
};
<'}'>

#endif /* H_GROS_<"$COMP">_PORT */

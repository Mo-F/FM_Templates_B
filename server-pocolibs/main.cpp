<'
# Copyright (c) 2017 LAAS/CNRS
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
#                                      Felix Ingrand on 20/04/2017
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]

lang c++
'>

#include <Launcher.hpp>
#include <BipCallBack.hpp>
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */


extern "C" {
void BIP_<"$comp">_init_genom(void); // This should be in an include for consistency checking...
long taskFromThread(const char *name);
}

Component* deploy(int argc, char **argv);

int main(int argc, char **argv) {
    int ret = EXIT_SUCCESS;

    class MyBipCallBack1 : public BipCallBack {      
      virtual void callBackMethod() { 
	cout << "init call back!" << endl; 
	BIP_<"$comp">_init_genom();

      }
    };
      
    class MyBipCallBack2 : public BipCallBack {
      virtual void callBackMethod() {
	int i = syscall(SYS_gettid);
	cout << "thread callback tid = " << i << endl; 
	taskFromThread(NULL);
}
    };

    MyBipCallBack1 call1;
    MyBipCallBack2 call2;


    // deploy the system corresponding to the root component
    Component &component = *deploy(argc, argv);

    // create an engine launcher
    Launcher launcher(argc, argv, component, call1, call2);

    // initialize the launcher (components, etc.)
    ret = launcher.initialize();

    // run the engine
    if (ret == EXIT_SUCCESS) {
      ret = launcher.launch();
    }

    return ret;
}

/* eof */

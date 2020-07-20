<'
# Copyright (c) 2012-2015 LAAS/CNRS
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
#



# compute handy shortcuts

lang c'>

/* this model is automatically generated - Mohammed Foughali 2016 */

/* package */

package <'
foreach component [dotgen components] {'><"[$component name]"><'
	if {$component != [lindex [dotgen components] end]} {'>_<'}}'>


/* definition of port types */

port type Port() <"\n">
/* definition of connector types types */<"\n"><'
set max 2
set synclengths {2 3 4}
set triglengths {2 3}

foreach component [dotgen components] {
set instpertask [list]
set interperservice [list]

set lockspercodel [list]

set requests [list]
set activities [list]
set nonactivities [list]

foreach t [$component tasks] {
lappend instpertask [expr [llength [$t services]]*$max]}

foreach s [$component services] {
if {[$s name]!="abort_activity" && [$s name]!="kill" && [$s name]!="connect_port" && [$s name]!="connect_service"} {
	lappend requests $s
	if {[$s kind] == "activity"} {lappend activities $s
	} else {lappend nonactivities $s}
	if {$s in [$s interrupts]} {lappend interperservice [expr [llength [$s interrupts]]*$max-1]
	} else {lappend interperservice [expr [llength [$s interrupts]]*$max]}
	foreach c [$s validate] {
	if {[llength [$c mutex]]} {
	lappend lockspercodel [llength [$c mutex]]}}
	foreach c [$s codels] {
	if {[llength [$c mutex]]} {
	lappend lockspercodel [llength [$c mutex]]}}
	if {[$s kind]!="activity"} {
	if {[llength [$s mutex]]} {
	lappend lockspercodel [llength [$s mutex]]}}
	}
}

foreach t [$component tasks] {
	foreach c [$t codels] {
		if {[llength [$c mutex]]} {lappend lockspercodel [llength [$c mutex]]}}}

foreach i $instpertask {
set test 0
foreach sync $synclengths {
if {[expr $i+1]==$sync} {set test 1
			break}}
if {!$test} {lappend synclengths [expr $i+1]}}

foreach p $interperservice {
set test 0
foreach sync $synclengths {
if {[expr $p+2]==$sync} {set test 1
			break}}
if {!$test} {lappend synclengths [expr $p+2]}}

foreach l $lockspercodel {
set test 0
foreach sync $synclengths {
if {[expr $l+2]==$sync} {set test 1
			break}}
if {!$test} {lappend synclengths [expr $l+2]}}

set allinst 0
foreach i $instpertask {
set allinst [expr $allinst+$i]}

foreach p $interperservice {
set test 0
if {[expr $p%$max]} {incr p}
foreach trig $triglengths {
if {[expr $p+1]==$trig || $p==0} {set test 1
			break}}
if {!$test} {lappend triglengths [expr $p+1]}}

set test 0
foreach trig $triglengths {
if {[expr $allinst+1]==$trig} {set test 1
			break}}
if {!$test} {lappend triglengths [expr $allinst+1]}}'>

/* synchrones */<"\n"><'

foreach sync $synclengths {'><"\n">connector type sync<"$sync">(<'
for {set k 1} {$k <= $sync} {incr k} {'>Port p<"$k"><'
	if {$k < $sync} {'>, <'}}'>)<"\n">define <'
for {set k 1} {$k <= $sync} {incr k} {'>p<"$k"> <'}'><"\n">end<"\n\n"><'}

foreach sync $synclengths {'><"\n">connector type sync<"$sync">_exp(<'
for {set k 1} {$k <= $sync} {incr k} {'>Port p<"$k"><'
	if {$k < $sync} {'>, <'}}'>)<"\n">export port Port exp()<"\n">define <'
for {set k 1} {$k <= $sync} {incr k} {'>p<"$k"> <'}'><"\n">end<"\n\n"><'}'>/* triggers */<"\n"><'


foreach trig $triglengths {'><"\n">connector type trig<"$trig">(<'
for {set k 1} {$k <= $trig} {incr k} {'>Port p<"$k"><'
	if {$k < $trig} {'>, <'}}'>)<"\n">define <'
for {set k 1} {$k <= $trig} {incr k} {'>p<"$k"><'
if {$k == 1} {'>'<'}'> <'}'><"\n">end<"\n\n"><'}'>

/* definition of atoms */

/* global types */

/* mutual exclusion */

atom type LOCK()
	export port Port take()
	export port Port give()
	export port Port check()

	place free, taken

	initial to free

	on check
	from free to free

	on take
	from free to taken

	on give
	from taken to free

end


/* manage activities termination */

atom type SIGNAL()

	export port Port sig()
	export port Port endsig()
	export port Port hold()

	place start, intermediate, notify

	initial to start

	on sig
	from start to intermediate

	on sig
	from intermediate to intermediate

	on hold
	from intermediate to notify

	on sig
	from notify to notify

	on endsig
	from notify to start
end

/* activities status */

atom type STATUS()
	export port Port activ()
	export port Port deactiv()
    export port Port launch()
    export port Port hold()
    export port Port interrupt()
    export port Port clear()
    export port Port void()
    export port Port run()
    export port Port inter()
    export port Port finished()

place idle, activate, start, running, stopp, ether

	initial to idle

	on activ
	from idle to activate

	on deactiv
	from activate to idle

	on hold
	from activate to start

	on launch
	from start to running

	on interrupt
	from running to stopp

	on interrupt
	from start to ether

	on clear
	from ether to idle

	on void
	from idle to idle

	on void
	from ether to ether

	on void
	from activate to activate

	on void
	from start to start

	on inter
	from stopp to stopp

	on finished
	from stopp to ether

	on finished
	from running to ether

	on run
	from running to running


end


<'
set gcounter 0
foreach component [dotgen component] {
	set requests [list]
	set activities [list]
	set nonactivities [list]
	foreach s [$component services] {
if {[$s name]!="abort_activity" && [$s name]!="kill" && [$s name]!="connect_port" && [$s name]!="connect_service"} {
	lappend requests $s
	if {[$s kind] == "activity"} {lappend activities $s
	} else {lappend nonactivities $s}
	
}}'>
	
/* component <"[$component name]"> types */ 

/* client */

atom type CLIENT_<"[$component name]">()
	export port Port norequest()
	export port Port finished()<"\n"><'
	foreach r $requests {'>
	export port Port req_<"[$r name]">()<"\n"><'}'>
	
	place idle, waiting
	
	initial to idle
	
	on norequest
	from idle to waiting
		
<'foreach r $requests {'>
	on req_<"[$r name]">
	from idle to waiting<"\n"><'}'>
	
	on finished
	from waiting to idle
	
end 

/* control task */

atom type CONTROL_<"[$component name]">()
	clock c unit millisecond
	export port Port spawn()
	export port Port norequest()
	export port Port immediate()
	export port Port clear()
	export port Port launch()
	export port Port finished()<"\n"><'
	set counter 0
	foreach r $requests {'>
	export port Port req_<"[$r name]">()<"\n"><'
	if {[$r kind]=="activity"} {'>
	export port Port act_<"[$r name]">()<"\n"><'
	} else {
	set mut($counter) 0
	foreach c [$r codels] {
		if {[llength [$c mutex]] || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]])} {
			set mut($counter) 1
			break}}
	if {[llength [$r mutex]] || $mut($counter) || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $r]])} {'>
	export port Port lock_res_<"[$r name]">()<"\n\t">export port Port unlock_res_<"[$r name]">()<"\n"><'
	} else {'>
	port Port exec_<"[$r name]">()<"\n"><'}}
	if {[llength [$r validate]]} {'>
	port Port valid_<"[$r name]">()<"\n\t">export port Port invalid_<"[$r name]">()<"\n"><'
	if {[llength [[$r validate] mutex]] || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen [$r validate]]])} {'>
	export port Port lock_res_<"[$r name]">_validate()<"\n\t">export port Port unlock_res_<"[$r name]">_validate()<"\n"><'
	} else {'>
	port Port <"[$r name]">_exe_validate()<"\n"><'}}
	if {[llength [$r interrupts]]} {'>
	export port Port inter_<"[$r name]">()<"\n"><'}
	incr counter}'>
	
	place unspawned, idle, ready, <'
	set counter 0
	foreach r $requests {
	if {[$r kind]=="activity"} {'><"[$r name]">_, <'
	if {[llength [$r interrupts]]} {'><"[$r name]">_interrupt, <'} 
	} else {'><"[$r name]">_, <'
	if {[llength [$r mutex]] || $mut($counter) || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $r]])} {'> <"[$r name]">_2, <'}
	if {[llength [$r interrupts]]} {'><"[$r name]">_interrupt, <'}}
	if {[llength [$r validate]]} {'><"[$r name]">_validate, <"[$r name]">_validate_2, <'
	if {[llength [[$r validate] mutex]]|| ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen [$r validate]]])} {'><"[$r name]">_validate_3, <'}}
	incr counter}'>free, release
	
	initial to unspawned
	
	
	on spawn
	from unspawned to idle
	eager
	resume

	on immediate
	from idle to ready
	eager
	resume
	
	on norequest
	from ready to free
	eager
	resume
	
<'
set counter 0
foreach r $requests {'>
	on req_<"[$r name]">
	from idle to <"[$r name]">_<'
	if {[llength [$r validate]]} {'>validate<'}'>
	
	do {c = 0;}
	resume
	
	on req_<"[$r name]">
	from ready to <"[$r name]">_<'
	if {[llength [$r validate]]} {'>validate<'}'>
	
	eager
	do {c = 0;}
	resume

	
<'if {[llength [$r validate]]} {
	if {[llength [[$r validate] mutex]] || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen [$r validate]]])} {'>
	on lock_res_<"[$r name]">_validate
	from <"[$r name]">_validate to <"[$r name]">_validate_2
	eager
	do {c = 0;}
	resume
	
	on unlock_res_<"[$r name]">_validate
	from <"[$r name]">_validate_2 to <"[$r name]">_validate_3
	eager
	resume<'
	if {![catch {[$r validate] wcet}]} {'> (c <= <"[expr [[[$r validate] wcet] value]*1000]">)<'}'>

	
	on valid_<"[$r name]">
	from <"[$r name]">_validate_3 to <"[$r name]">_
	eager
	resume
	
	on invalid_<"[$r name]">
	from <"[$r name]">_validate_3 to free
	eager
	resume

	
<'} else {'>
	
	on <"[$r name]">_exe_validate
	from <"[$r name]">_validate to <"[$r name]">_validate_2
	eager
	resume<'if {![catch {[$r validate] wcet}]} {'> (c <= <"[expr [[[$r validate] wcet] value]*1000]">)<'}'>
	
	
	on valid_<"[$r name]">
	from <"[$r name]">_validate_2 to <"[$r name]">_
	eager
	resume
	
	on invalid_<"[$r name]">
	from <"[$r name]">_validate_2 to free
	eager
	resume
	
	
<'}}
if {[$r kind]=="activity"} {
	if {[llength [$r interrupts]]} {'>
	on inter_<"[$r name]">
	from <"[$r name]">_ to <"[$r name]">_interrupt
	eager
	resume
	
	on act_<"[$r name]">
	from <"[$r name]">_interrupt to free
	eager
	resume
	
	
<'} else {'>
	on act_<"[$r name]">
	from <"[$r name]">_ to free
	eager
	resume

	
<'}
} else {
if {[llength [$r interrupts]]} {
	if {[llength [$r mutex]] || $mut($counter) || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $r]])} {'>
	on lock_res_<"[$r name]">
	from <"[$r name]">_ to <"[$r name]">_2
	eager
	do {c = 0;}
	resume
	
	on unlock_res_<"[$r name]">
	from <"[$r name]">_2 to <"[$r name]">_interrupt
	eager
	resume<'if {![catch {$r wcet}]} {'> (c <= <"[expr [$r wcet]*1000]">)<'}'>

	
<'} else {'>
	on exec_<"[$r name]">
	from <"[$r name]">_ to <"[$r name]">_interrupt
	eager
	resume<'if {![catch {$r wcet}]} {'> (c <= <"[expr [$r wcet]*1000]">)<'}'>

	
<'}'>

	on inter_<"[$r name]">
	from <"[$r name]">_interrupt to free
	eager
	resume

	
<'} else {
	if {[llength [$r mutex]] || $mut($counter) || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $r]])} {'>
	on lock_res_<"[$r name]">
	from <"[$r name]">_ to <"[$r name]">_2
	eager
	do {c = 0;}
	resume
	
	on unlock_res_<"[$r name]">
	from <"[$r name]">_2 to free
	eager
	resume<'if {![catch {$r wcet}]} {'> (c <= <"[expr [$r wcet]*1000]">)<'}'>

	
<'} else {'>
	on exec_<"[$r name]">
	from <"[$r name]">_ to free
	eager
	resume<'if {![catch {$r wcet}]} {'> (c <= <"[expr [$r wcet]*1000]">)<'}'>

	
<'}}}
incr counter}'>
	
	on clear
	from free to release
	eager
	resume
	
	on launch
	from release to release
	eager
	resume
	
	on finished
	from release to idle
	eager
	resume
	
end


<'set counter 0
set permanent [list]
foreach t [$component tasks] {
	set nostopt($counter) [list]
	set resumeperm($counter) [list]
	set resumepermstart($counter) [list]
	if {![catch {$t period}]} {'>
		

/* <"[$t name]"> timer */

atom type TIMER_<"[$t name]">_<"[$component name]">()
	clock c unit millisecond
	export port Port tick()

	place loop

	initial to loop

	on tick
	from loop to loop
	provided (c==<"[expr [[$t period] value]*1000]">)
	do { c = 0; }
end

<'}
if {[llength [$t codels]]} {
lappend permanent $t
'>
	
/* <"[$t name]"> permanent activity */

atom type PERM_<"[$t name]">_<"[$component name]">()
	clock c unit millisecond

<'set test [list]
set startyields [list]
	foreach c [$t codels] {
		foreach tr [$c triggers] {
	if {[$tr name]=="start"} {
		foreach y [$c yields] {
		lappend startyields $y}'>
		
		
	<'if {$t != [lindex [$component tasks] 0]} {'>export <'}'>port Port begin()<'
	} else {'><"\n\t"><'
		if {$tr in $startyields} {'>export <'}'>port Port to_<"[$tr name]">()<"\n"><'}
			 
		if {[llength [$c mutex]] || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]])} {'>
	
	export port Port lock_res_<"[$c name]">()<"\n\t">export port Port unlock_res_<"[$c name]">()<"\n"><'}}
	if {!([llength [$c mutex]] || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]]))} {'>
	port Port to_<"[$tr name]">_test()<"\n"><'}
		foreach y [$c yields] {
			if {[$y kind] == "pause event"} {
				if {[$y name] in $test} {continue}
				lappend test [$y name]
				if {$y in $startyields} {
				lappend resumepermstart($counter) [$y name]}}}}
	if {[llength $test]} {
	set resumeperm($counter) $test'>
	
	export port Port resume_<"\n"><'
	foreach te $test {'>
	export port Port to_pause_<"$te">()<"\n"><'}}'>
	
	export port Port to_ether()
	export port Port void()
	

	place idle<'
	foreach c [$t codels] {
		foreach tr [$c triggers] {'>, <"[$tr name]">_, <"[$tr name]">_test<'
		if {[llength [$c mutex]] || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]])} {'>, <"[$tr name]">_2<'}}
	foreach te $test {'>, pause_<"$te"><'}'>, ether_

	initial to idle

<'if {$t == [lindex [$component tasks] 0]} {'>resume<'}'>

	
	on begin
	from idle to start_ // spawning <"[$t name]">
	eager
	do {c = 0;}
	resume

<'foreach c [$t codels] {
		foreach tr [$c triggers] {
		if {[llength [$c mutex]] || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]])} {'>
				
	on lock_res_<"[$c name]">
	from <"[$tr name]">_ to <"[$tr name]">_2
	eager
	do {c = 0;}
	resume
	
	
	on unlock_res_<"[$c name]"> // call codel
	from <"[$tr name]">_2 to <"[$tr name]">_test
	eager
	resume<'
	if {![catch {$c wcet}]} {'> (c <= <"[expr [[$c wcet] value]*1000]">)<'}'>

	
<'} else {'>
		
	on to_<"[$tr name]">_test // call codel
	from <"[$tr name]">_ to <"[$tr name]">_test
	eager
	resume<'
	if {![catch {$c wcet}]} {'> (c <= <"[expr [[$c wcet] value]*1000]">)<'}}
	
	foreach y [$c yields] {
		
	if {[$y kind] == "pause event"} {'>
		
	on to_pause_<"[$y name]">
	from <"[$tr name]">_test to pause_<"[$y name]">
	eager
	resume
	
	on resume_
	from pause_<"[$y name]"> to <"[$y name]">_
	do {c = 0;}
	resume
	
<'} else {'>
	
	on to_<"[$y name]">
	from <"[$tr name]">_test to <"[$y name]">_
	eager
	do {c = 0;}
	resume
<'}}}}'>

	on void
	from ether_ to ether_
	eager
	resume
	
end
	  
<'}}'>

/* <"[$t name]"> manager */

atom type MANAGER_<"[$t name]">_<"[$component name]">()
	clock c unit millisecond

<'if {![catch {$t period}]} {'>
	export port Port tick()<'}
if {[llength [$t services]]} {'>
	
	export port Port skipall()<'}'>
	export port Port ended()<'
	if {[llength [$t codels]]} {'>
	
	export port Port begin_perm()
	export port Port cycle_perm()
	export port Port skip_perm()
	
<'}
set nostop [list]
	foreach s [$t services] {'>
		
	export port Port skip_<"[$s name]">_1()
	export port Port skip_<"[$s name]">_2()
	export port Port begin_<"[$s name]">_1()
	export port Port begin_<"[$s name]">_2()
	export port Port cycle_<"[$s name]">_1()
	export port Port cycle_<"[$s name]">_2()
	
<'
set flag 0
foreach st [$s fsm] {
		if {[$st name] == "stop"} {
		set flag 1
		break}
		}
		if {!$flag} {lappend nostop [$s name]
					lappend nostopt($counter) [$s name]}}
		
	foreach n $nostop {'>
	export port Port inter_1_<"$n">_1()	
	export port Port inter_2_<"$n">_1()
	export port Port inter_1_<"$n">_2()
	export port Port inter_2_<"$n">_2()<'}'>
	
	
	place idle, <'
	if {[llength [$t codels]]} {'>start_perm, end_perm, <'}
	foreach s [$t services] {'>start_<"[$s name]">_1, end_<"[$s name]">_1, start_<"[$s name]">_2, end_<"[$s name]">_2, <'}
	foreach n $nostop {'>interrupt_<"$n">_1, interrupt_<"$n">_2, <'}'>finish
	
	initial to idle
	
<'if {![catch {$t period}]} {'>
	on tick
	from idle to <'
	if {[llength [$t codels]]} {'>start_perm<'
	} else {'>start_<"[[lindex [$t services] 0] name]"><'}'>
	
	eager
	resume
	
<'}

	if {[llength [$t codels]]} {'>
		
	on begin_perm
	from start_perm to end_perm
	eager
	
	on cycle_perm
	from end_perm to <'
	if {[llength [$t services]]} {'>start_<"[[lindex [$t services] 0] name]">_1<'
	} else {'>idle<'}'>
	resume
	
	on skip_perm
	from start_perm to <'
	if {[llength [$t services]]} {'>start_<"[[lindex [$t services] 0] name]">_1<'
	} else {'>idle<'}'>
	eager
	resume
	
<'}

set index 1
foreach s [$t services] {'>
	
	
	on begin_<"[$s name]">_1
	from start_<"[$s name]">_1 to end_<"[$s name]">_1
	eager
	
	on cycle_<"[$s name]">_1
	from end_<"[$s name]">_1 to start_<"[$s name]">_2
	resume
	
	on begin_<"[$s name]">_2
	from start_<"[$s name]">_2 to end_<"[$s name]">_2
	eager
	
	on cycle_<"[$s name]">_2
	from end_<"[$s name]">_2 to <'
	if {$index < [llength [$t services]]} {'>start_<"[[lindex [$t services] $index] name]">_1<'
	} else {'>finish<'}'>

	resume
	
	on skip_<"[$s name]">_1
	from start_<"[$s name]">_1 to start_<"[$s name]">_2
	eager
	resume
	
	on skip_<"[$s name]">_2
	from start_<"[$s name]">_2 to <'
	if {$index < [llength [$t services]]} {'>start_<"[[lindex [$t services] $index] name]">_1<'
	} else {'>finish<'}'>

	eager
	resume
	
<'
incr index}

foreach n $nostop {'>
	
	on inter_1_<"$n">_1
	from start_<"$n">_1 to interrupt_<"$n">_1
	eager
	resume
	
	on inter_2_<"$n">_1
	from interrupt_<"$n">_1 to start_<"$n">_2
	eager
	resume
	
	on inter_1_<"$n">_2
	from start_<"$n">_2 to interrupt_<"$n">_2
	eager
	resume
	
	on inter_2_<"$n">_2
	from interrupt_<"$n">_2 to <'
	set index 1
	foreach s [$t services] {
		if {[$s name] == $n} {break}
		incr index}
		if {$index < [llength [$t services]]} {'>start_<"[[lindex [$t services] $index] name]">_1<'
		} else {'>finish<'}'>
	
	eager
	resume
		
<'}

if {[llength [$t services]]} {'>
	
	on skipall
	from start_<"[[lindex [$t services] 0] name]">_1 to idle
	eager
	resume
<'}'>

	on ended
	from finish to idle
	eager
	resume
	
end
		
	
	
<'foreach s [$t services] {'>
	
/* service <"[$s name]"> of <"[$t name]"> */
	
atom type SERVICE_<"[$s name]">_<"[$component name]">()
	clock c unit millisecond
	export port Port ended()
	export port Port begin()<'
	foreach st [$s fsm] {'>
		
	port Port to_<"[$st name]">()<"\n"><'}
	
	set test [list]
	foreach c [$s codels] {
		if {[llength [$c mutex]] || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]])} {'>
	export port Port lock_res_<"[$c name]">()<"\n\t">export port Port unlock_res_<"[$c name]">()<"\n"><'}
		foreach tr [$c triggers] {
	
	if {!([llength [$c mutex]] || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]]))} {'>
	port Port to_<"[$tr name]">_test()<"\n"><'}}
		foreach y [$c yields] {
			if {[$y kind] == "pause event"} {
				if {[$y name] in $test} {continue}
				lappend test [$y name]}}}
	if {[llength $test]} {'>
	
	export port Port resume_()<"\n"><'
	foreach te $test {'>
	export port Port to_pause_<"$te">()<"\n"><'}}'>
	
	export port Port interrupted()// all can be interrupted by abort/kill anyway<"\n">
	
	
	
	place idle<'
	foreach c [$s codels] {
		foreach tr [$c triggers] {'>, <"[$tr name]">_, <"[$tr name]">_test<'
		if {[llength [$c mutex]] || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]])} {'>, <"[$tr name]">_2<'}}}
	foreach te $test {'>, pause_<"$te"><'}'>
		

	initial to idle
	
	on begin
	from idle to start_
	do {c = 0;}
	resume
	
<'if {[$s name] in $nostop} {'>
	
	on interrupted
	from idle to idle
	
<'} else {'>
	
	on interrupted
	from idle to stop_
	do {c = 0;}
	resume
	
	
<'}

foreach c [$s codels] {
		foreach tr [$c triggers] {
		if {[llength [$c mutex]] || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]])} {'>
				
	on lock_res_<"[$c name]">
	from <"[$tr name]">_ to <"[$tr name]">_2
	eager
	do {c = 0;}
	resume
	
	on unlock_res_<"[$c name]"> // call codel
	from <"[$tr name]">_2 to <"[$tr name]">_test
	eager
	resume<'
	if {![catch {$c wcet}]} {'> (c <= <"[expr [[$c wcet] value]*1000]">)<'}'>
	
	
<'} else {'>
		
	on to_<"[$tr name]">_test // call codel
	from <"[$tr name]">_ to <"[$tr name]">_test
	eager
	resume<'
	if {![catch {$c wcet}]} {'> (c <= <"[expr [[$c wcet] value]*1000]">)<'}'>
	
	
<'}
	
	foreach y [$c yields] {
		
	if {[$y kind] == "pause event"} {'>
		
	on to_pause_<"[$y name]">
	from <"[$tr name]">_test to pause_<"[$y name]">
	eager
	resume

	
	on resume_
	from pause_<"[$y name]"> to <"[$y name]">_
	do {c = 0;}
	resume
	
	
<'if {[$s name] in $nostop} {'>
	
	on interrupted
	from pause_<"[$y name]"> to idle
	
<'} else {'>
	
	on interrupted
	from pause_<"[$y name]"> to stop_
	do {c = 0;}
	resume
	
<'}
	
} else {
	
	if {[$y name]=="ether"} {'>
		
	on ended
	from <"[$tr name]">_test to idle
	eager
	resume

<'} else {'>
	
	on to_<"[$y name]">
	from <"[$tr name]">_test to <"[$y name]">_
	eager
	do {c = 0;}
	resume <'}}}}}'>

	
end	
	
<'}
incr counter}'>

/* manage spawning of different tasks */

atom type READY_<"[$component name]">()
	clock c unit millisecond
	export port Port block()
	export port Port allow()

	place start, finish

	initial to <'
	if {[llength $permanent]} {'>start<'
	} else {'>finish<'}'>

	on block
	from start to finish
	eager
	resume

	on allow
	from finish to finish
	resume

end


/* module */

compound type <"[$component name]">()

	component CLIENT_<"[$component name]"> client_<"[$component name]">()
	component CONTROL_<"[$component name]"> control_<"[$component name]">()
	component READY_<"[$component name]"> ready_<"[$component name]">()
	component SIGNAL <'
foreach t [$component tasks] {'>signal_<"[$t name]">_<"[$component name]">()<'
	if {$t != [lindex [$component tasks] end]} {'>, <'}}
foreach t [$component tasks] {'>
	
	
	component MANAGER_<"[$t name]">_<"[$component name]"> manager_<"[$t name]">_<"[$component name]">()
	component TIMER_<"[$t name]">_<"[$component name]"> timer_<"[$t name]">_<"[$component name]">()
	
<'if {[llength [$t codels]]} {'>		
	component PERM_<"[$t name]">_<"[$component name]"> perm_<"[$t name]">_<"[$component name]">()<'}
foreach s [$t services] {'>
	
	component SERVICE_<"[$s name]">_<"[$component name]"> <"[$s name]">_<"[$component name]">_inst_1(), <"[$s name]">_<"[$component name]">_inst_2()<'}}'>

	
	component STATUS <'
	foreach a $activities {'><"[$a name]">_<"[$component name]">_status_1(), <"[$a name]">_<"[$component name]">_status_2()<'
		if {$a != [lindex $activities end]} {'>, <'}}
		
		set mutex($gcounter) [list]
		foreach r $requests {
				foreach c [$r validate] {
					if {[llength [$c mutex]] || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]])} {
						lappend mutex($gcounter) [join [list "val" [$r name] "lk" [$component name]] _]}}
				if {[$r kind] == "activity"} {
				foreach c [$r codels] {
					if {[llength [$c mutex]]  || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]])} {
						lappend mutex($gcounter) [join [list [$c name] [$r name] "lk" [$component name]] _]}}
					} else {
					if {[llength [$r mutex]]  || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $r]])} {
						lappend mutex($gcounter) [join [list  [$r kind] [$r name] "lk" [$component name]] _]
					} else {
					foreach c [$r codels] {
					if {[llength [$c mutex]] || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]])} {
						lappend mutex($gcounter) [join [list  [$r kind] [$r name] "lk" [$component name]] _]
						break}}}
				}}
		foreach t [$component tasks] {
			foreach c [$t codels] {
				if {[llength [$c mutex]]} {lappend mutex($gcounter) [join [list [$c name] [$t name] "lk" [$component name]] _]}}}
				
		if {[llength $mutex($gcounter)]} {'>		
			
		component LOCK <'
			foreach m $mutex($gcounter) {'><"$m">()<'
				if {$m != [lindex $mutex($gcounter) end]} {'>, <'}}}'>
				
			
/* connectors */

/* restart control task */

	connector sync2 restart_<"[$component name]">(control_<"[$component name]">.finished, client_<"[$component name]">.finished)
	
/* spawn control task */

	connector sync2 spawn_cntrl_task_<"[$component name]">(control_<"[$component name]">.spawn, ready_<"[$component name]">.allow)
	
/* empty box but at least one service ended */

	connector sync2 noreq_<"[$component name]">(control_<"[$component name]">.norequest, client_<"[$component name]">.norequest)
	
/* clear all (and send final replies) */

	connector trig<"[expr [llength $activities]*2+1]"> clearall_<"[$component name]">(control_<"[$component name]">.clear<'
	foreach a $activities {'>, <"[$a name]">_<"[$component name]">_status_1.clear, <"[$a name]">_<"[$component name]">_status_2.clear<'}'>)
	
<'
if {[llength $permanent]} {'>
	
/* spawn execution tasks */

<'set index 0
foreach p $permanent {'>
	
/* <"[$p name]"> */

<'foreach c [$p codels] {
	foreach tr [$c triggers] {
		if {[$tr name] == "start"} {
			foreach y [$c yields] {'>
		
	connector trig2 spawn_<"[$p name]">_<"[$component name]">(perm_<"[$p name]">_<"[$component name]">.to_<'
	if {[$y kind] == "pause event"} {'>pause_<'}'><"[$y name]">, <'
	if {$p == [lindex $permanent end]} {'>ready_<"[$component name]">.block<'
	} else {'>perm_<"[[lindex $permanent [expr $index+1]] name]">_<"[$component name]">.begin<'}'>)

<'}
break}}
break}
incr index}}

set counter 0
foreach t [$component tasks] {
'>
	
/* signal end of activities (or not) */
	
	connector trig2 end_<"[$t name]">_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.ended, signal_<"[$t name]">_<"[$component name]">.hold)
	
/* report end of activities */

	connector sync2 report_<"[$t name]">_<"[$component name]">(control_<"[$component name]">.immediate, signal_<"[$t name]">_<"[$component name]">.endsig)
	
<'if {![catch {$t period}]} {'>

/* period signal */

	connector sync2 tick_<"[$t name]">_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.tick, timer_<"[$t name]">_<"[$component name]">.tick)
	
<'}

if {[llength [$t codels]]} {'>
	
/* end permanent activity (asynchrone for first execution) */

	connector trig2 end_perm_act_<"[$t name]">_<"[$component name]">(perm_<"[$t name]">_<"[$component name]">.to_ether, manager_<"[$t name]">_<"[$component name]">.cycle_perm)
	
/* skip permanent activity (when ended) */

	connector sync2 skip_perm_act_<"[$t name]">_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.skip_perm, perm_<"[$t name]">_<"[$component name]">.void)
	
<'if {[llength $resumeperm($counter)]} {'>
	
/* resume permanent activity */

	connector sync2 resume_perm_act_<"[$t name]">_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.begin_perm, perm_<"[$t name]">_<"[$component name]">.resume_)
	
/* pause permanent activity (asynchrone for first execution) */

<'foreach y $resumepermstart($counter) {'>
					
	connector trig2 pause_perm_act_<"$y">_<"[$t name]">_<"[$component name]">(perm_<"[$t name]">_<"[$component name]">.to_pause_<"$y">, manager_<"[$t name]">_<"[$component name]">.cycle_perm)

<'}	
foreach y $resumeperm($counter) {
	if {!($y in $resumepermstart($counter))} {'>
		
	connector sync2 pause_perm_act_<"$y">_<"[$t name]">_<"[$component name]">(perm_<"[$t name]">_<"[$component name]">.to_pause_<"$y">, manager_<"[$t name]">_<"[$component name]">.cycle_perm)
	
<'}}}}

	
	if {[llength [$t services]]} {'>
	
/* no active instance */

	connector sync<"[expr [llength [$t services]]*$max+1]"> skip_all_<"[$t name]">_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.skipall<'
	foreach s [$t services] {'>, <"[$s name]">_<"[$component name]">_status_1.void, <"[$s name]">_<"[$component name]">_status_2.void<'}'>)
	
<'foreach s [$t services] {
	set test [list]
	foreach c [$s codels] {
		foreach y [$c yields] {
		if {[$y kind] == "pause event"} {
			if {[$y name] in $test} {continue}
				lappend test [$y name]}}}
	if {[llength $test]} {'>
		
/* resume (paused last cycle) */

	connector sync3 resume_<"[$s name]">_1_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.begin_<"[$s name]">_1, <"[$s name]">_<"[$component name]">_inst_1.resume_, <"[$s name]">_<"[$component name]">_status_1.run)
	connector sync3 resume_<"[$s name]">_2_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.begin_<"[$s name]">_2, <"[$s name]">_<"[$component name]">_inst_2.resume_, <"[$s name]">_<"[$component name]">_status_2.run)

<'	foreach te $test {'>
	
/* pause events */

	connector sync2 pause_<"$te">_<"[$s name]">_1_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.cycle_<"[$s name]">_1, <"[$s name]">_<"[$component name]">_inst_1.to_pause_<"$te">)
	connector sync2 pause_<"$te">_<"[$s name]">_2_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.cycle_<"[$s name]">_2, <"[$s name]">_<"[$component name]">_inst_2.to_pause_<"$te">)
	
<'}}'>
	
/* inactive instances */

	connector sync2 jump_<"[$s name]">_1_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.skip_<"[$s name]">_1, <"[$s name]">_<"[$component name]">_status_1.void)
	connector sync2 jump_<"[$s name]">_2_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.skip_<"[$s name]">_2, <"[$s name]">_<"[$component name]">_status_2.void)
	
/* <"[$s name]"> instance 1 & 2*/

/* nominal execution */

	connector sync3 nominal_<"[$s name]">_1_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.begin_<"[$s name]">_1, <"[$s name]">_<"[$component name]">_inst_1.begin, <"[$s name]">_<"[$component name]">_status_1.run)
	connector sync3 nominal_<"[$s name]">_2_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.begin_<"[$s name]">_2, <"[$s name]">_<"[$component name]">_inst_2.begin, <"[$s name]">_<"[$component name]">_status_2.run)
	
/* end */

	connector sync4 finished_<"[$s name]">_1_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.cycle_<"[$s name]">_1, <"[$s name]">_<"[$component name]">_inst_1.ended, <"[$s name]">_<"[$component name]">_status_1.finished, signal_<"[$t name]">_<"[$component name]">.sig)
	connector sync4 finished_<"[$s name]">_2_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.cycle_<"[$s name]">_2, <"[$s name]">_<"[$component name]">_inst_2.ended, <"[$s name]">_<"[$component name]">_status_2.finished, signal_<"[$t name]">_<"[$component name]">.sig)
	
/* interruption */


<'if {[$s name] in $nostopt($counter)} {'>
	
	connector sync2 interrupted_1_<"[$s name]">_1_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.inter_1_<"[$s name]">_1, <"[$s name]">_<"[$component name]">_status_1.inter)
	connector sync4 interrupted_2_<"[$s name]">_1_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.inter_2_<"[$s name]">_1, <"[$s name]">_<"[$component name]">_inst_1.interrupted, <"[$s name]">_<"[$component name]">_status_1.finished, signal_<"[$t name]">_<"[$component name]">.sig)
	
	connector sync2 interrupted_1_<"[$s name]">_2_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.inter_1_<"[$s name]">_2, <"[$s name]">_<"[$component name]">_status_2.inter)
	connector sync4 interrupted_2_<"[$s name]">_2_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.inter_2_<"[$s name]">_2, <"[$s name]">_<"[$component name]">_inst_2.interrupted, <"[$s name]">_<"[$component name]">_status_2.finished, signal_<"[$t name]">_<"[$component name]">.sig)   
	
<'} else {'>
	
	connector sync3 interrupted_<"[$s name]">_1_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.begin_<"[$s name]">_1, <"[$s name]">_<"[$component name]">_inst_1.interrupted, <"[$s name]">_<"[$component name]">_status_1.inter)	
	connector sync3 interrupted_<"[$s name]">_2_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.begin_<"[$s name]">_2, <"[$s name]">_<"[$component name]">_inst_2.interrupted, <"[$s name]">_<"[$component name]">_status_2.inter)
		
<'}}}
incr counter}

foreach a $activities {'>
	
/* receive requests */
	
	connector sync3 request_<"[$a name]">_1_<"[$component name]">(control_<"[$component name]">.req_<"[$a name]">, client_<"[$component name]">.req_<"[$a name]">, <"[$a name]">_<"[$component name]">_status_1.activ)
	connector sync3 request_<"[$a name]">_2_<"[$component name]">(control_<"[$component name]">.req_<"[$a name]">, client_<"[$component name]">.req_<"[$a name]">, <"[$a name]">_<"[$component name]">_status_2.activ)
	
/* activate */

	connector trig3 activate_<"[$a name]">_<"[$component name]">(control_<"[$component name]">.act_<"[$a name]">, <"[$a name]">_<"[$component name]">_status_1.hold, <"[$a name]">_<"[$component name]">_status_2.hold)
	
<'if {[llength [$a validate]]} {'>
	
/* deactivate */

	connector trig3 deactivate_<"[$a name]">_<"[$component name]">(control_<"[$component name]">.invalid_<"[$a name]">, <"[$a name]">_<"[$component name]">_status_1.deactiv, <"[$a name]">_<"[$component name]">_status_2.deactiv)
<'}
if {[llength [$a interrupts]]} {'>
	
/* interrupt incompatible activities */

	connector trig<"[expr [llength [$a interrupts]]*2+1]"> interr_<"[$a name]">_<"[$component name]">(control_<"[$component name]">.inter_<"[$a name]"><'
	foreach i [$a interrupts] {'>, <"[$i name]">_<"[$component name]">_status_1.interrupt, <"[$i name]">_<"[$component name]">_status_2.interrupt<'}'>)

<'}'>

/* launch pending instances */

	connector sync<'
	if {$a in [$a interrupts]} {'><"[expr [llength [$a interrupts]]*2+1]"><'
	} else {'><"[expr [llength [$a interrupts]]*2+2]"><'}'> launch_<"[$a name]">_1_<"[$component name]">(control_<"[$component name]">.launch, <"[$a name]">_<"[$component name]">_status_1.launch<'
	if {$a in [$a interrupts]} {'>, <"[$a name]">_<"[$component name]">_status_2.void<'}
	foreach i [$a interrupts] {
		if {$i != $a} {'>, <"[$i name]">_<"[$component name]">_status_1.void, <"[$i name]">_<"[$component name]">_status_2.void<'}}'>)
		
	connector sync<'
	if {$a in [$a interrupts]} {'><"[expr [llength [$a interrupts]]*2+1]"><'
	} else {'><"[expr [llength [$a interrupts]]*2+2]"><'}'> launch_<"[$a name]">_2_<"[$component name]">(control_<"[$component name]">.launch, <"[$a name]">_<"[$component name]">_status_2.launch<'
	if {$a in [$a interrupts]} {'>, <"[$a name]">_<"[$component name]">_status_1.void<'}
	foreach i [$a interrupts] {
		if {$i != $a} {'>, <"[$i name]">_<"[$component name]">_status_1.void, <"[$i name]">_<"[$component name]">_status_2.void<'}}'>)
		

<'}'>

/* non activities */

<'foreach n $nonactivities {'>
	
/* receive request */
	
	connector sync2 request_<"[$n name]">_<"[$component name]">(control_<"[$component name]">.req_<"[$n name]">, client_<"[$component name]">.req_<"[$n name]">)
	
<'if {[llength [$n interrupts]]} {'>
	
	connector trig<"[expr [llength [$n interrupts]]*2+1]"> inter_inc_<"[$n name]">_<"[$component name]">(control_<"[$component name]">.inter_<"[$n name]"><'
	foreach i [$n interrupts] {'>, <"[$i name]">_<"[$component name]">_status_1.interrupt, <"[$i name]">_<"[$component name]">_status_2.interrupt<'}'>)<'}'>

<'}

if {[llength $mutex($gcounter)]} {'>
	
/* mutual exclusion connectors */<'}

set exports [list]
foreach t [$component tasks] {
	foreach c [$t codels] {
		if {[llength [$c mutex]] || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]])} {'>
			
/* <"[$t name]"> permanent activity. codel <"[$c name]"> */


	connector sync<"[expr [llength [$c mutex]]+2]"><'
	if {[llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]]} {
	lappend exports [join [list "take" "res" [$c name] [$t name] [$component name]] _]'>_exp<'}'> take_res_<"[$c name]">_<"[$t name]">_<"[$component name]">(perm_<"[$t name]">_<"[$component name]">.lock_res_<"[$c name]">, <"[$c name]">_<"[$t name]">_lk_<"[$component name]">.take<'
	foreach m [$c mutex] {
		if {![catch {$m service}]} {
			if {$m == [[$m service] validate]} {'>, val<'
			} else {
				if {[[$m service] kind] == "activity"} {'>, <"[$m name]"><'
					} else {'>, <"[[$m service] kind]"><'}}'>_<"[[$m service] name]">_lk<'
		} else {
			if {![catch {$m task}]} {'>, <"[$m name]">_<"[[$m task] name]">_lk<'
			} else {'>, <"[$m kind]">_<"[$m name]">_lk<'}}'>_<"[$component name]">.check<'}'>)
	
	connector sync2<'
	if {[llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]]} {
	lappend exports [join [list "give" "res" [$c name] [$t name] [$component name]] _]'>_exp<'}'> give_res_<"[$c name]">_<"[$t name]">_<"[$component name]">(perm_<"[$t name]">_<"[$component name]">.unlock_res_<"[$c name]">, <"[$c name]">_<"[$t name]">_lk_<"[$component name]">.give)

<'}}}

foreach r $requests {
		foreach v [$r validate] {
			if {[llength [$v mutex]] || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $v]])} {'>
				
/* <"[$r name]"> validation codel */
				
	
	connector sync<"[expr [llength [$v mutex]]+2]"><'
	if {[llength [dotgen component]]>1 && [llength [mutex-ports dotgen $v]]} {
	lappend exports [join [list "take" "res" [$v name] [$r name] [$component name]] _]'>_exp<'}'> take_res_<"[$v name]">_<"[$r name]">_<"[$component name]">(control_<"[$component name]">.lock_res_<"[$r name]">_validate, val_<"[$r name]">_lk_<"[$component name]">.take<'
	foreach m [$v mutex] {
		if {![catch {$m service}]} {
			if {$m == [[$m service] validate]} {'>, val<'
			} else {
				if {[[$m service] kind] == "activity"} {'>, <"[$m name]"><'
					} else {'>, <"[[$m service] kind]"><'}}'>_<"[[$m service] name]">_lk<'
		} else {
			if {![catch {$m task}]} {'>, <"[$m name]">_<"[[$m task] name]">_lk<'
			} else {'>, <"[$m kind]">_<"[$m name]">_lk<'}}'>_<"[$component name]">.check<'}'>)
		
	connector sync2<'
	if {[llength [dotgen component]]>1 && [llength [mutex-ports dotgen $v]]} {
	lappend exports [join [list "give" "res" [$v name] [$r name] [$component name]] _]'>_exp<'}'> give_res_<"[$v name]">_<"[$r name]">_<"[$component name]">(control_<"[$component name]">.unlock_res_<"[$r name]">_validate, val_<"[$r name]">_lk_<"[$component name]">.give)
		
<'}}
if {[$r kind] == "activity"} {
foreach c [$r codels] {
		if {[llength [$c mutex]] || ([llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]])} {'>
			
/* codel <"[$c name]"> service <"[$r name]"> */
	
	
	connector sync<"[expr [llength [$c mutex]]+2]"><'
	if {[llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]]} {
	lappend exports [join [list "take" "res" [$c name] [$r name] "1" [$component name]] _]'>_exp<'}'> take_res_<"[$c name]">_<"[$r name]">_1_<"[$component name]">(<"[$r name]">_<"[$component name]">_inst_1.lock_res_<"[$c name]">, <"[$c name]">_<"[$r name]">_lk_<"[$component name]">.take<'
	foreach m [$c mutex] {
		if {![catch {$m service}]} {
			if {$m == [[$m service] validate]} {'>, val<'
			} else {
				if {[[$m service] kind] == "activity"} {'>, <"[$m name]"><'
					} else {'>, <"[[$m service] kind]"><'}}'>_<"[[$m service] name]">_lk<'
		} else {
			if {![catch {$m task}]} {'>, <"[$m name]">_<"[[$m task] name]">_lk<'
			} else {'>, <"[$m kind]">_<"[$m name]">_lk<'}}'>_<"[$component name]">.check<'}'>)
		
	connector sync<"[expr [llength [$c mutex]]+2]"><'
	if {[llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]]} {
	lappend exports [join [list "take" "res" [$c name] [$r name] "2" [$component name]] _]'>_exp<'}'> take_res_<"[$c name]">_<"[$r name]">_2_<"[$component name]">(<"[$r name]">_<"[$component name]">_inst_2.lock_res_<"[$c name]">, <"[$c name]">_<"[$r name]">_lk_<"[$component name]">.take<'
	foreach m [$c mutex] {
		if {![catch {$m service}]} {
			if {$m == [[$m service] validate]} {'>, val<'
			} else {
				if {[[$m service] kind] == "activity"} {'>, <"[$m name]"><'
					} else {'>, <"[[$m service] kind]"><'}}'>_<"[[$m service] name]">_lk<'
		} else {
			if {![catch {$m task}]} {'>, <"[$m name]">_<"[[$m task] name]">_lk<'
			} else {'>, <"[$m kind]">_<"[$m name]">_lk<'}}'>_<"[$component name]">.check<'}'>)
		
	connector sync2<'
	if {[llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]]} {
	lappend exports [join [list "give" "res" [$c name] [$r name] "1" [$component name]] _]'>_exp<'}'> give_res_<"[$c name]">_<"[$r name]">_1_<"[$component name]">(<"[$r name]">_<"[$component name]">_inst_1.unlock_res_<"[$c name]">, <"[$c name]">_<"[$r name]">_lk_<"[$component name]">.give)
		
	connector sync2<'
	if {[llength [dotgen component]]>1 && [llength [mutex-ports dotgen $c]]} {
	lappend exports [join [list "give" "res" [$c name] [$r name] "2" [$component name]] _]'>_exp<'}'> give_res_<"[$c name]">_<"[$r name]">_2_<"[$component name]">(<"[$r name]">_<"[$component name]">_inst_2.unlock_res_<"[$c name]">, <"[$c name]">_<"[$r name]">_lk_<"[$component name]">.give)
		
<'}}
} else {'>
	
/* <"[$r name]"> */

<'	set test [join [list  [$r kind] [$r name] "lk" [$component name]] _]
	if {$test in $mutex($gcounter)} {
		
		set mutport [mutex-ports dotgen $r]
		set mutexx [$r mutex]
		set mutex2 [list]
	foreach c [$r codels] {
		if {[llength [mutex-ports dotgen $c]]} {
		lappend mutport [mutex-ports dotgen $c]]}
		if {[llength [$c mutex]]} { 
		set mutex2 [concat $mutex2 [$c mutex]]}}

		set mutexx [concat $mutexx $mutex2]
		set mutexx [lsort -unique $mutexx]'>
	
	connector sync<"[expr [llength $mutexx]+2]"><'
	if {[llength [dotgen component]]>1 && [llength $mutport]} {
	lappend exports [join [list "take" "res" [$r kind] [$r name] [$component name]] _]'>_exp<'}'> take_res_<"[$r kind]">_<"[$r name]">_<"[$component name]">(control_<"[$component name]">.lock_res_<"[$r name]">, <"[$r kind]">_<"[$r name]">_lk_<"[$component name]">.take<'
	foreach m $mutexx {
		if {![catch {$m service}]} {
			if {$m == [[$m service] validate]} {'>, val<'
			} else {
				if {[[$m service] kind] == "activity"} {'>, <"[$m name]"><'
					} else {'>, <"[[$m service] kind]"><'}}'>_<"[[$m service] name]">_lk<'
		} else {
			if {![catch {$m task}]} {'>, <"[$m name]">_<"[[$m task] name]">_lk<'
			} else {'>, <"[$m kind]">_<"[$m name]">_lk<'}}'>_<"[$component name]">.check<'}'>)
		
	connector sync2<'
	if {[llength [dotgen component]]>1 && [llength $mutport]} {
	lappend exports [join [list "give" "res" [$r kind] [$r name] [$component name]] _]'>_exp<'}'> give_res_<"[$r kind]">_<"[$r name]">_<"[$component name]">(control_<"[$component name]">.unlock_res_<"[$r name]">, <"[$r kind]">_<"[$r name]">_lk_<"[$component name]">.give)
	
<'}}}'>


/* priorities */

<'foreach t [$component tasks] {
	if {[llength [$t codels]]} {'>
		
/* spawn */

	priority spawn_pr_<"[$t name]">_<"[$component name]"> end_perm_act_<"[$t name]">_<"[$component name]">:* < spawn_<"[$t name]">_<"[$component name]">:*
	
<'}

	if {[llength [$t services]]} {'>
		
/* skip all instances if possible */
		
	priority no_active_inst_<"[$t name]">_<"[$component name]"> jump_<"[[lindex [$t services] 0] name]">_1_<"[$component name]">:* < skip_all_<"[$t name]">_<"[$component name]">:* 
	
<'}

foreach r [$t services] {'>
	
/* report end of activities  immediately */

	priority report_first_<"[$r name]">_<"[$t name]">_<"[$component name]"> request_<"[$r name]"><'
if {[$r kind] == "activity"} {'>_1<'}'>_<"[$component name]">:* < report_<"[$t name]">_<"[$component name]">:*

<'if {[$r kind] == "activity"} {'>

	priority report_first_<"[$r name]">_<"[$t name]">_2_<"[$component name]"> request_<"[$r name]">_2_<"[$component name]">:* < report_<"[$t name]">_<"[$component name]">:*
	
<'}}}

foreach a $activities {'>
	
/* launch activity before restarting control task */

	priority launch_<"[$a name]">_first_<"[$component name]"> restart_<"[$component name]">:* < launch_<"[$a name]">_1_<"[$component name]">:*
	priority launch_<"[$a name]">_2_first_<"[$component name]"> restart_<"[$component name]">:* < launch_<"[$a name]">_2_<"[$component name]">:*
	
<'}'>

<'foreach e $exports {'>
	
	export port <"$e">.exp as <"$e">_
	
<'}'>

end<'

incr gcounter}

if {[llength [dotgen components]]>1} {'>
	
/* all modules */
	
compound type <'
foreach component [dotgen components] {'><"[$component name]"><'
	if {$component != [lindex [dotgen components] end]} {'>_<'}}'>()
	
<'foreach component [dotgen components] {'>
	
	component <"[$component name]"> <"[$component name]">_()
<'}'>

/* ports */

<'foreach p [ports-names dotgen] {'>
	
	component LOCK <"$p">()
<'}'>

/* mutual exclusion between modules*/

<'set gcounter 0

foreach comp [dotgen component] {
	foreach t [$comp tasks] {
		foreach c [$t codels] {
			set test [join [list [$c name] [$t name] "lk" [$comp name]] _]
			if {$test in $mutex($gcounter) && [llength [mutex-ports dotgen $c]]} {'>
				
		connector sync<"[expr [llength [mutex-ports dotgen $c]]+1]"> take_res_<"[$c name]">_<"[$t name]">_<"[$comp name]">_ports(<"[$comp name]">_.take_res_<"[$c name]">_<"[$t name]">_<"[$comp name]">_<'
		foreach p [mutex-ports dotgen $c] {'>, <"[lindex $p 2]">_<"[lindex $p 3]">.take<'}'>)
		
		connector sync<"[expr [llength [mutex-ports dotgen $c]]+1]]"> give_res_<"[$c name]">_<"[$t name]">_<"[$comp name]">_ports(<"[$comp name]">_.give_res_<"[$c name]">_<"[$t name]">_<"[$comp name]">_<'
		foreach p [mutex-ports dotgen $c] {'>, <"[lindex $p 2]">_<"[lindex $p 3]">.give<'}'>)

<'}}
	foreach s [$t services] {
		foreach c [$s codels] {
				set test [join [list [$c name] [$s name] "lk" [$comp name]] _]
				if {$test in $mutex($gcounter) && [llength [mutex-ports dotgen $c]]} {'>
		connector sync<"[expr [llength [mutex-ports dotgen $c]]+1]"> take_res_<"[$c name]">_<"[$s name]">_<"[$comp name]">_ports_1(<"[$comp name]">_.take_res_<"[$c name]">_<"[$s name]">_1_<"[$comp name]">_<'
		foreach p [mutex-ports dotgen $c] {'>, <"[lindex $p 2]">_<"[lindex $p 3]">.take<'}'>)	
		
		connector sync<"[expr [llength [mutex-ports dotgen $c]]+1]"> take_res_<"[$c name]">_<"[$s name]">_<"[$comp name]">_ports_2(<"[$comp name]">_.take_res_<"[$c name]">_<"[$s name]">_2_<"[$comp name]">_<'
		foreach p [mutex-ports dotgen $c] {'>, <"[lindex $p 2]">_<"[lindex $p 3]">.take<'}'>)	
		
		connector sync<"[expr [llength [mutex-ports dotgen $c]]+1]"> give_res_<"[$c name]">_<"[$s name]">_<"[$comp name]">_ports_1(<"[$comp name]">_.give_res_<"[$c name]">_<"[$s name]">_1_<"[$comp name]">_<'
		foreach p [mutex-ports dotgen $c] {'>, <"[lindex $p 2]">_<"[lindex $p 3]">.give<'}'>)	
		
		connector sync<"[expr [llength [mutex-ports dotgen $c]]+1]"> give_res_<"[$c name]">_<"[$s name]">_<"[$comp name]">_ports_2(<"[$comp name]">_.give_res_<"[$c name]">_<"[$s name]">_2_<"[$comp name]">_<'
		foreach p [mutex-ports dotgen $c] {'>, <"[lindex $p 2]">_<"[lindex $p 3]">.give<'}'>)
		
<'}}}}

foreach s [$comp services] {
	if {[$s kind] != "activity"} {
		set test [join [list  [$r kind] [$r name] "lk" [$comp name]] _]
		if {$test in $mutex($gcounter)} {
			set mutport [mutex-ports dotgen $s]
			set mutport2 [list]
			foreach c [$s codels] {
				if {[llength [mutex-ports dotgen $c]]} {
					lappend mutport2 [mutex-ports dotgen $c]}}
			set mutport [concat $mutport $mutport2]
			set mutport [lsort -unique $mutport]
			if {[llength $mutport]} {'>
		
		connector sync<"[expr [[llength $mutport]]+1]"> take_res_<"[$s name]">_<"[$comp name]">_ports(<"[$comp name]">_.take_res_<"[$s kind]">_<"[$s name]">_<"[$comp name]">_<'
		foreach p $mutport {'>, <"[lindex $p 2]">_<"[lindex $p 3]">.take<'}'>)	
		
		connector sync<"[expr [[llength $mutport]]+1]"> give_res_<"[$s name]">_<"[$comp name]">_ports(<"[$comp name]">_.give_res_<"[$s kind]">_<"[$s name]">_<"[$comp name]">_<'
		foreach p $mutport {'>, <"[lindex $p 2]">_<"[lindex $p 3]">.give<'}'>)
		
<'}}}}
incr gcounter}'>

end

<'}'>


end



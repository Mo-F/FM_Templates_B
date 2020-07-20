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

		

/* This model is automatically generated - Mohammed Foughali 2016 */

	
/* for verification purposes */

package <'
foreach component [dotgen components] {'><"[$component name]"><'
	if {$component != [lindex [dotgen components] end]} {'>_<'}}'>

<'
set counter 0
foreach component [dotgen components] {
foreach t [$component tasks] {
set pauseevents($counter) 0
foreach s [$t services] {
	set check 0
	foreach c [$s codels] {
		foreach y [$c yields] {
			if {[$y kind] == "pause event"} {
				incr pauseevents($counter)
				set check 1
				break}
		}
		if {$check} {break}
	}
}
incr counter}}
'>
	

/* definition of port types */

port type Port() <"\n">
/* definition of connector types types */<"\n"><'
set max 2
set synclengths {2 3 4}
set triglengths {2 3}
set counter 0
foreach component [dotgen components] {
foreach t [$component tasks] {
if {$pauseevents($counter) > 1} {
lappend triglengths [expr $pauseevents($counter)*2+1]
}
incr counter}}
set triglengths [lsort -unique $triglengths]
foreach component [dotgen components] {
set instpertask [list]
set interperservice [list]

set lockspercodel [list]

set request [list]
set activity [list]
set nonactivity [list]

foreach t [$component tasks] {
lappend instpertask [expr [llength [$t services]]*$max]}

foreach s [$component services] {
if {[$s name]!="abort_activity" && [$s name]!="kill" && [$s name]!="connect_port" && [$s name]!="connect_service"} {
	lappend request $s
	if {[$s kind] == "activity"} {lappend activity $s
	} else {lappend nonactivity $s}
	if {$s in [$s interrupts]} {lappend interperservice [expr [llength [$s interrupts]]*$max-1]
	} else {lappend interperservice [expr [llength [$s interrupts]]*$max]}
	foreach c [$s validate] {
	if {[llength [$c mutex]]} {
	lappend lockspercodel [llength [$c mutex]]}}
	if {[$s kind]=="activity"} {
	foreach c [$s codels] {
	if {[llength [$c mutex]] || [llength [mutex-ports-basic dotgen $c]]} {
	lappend lockspercodel [expr [llength [$c mutex]] + [llength [mutex-ports-basic dotgen $c]]]}}
	} else {
	set conflicts [$s mutex]
	foreach c [$s codels] {
		foreach mu [$c mutex] {lappend conflicts $mu]}}
	if {[llength $conflicts]} {
	lappend lockspercodel [llength $conflicts]}}
	}
}

foreach t [$component tasks] {
	foreach c [$t codels] {
		if {[llength [$c mutex]] || [llength [mutex-ports-basic dotgen $c]]} {lappend lockspercodel [expr [llength [$c mutex]] + [llength [mutex-ports-basic dotgen $c]]]}}}

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

/* synchrones */<"\n">connector type singleton (Port p)<"\n">define p<"\n">end<"\n"><'

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


/* manage activities termination */

atom type SIGNAL()

	export port Port sig()
	export port Port endsig()
	export port Port hold()
	export port Port nosched()

	place start, intermediate, notify

	initial to start
	
	on nosched
	from start to start

	on sig
	from start to intermediate
	
	on nosched
	from intermediate to intermediate

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
	export port Port not_ready()
	export port Port ready()
    export port Port interrupt()
    export port Port clear()
    export port Port void()
    export port Port run()
    export port Port inter()
    export port Port finished()

place idle, activate, start, running, stopp, suspended, ether

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
	
	on not_ready
	from running to suspended
	
	on ready
	from suspended to running


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
	set requests($gcounter) [list]
	set activities($gcounter) [list]
	set nonactivities($gcounter) [list]
	foreach s [$component services] {
if {[$s name]!="abort_activity" && [$s name]!="kill" && [$s name]!="connect_port" && [$s name]!="connect_service"} {
	lappend requests($gcounter) $s
	if {[$s kind] == "activity"} {lappend activities($gcounter) $s
	} else {lappend nonactivities($gcounter) $s}
	
}}'>
	
/* component <"[$component name]"> types */ 

/* client */

atom type CLIENT_<"[$component name]">()
	export port Port norequest()
	export port Port finished()<"\n"><'
	foreach r $requests($gcounter) {'>
	export port Port req_<"[$r name]">()<"\n"><'}'>
	
	place idle, waiting
	
	initial to idle
	
	on norequest
	from idle to waiting
		
<'foreach r $requests($gcounter) {'>
	on req_<"[$r name]">
	from idle to waiting<"\n"><'}'>
	
	on finished
	from waiting to idle
	
end 

/* control task */

atom type CONTROL_<"[$component name]">()
	clock c
	export port Port spawn()
	export port Port check()
	export port Port norequest()
	export port Port immediate()
	export port Port clear()
	export port Port launch()
	export port Port finished()<"\n"><'
	foreach r $requests($gcounter) {'>
	export port Port req_<"[$r name]">()<"\n"><'

	if {[$r kind]=="activity"} {'>
	export port Port act_<"[$r name]">()<"\n"><'
	} else {
	set conflicts [$r mutex]
	foreach c [$r codels] {
		lappend conflicts [$c mutex]}
	if {[llength $conflicts]} {'>
	export port Port wait_<"[$r name]">()<"\n\t">export port Port lock_<"[$r name]">()<"\n\t">export port Port unlock_<"[$r name]">()<"\n"><'
	} else {'>
	port Port exec_<"[$r name]">()<"\n"><'}}
	if {[llength [$r validate]]} {'>
	port Port valid_<"[$r name]">()<"\n\t"><'if {[$r kind]=="activity"} {'>export<'}'> port Port invalid_<"[$r name]">()<"\n"><'
	if {[llength [[$r validate] mutex]]} {'>
	export port Port wait_<"[$r name]">_validate()<"\n\t">export port Port lock_<"[$r name]">_validate()<"\n\t">export port Port unlock_<"[$r name]">_validate()<"\n"><'
	} else {'>
	port Port <"[$r name]">_exe_validate()<"\n"><'}}
	if {[llength [$r interrupts]]} {'>
	export port Port inter_<"[$r name]">()<"\n"><'}}'>
	
	place unspawned, idle, ready, test, <'
	set invariants [list]
	foreach r $requests($gcounter) {
		
	# additional states (side effect to removing urgencies)	
		
	if {[$r kind]=="activity"} {lappend invariants [join [list [$r name] ""] _] 0'><"[$r name]">_, <'
	if {[llength [$r interrupts]]} {lappend invariants [join [list [$r name] "interrupt"] _] 0'><"[$r name]">_interrupt, <'} 
	} else {'><"[$r name]">_, <'
	set conflicts [$r mutex]
	foreach c [$r codels] {lappend conflicts [$c mutex]}
	if {[llength $conflicts]} {
		lappend invariants [join [list [$r name] "wait"] _] 0'><"[$r name]">_wait, <'
		lappend invariants [join [list [$r name] "2"] _]
		if {![catch {$r wcet}]} {lappend invariants [expr [[$r wcet] value]*1000]
			} else {lappend invariants 0}'><"[$r name]">_2, <'
		} else {lappend invariants [join [list [$r name] ""] _]
			if {![catch {$r wcet}]} {lappend invariants [expr [[$r wcet] value]*1000]
			} else {lappend invariants 0} }
	if {[llength [$r interrupts]]} {lappend invariants [join [list [$r name] "interrupt"] _] 0'><"[$r name]">_interrupt, <'}}
	if {[llength [$r validate]]} {lappend invariants [join [list [$r name] "validate" "2"] _]
		if {![catch {[$r validate] wcet}]} {lappend invariants [expr [[[$r validate] wcet] value]*1000]
			} else {lappend invariants 0}'><"[$r name]">_validate, <"[$r name]">_validate_2, <'
	if {[llength [[$r validate] mutex]]} { 
		lappend invariants [join [list [$r name] "validate" "wait"] _] 0'><"[$r name]">_validate_wait,<'
		lappend invariants [join [list [$r name] "validate" "3"] _] 0'><"[$r name]">_validate_3, <'
		} else {lappend invariants [join [list [$r name] "validate"] _] 0}}}'>free, release
	
	initial to unspawned
	
	
	on spawn
	from unspawned to idle
	
	on norequest
	from ready to free
	reset c
	
<'foreach r $requests($gcounter) {'>
	on req_<"[$r name]">
	from idle to <"[$r name]">_<'
	if {[llength [$r validate]]} {'>validate<'
	if {[llength [[$r validate] mutex]]} {'>_wait
	reset c
	
	on wait_<"[$r name]">_validate 
	from <"[$r name]">_validate_wait to <"[$r name]">_validate
<'}
	} else {
	if {[$r kind] != "activity"} {
	set conflicts [$r mutex]
	foreach c [$r codels] {lappend conflicts [$c mutex]}
	if {[llength $conflicts]} {'>wait
	reset c
	
	on wait_<"[$r name]">
	from <"[$r name]">_wait to <"[$r name]">_
<'}}}'>
	reset c

	on req_<"[$r name]">
	from ready to <"[$r name]">_<'
	if {[llength [$r validate]]} {'>validate<'
	if {[llength [[$r validate] mutex]]} {'>_wait
	
<'}
	} else {
	if {[$r kind] != "activity"} {
	set conflicts [$r mutex]
	foreach c [$r codels] {lappend conflicts [$c mutex]}
	if {[llength $conflicts]} {'>wait
	
<'}}}'>
	reset c
	
<'if {[llength [$r validate]]} {
	if {[llength [[$r validate] mutex]]} {'>
	on lock_<"[$r name]">_validate
	from <"[$r name]">_validate to <"[$r name]">_validate_2
	reset c
	
	on unlock_<"[$r name]">_validate
	from <"[$r name]">_validate_2 to <"[$r name]">_validate_3
	reset c

	
	on valid_<"[$r name]">
	from <"[$r name]">_validate_3 to <"[$r name]">_
<'if {[$r kind] != "activity"} {
	set conflicts [$r mutex] 
	foreach c [$r codels] {lappend conflicts [$c mutex]}
	if {[llength $conflicts]} {'>wait
	reset c
	
	on wait_<"[$r name]">
	from <"[$r name]">_wait to <"[$r name]">_
<'}}'>
	reset c

	on invalid_<"[$r name]">
	from <"[$r name]">_validate_3 to free
	reset c

	
<'} else {'>
	
	on <"[$r name]">_exe_validate
	from <"[$r name]">_validate to <"[$r name]">_validate_2
	reset c
	
	
	on valid_<"[$r name]">
	from <"[$r name]">_validate_2 to <"[$r name]">_
<'if {[$r kind] != "activity"} {
	set conflicts [$r mutex] 
	foreach c [$r codels] {lappend conflicts [$c mutex]}
	if {[llength $conflicts]} {'>wait
	reset c
	
	on wait_<"[$r name]">
	from <"[$r name]">_wait to <"[$r name]">_
<'}}'>
	reset c
	
	on invalid_<"[$r name]">
	from <"[$r name]">_validate_2 to free
	reset c
	
	
<'}}
if {[$r kind]=="activity"} {
	if {[llength [$r interrupts]]} {'>
	on inter_<"[$r name]">
	from <"[$r name]">_ to <"[$r name]">_interrupt
	reset c
	
	on act_<"[$r name]">
	from <"[$r name]">_interrupt to free
	reset c
	
	
<'} else {'>
	on act_<"[$r name]">
	from <"[$r name]">_ to free
	reset c

	
<'}
} else {
if {[llength [$r interrupts]]} {
	set conflicts [$r mutex] 
	foreach c [$r codels] {lappend conflicts [$c mutex]}
	if {[llength $conflicts]} {'>
	on lock_<"[$r name]">
	from <"[$r name]">_ to <"[$r name]">_2
	reset c
	
	on unlock_<"[$r name]">
	from <"[$r name]">_2 to <"[$r name]">_interrupt
	reset c
	
<'} else {'>
	on exec_<"[$r name]">
	from <"[$r name]">_ to <"[$r name]">_interrupt
	reset c

	
<'}'>

	on inter_<"[$r name]">
	from <"[$r name]">_interrupt to free
	reset c

	
<'} else {
	set conflicts [$r mutex] 
	foreach c [$r codels] {lappend conflicts [$c mutex]}
	if {[llength $conflicts]} {'>
	on lock_<"[$r name]">
	from <"[$r name]">_ to <"[$r name]">_2
	reset c
	
	on unlock_<"[$r name]">
	from <"[$r name]">_2 to free
	reset c
	
<'} else {'>
	on exec_<"[$r name]">
	from <"[$r name]">_ to free
	reset c

	
<'}}}}'>
	
	on clear
	from free to release
	reset c
	
	on launch
	from release to release
	reset c
	
	on finished
	from release to test
	reset c
	
	on check
	from test to idle
	
	on immediate
	from test to ready
	reset c
	
	invariant inv_free at free when (c==0)
	invariant inv_release at release when (c==0)
	invariant inv_test at test when (c==0)
	invariant inv_ready at ready when (c==0)
	
<'for {set k 0} {$k < [expr [llength $invariants]-1]} {incr k 2} {'>
	
	invariant inv_<"[lindex $invariants $k]"> at <"[lindex $invariants $k]"> when (c <= <"[lindex $invariants [expr $k+1]]">)
	
<'}'>
	
	
	
end
<'
set locks($gcounter) [list]
	foreach r $requests($gcounter) {
	if {[llength [$r validate]] && [llength [[$r validate] mutex]]} {
	lappend locks($gcounter) [join [list "validate" [$r name] [$component name]] _]'>
	
atom type LOCK_validate_<"[$r name]">_<"[$component name]">()

	clock c
	export port Port execute()
	export port Port end_exec()
	export port Port exec_ext()
	export port Port endexec_ext()
	export port Port wait()
	
	place <'
	for {set k 0} {$k <= [expr [llength [[$r validate] mutex]] + 1]} {incr k} {'>level_<"$k">, <'}'>running
	
	initial to level_<"[expr $k-2]">
	
<'for {set k 0} {$k <= [llength [[$r validate] mutex]] } {incr k} {'>
	
	on exec_ext
	from level_<"[expr $k+1]"> to level_<"$k">
	
	on endexec_ext
	from level_<"$k"> to level_<"[expr $k+1]">
<'if {$k== [llength [[$r validate] mutex]]} {'>
	reset c
	
<'}'>
	

	on wait
	from level_<"$k"> to level_<"[expr $k+1]">
<'if {$k==[llength [[$r validate] mutex]]} {'>
	reset c
	
<'}'>
	
<'}'>
	
	on execute
	from level_<"$k"> to running
	
	on end_exec
	from running to level_<"[expr $k-1]">
	
	invariant inv_ready at level_<"$k"> when (c == 0)
	
	
end
	
<'}
	
if {[$r kind] != "activity"} { 
set conflicts [$r mutex]
foreach c [$r codels] {lappend conflicts [$c mutex]}
if {[llength $conflicts]} {
lappend locks($gcounter) [join [list [$r name] [$component name]] _]'>
		
	atom type LOCK_<"[$r name]">_<"[$component name]">()

	clock c
	export port Port execute()
	export port Port end_exec()
	export port Port exec_ext()
	export port Port endexec_ext()
	export port Port wait()


	place <'
for {set k 0} {$k <= [expr [llength $conflicts] + 1]} {incr k 1} {'>
level_<"$k">, <'}'>running
	
	initial to level_<"[expr $k-2]">
	
<'for {set k 0} {$k <= [llength $conflicts]} {incr k} {'>
	
	on exec_ext
	from level_<"[expr $k+1]"> to level_<"$k">
	
	on endexec_ext
	from level_<"$k"> to level_<"[expr $k+1]">

<'if {$k==[llength $conflicts]} {'>
	reset c
	
<'}'>


	on wait
	from level_<"$k"> to level_<"[expr $k+1]">
<'if {$k==[llength $conflicts]} {'>
	reset c
	
<'}}'>
	
	on execute
	from level_<"$k"> to running
	
	on end_exec
	from running to level_<"[expr $k-1]">
	
	invariant inv_ready at level_<"$k"> when (c == 0)
	
end
	
<'}
} else {
	
foreach co [$r codels] {
	
if {[llength [$co mutex]] || [llength [mutex-ports-basic dotgen $co]]} {
lappend locks($gcounter) [join [list [$co name] [$r name] [$component name]] _]'>

atom type LOCK_<"[$co name]">_<"[$r name]">_<"[$component name]">()

	clock c
	export port Port execute()
	export port Port end_exec()
	export port Port exec_ext()
	export port Port endexec_ext()
	export port Port wait()

	
	place <'
	for {set k 0} {$k <= [expr [llength [$co mutex]] + [llength [mutex-ports-basic dotgen $co]] + 1]} {incr k} {'>level_<"$k">, <'}'>running
	
	initial to level_<"[expr $k-2]">
	
<'for {set k 0} {$k <= [expr [llength [$co mutex]] + [llength [mutex-ports-basic dotgen $co]]]} {incr k} {'>
	
	on exec_ext
	from level_<"[expr $k+1]"> to level_<"$k">
	
	on endexec_ext
	from level_<"$k"> to level_<"[expr $k+1]">
<'if {$k==[expr [llength [$co mutex]] + [llength [mutex-ports-basic dotgen $co]]]} {'>
	reset c
	
<'}'>
	

	on wait
	from level_<"$k"> to level_<"[expr $k+1]">
<'if {$k==[expr [llength [$co mutex]] + [llength [mutex-ports-basic dotgen $co]]]} {'>
	reset c
	
<'}'>
	
<'}'>
	
	on execute
	from level_<"$k"> to running
	
	on end_exec
	from running to level_<"[expr $k-1]">
	
	invariant inv_ready at level_<"$k"> when (c == 0)
	
end
	
<'}}}}'>

<'
foreach t [$component tasks] {
foreach co [$t codels] {
	
if {[llength [$co mutex]] || [llength [mutex-ports-basic dotgen $co]]} {
lappend locks($gcounter) [join [list [$co name] [$t name] [$component name]] _]'>

atom type LOCK_<"[$co name]">_<"[$t name]">_<"[$component name]">()

	clock c
	export port Port execute()
	export port Port end_exec()
	export port Port exec_ext()
	export port Port endexec_ext()
	export port Port wait()

	
	place <'
	for {set k 0} {$k <= [expr [llength [$co mutex]] + [llength [mutex-ports-basic dotgen $co]] + 1]} {incr k} {'>level_<"$k">, <'}'>running
	
	initial to level_<"[expr $k-2]">
	
<'for {set k 0} {$k <= [expr [llength [$co mutex]] + [llength [mutex-ports-basic dotgen $co]]]} {incr k} {'>
	
	on exec_ext
	from level_<"[expr $k+1]"> to level_<"$k">
	
	on endexec_ext
	from level_<"$k"> to level_<"[expr $k+1]">
<'if {$k==[expr [llength [$co mutex]] + [llength [mutex-ports-basic dotgen $co]]]} {'>
	reset c
	
<'}'>
	

	on wait
	from level_<"$k"> to level_<"[expr $k+1]">
<'if {$k==[expr [llength [$co mutex]] + [llength [mutex-ports-basic dotgen $co]]]} {'>
	reset c
	
<'}'>
	
<'}'>
	
	on execute
	from level_<"$k"> to running
	
	on end_exec
	from running to level_<"[expr $k-1]">
	
	invariant inv_ready at level_<"$k"> when (c == 0)

end
	
<'}}}'>


<'set counter 0
set permanent($gcounter) [list]
foreach t [$component tasks] {
	set nostopt($gcounter,$counter) [list]
	set resumeperm($gcounter,$counter) [list]
	set resumepermstart($gcounter,$counter) [list]
	if {![catch {$t period}]} {'>
		

/* <"[$t name]"> timer */

atom type TIMER_<"[$t name]">_<"[$component name]">()
	clock c
	export port Port tick()
	export port Port restart()
	export port Port init()

	place idle, start, test

	initial to idle
	
	on init
	from idle to start
	reset c

	on tick
	from start to test
	when (c == <"[expr [[$t period] value]*1000]">)
	reset c
	
	on restart
	from test to start

	invariant inv_start at start when (c <= <"[expr [[$t period] value]*1000]">)

	
end

<'}
if {[llength [$t codels]]} {
lappend permanent($gcounter) $t
'>
	
/* <"[$t name]"> permanent activity */

atom type PERM_<"[$t name]">_<"[$component name]">()
	clock c

<'if {$t != [lindex [$component tasks] 0]} {'>export <'}'>port Port begin()

<'set test [list]
set st 0
set startyields [list]
	foreach c [$t codels] {
		foreach tr [$c triggers] {
	if {[$tr name]=="start"} {
		if {([llength [$c mutex]] || [llength [mutex-ports-basic dotgen $c]])} {
		set st 1}
		foreach y [$c yields] {
		lappend startyields $y}
	} else {'><"\n\t"><'
		if {$tr in $startyields} {'>export <'}'>port Port to_<"[$tr name]">()<"\n"><'}
			 
		if {[llength [$c mutex]] || [llength [mutex-ports-basic dotgen $c]]} {'>
	
	export port Port wait_<"[$c name]">()<"\n\t">export port Port lock_<"[$c name]">()<"\n\t">export port Port unlock_<"[$c name]">()<"\n"><'}}
	if {!([llength [$c mutex]] || [llength [mutex-ports-basic dotgen $c]])} {'>
	port Port to_<"[$tr name]">_test()<"\n"><'}
		foreach y [$c yields] {
			if {[$y kind] == "pause event"} {
				if {[$y name] in $test} {continue}
				lappend test [$y name]
				if {$y in $startyields} {
				lappend resumepermstart($gcounter,$counter) [$y name]}}}}
	if {[llength $test]} {
	set resumeperm($gcounter,$counter) $test'>
	
	export port Port resume_()<"\n"><'
	foreach te $test {'>
	export port Port to_pause_<"$te">()<"\n"><'}}'>
	
	export port Port to_ether()
	export port Port void()
	

	place idle<'
	set invariants [list]
	set nts [list]
	foreach c [$t codels] {
		foreach tr [$c triggers] {lappend invariants [join [list [$tr name] "test"] _] 0'>, <"[$tr name]">_, <"[$tr name]">_test<'
		if {[llength [$c mutex]] || [llength [mutex-ports-basic dotgen $c]]} {
			lappend nts [$tr name]'>, <"[$tr name]">_wait<'
			lappend invariants [join [list [$tr name] "2"] _]
			if {![catch {$c wcet}]} {lappend invariants [expr [[$c wcet] value]*1000]
			} else {lappend invariants 0}'>, <"[$tr name]">_2<'
		} else {
			lappend invariants [join [list [$tr name] ""] _]
			if {![catch {$c wcet}]} {lappend invariants [expr [[$c wcet] value]*1000]
			} else {lappend invariants 0}}}}
	if {$t == [lindex [$component tasks] 0]} {lappend invariants "idle" 0}
foreach te $test {'>, pause_<"$te"><'}'>, ether_

	initial to idle

	on begin
	from idle to start_<'
	if {$st} {'>wait<'}'> // spawning <"[$t name]">
	reset c

<'

set genpause [list]

foreach c [$t codels] {
		foreach tr [$c triggers] {
		if {[llength [$c mutex]] || [llength [mutex-ports-basic dotgen $c]]} {'>
	
	on wait_<"[$c name]"> 
	from <"[$tr name]">_wait to <"[$tr name]">_
				
	on lock_<"[$c name]">
	from <"[$tr name]">_ to <"[$tr name]">_2
	eager
	reset c
	
	
	on unlock_<"[$c name]"> // call codel
	from <"[$tr name]">_2 to <"[$tr name]">_test
	reset c

	
<'} else {'>
		
	on to_<"[$tr name]">_test // call codel
	from <"[$tr name]">_ to <"[$tr name]">_test
	reset c
	
<'}
	
foreach y [$c yields] {
		
	if {[$y kind] == "pause event"} {'>
		
	on to_pause_<"[$y name]">
	from <"[$tr name]">_test to pause_<"[$y name]">

<'if {!($y in $genpause)} {'>
		
	on resume_
	from pause_<"[$y name]"> to <"[$y name]">_<'
	if {[$y name] in $nts} {'>wait<'}'>
	reset c

<'lappend genpause $y}'>
	
<'} else {'>
	
	on to_<"[$y name]">
	from <"[$tr name]">_test to <"[$y name]">_<'
	if {[$y name] in $nts} {'>wait<'}'>
	reset c
	
<'}}}}'>

	on void
	from ether_ to ether_

	
	
<'for {set k 0} {$k < [expr [llength $invariants]-1]} {incr k 2} {'>
	
	invariant inv_<"[lindex $invariants $k]"> at <"[lindex $invariants $k]"> when (c <= <"[lindex $invariants [expr $k+1]]">)
	
<'}'>
	
end
	  
<'}'>

/* <"[$t name]"> manager */

atom type MANAGER_<"[$t name]">_<"[$component name]">()
	clock c

	export port Port ended()
	
<'if {![catch {$t period}]} {'>
	export port Port tick()<'}
if {[llength [$t services]]} {'>
	
	export port Port no_more_services()
	export port Port begin()
	export port Port cycle()
	<'}
	if {[llength [$t codels]]} {'>
	
	export port Port begin_perm()
	export port Port cycle_perm()
	export port Port skip_perm()
	
<'}
set nostop [list]

foreach s [$t services] {
set flag 0
foreach st [$s fsm] {
		if {[$st name] == "stop"} {
		set flag 1
		break}
		}
		if {!$flag} {lappend nostop [$s name]
					lappend nostopt($gcounter,$counter) [$s name]}}
		
	if {[llength $nostop]} {'>
		
	export port Port inter_1()	
	export port Port inter_2()
	
	<'}'>
	
	
	place idle, <'
	set invariants [list]
	if {[llength [$t codels]]} {lappend invariants [join [list "start" "perm"] _]'>start_perm, end_perm, <'}
	if {[llength $nostop]} {lappend invariants [join [list "no" "stop" "defined"] _]'>no_stop_defined, <'}
	if {[llength [$t services]]} {lappend invariants "start"'>start, next, <'}'>finish
	
	initial to idle
	
<'if {![catch {$t period}]} {'>
	on tick
	from idle to <'
	if {[llength [$t codels]]} {'>start_perm<'
	} else {'>start<'}'>
	
	reset c
	
<'}

	if {[llength [$t codels]]} {'>
		
	on begin_perm
	from start_perm to end_perm

	
	on cycle_perm
	from end_perm to <'
	if {[llength [$t services]]} {'>start<'
	} else {'>finish<'}'>
	reset c
	
	on skip_perm
	from start_perm to <'
	if {[llength [$t services]]} {'>start<'
	} else {'>finish<'}'>
	reset c
	
<'}


if {[llength [$t services]]} {'>
	
	
	on begin
	from start to next
	
	on cycle
	from next to start
	reset c
	
	on no_more_services
	from start to finish
	reset c
	

<'if {[llength $nostop]} {'>
	
	on inter_1
	from start to no_stop_defined
	reset c
	
	on inter_2
	from no_stop_defined to start
	reset c
	
		
<'}}'>
 

	on ended
	from finish to idle

	invariant inv_finish at finish when (c == 0)
	
<'foreach inv $invariants  {'>
	
	invariant inv_<"$inv"> at <"$inv"> when (c == 0)
	
<'}'>	
	
end
		
	
	
<'foreach s [$t services] {'>
	
/* service <"[$s name]"> of <"[$t name]"> */
	
atom type SERVICE_<"[$s name]">_<"[$component name]">()
	clock c
	export port Port ended()
	export port Port begin()<'
	foreach st [$s fsm] {'>
		
	port Port to_<"[$st name]">()<"\n"><'}
	
	set test [list]
	set nts [list]
	set st 0
	set sto 0
	foreach c [$s codels] {
		if {[$c name] == "start" && ([llength [$c mutex]] || [llength [mutex-ports-basic dotgen $c]])} {set st 1}
		if {[$c name] == "stop" && ([llength [$c mutex]] || [llength [mutex-ports-basic dotgen $c]])} {set sto 1}
		if {[llength [$c mutex]] || [llength [mutex-ports-basic dotgen $c]]} {'>
	export port Port wait_<"[$c name]">()<"\n\t">export port Port lock_<"[$c name]">()<"\n\t">export port Port unlock_<"[$c name]">()<"\n"><'}
		foreach tr [$c triggers] {
	
	if {!([llength [$c mutex]] || [llength [mutex-ports-basic dotgen $c]])} {'>
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
	set invariants [list]
	foreach c [$s codels] {
		foreach tr [$c triggers] {lappend invariants [join [list [$tr name] "test"] _] 0'>, <"[$tr name]">_, <"[$tr name]">_test<'
		if {[llength [$c mutex]] || [llength [mutex-ports-basic dotgen $c]]} {
			lappend nts [$tr name]
			lappend invariants [join [list [$tr name] "2"] _]
			if {![catch {$c wcet}]} {lappend invariants [expr [[$c wcet] value]*1000]
			} else {lappend invariants 0}'>, <"[$tr name]">_wait, <"[$tr name]">_2<'
			} else {
			lappend invariants [join [list [$tr name] ""] _]
			if {![catch {$c wcet}]} {lappend invariants [expr [[$c wcet] value]*1000]
			} else {lappend invariants 0}}}}
	foreach te $test {'>, pause_<"$te"><'}'>
		

	initial to idle
	
	on begin
	from idle to start_<'
	if {$st} {'>wait<'}'>

	reset c
	
<'if {[$s name] in $nostop} {'>
	
	on interrupted
	from idle to idle
	
<'} else {'>
	
	on interrupted
	from idle to stop_<'
	if {$st} {'>wait<'}'>
	
	reset c
	
	
<'}

set genpause [list]
set stpause [list]

foreach c [$s codels] {
		foreach tr [$c triggers] {
		if {[llength [$c mutex]] || [llength [mutex-ports-basic dotgen $c]]} {'>
			
	on wait_<"[$c name]"> 
	from <"[$tr name]">_wait to <"[$tr name]">_
				
	on lock_<"[$c name]">
	from <"[$tr name]">_ to <"[$tr name]">_2
	reset c
	
	on unlock_<"[$c name]"> // call codel
	from <"[$tr name]">_2 to <"[$tr name]">_test
	reset c
	
	
<'} else {'>
		
	on to_<"[$tr name]">_test // call codel
	from <"[$tr name]">_ to <"[$tr name]">_test
	reset c
	
	
<'}
	
	foreach y [$c yields] {
		
	if {[$y kind] == "pause event"} {'>
		
	on to_pause_<"[$y name]">
	from <"[$tr name]">_test to pause_<"[$y name]">

<'if {!($y in $genpause)} {'>	
	on resume_
	from pause_<"[$y name]"> to <"[$y name]">_<'
	if {[$y name] in $nts} {'>wait<'}'>
	reset c
	
<'lappend genpause $y}'>

<'if {!($y in $stpause)} {'>
	
<'if {[$s name] in $nostop} {'>
	
	on interrupted
	from pause_<"[$y name]"> to idle
	
<'} else {'>
	
	on interrupted
	from pause_<"[$y name]"> to stop_<'
	if {$sto} {'>wait<'}'>
	reset c
	
<'}

lappend stpause $y}'>
	
<'} else {
	
	if {[$y name]=="ether"} {'>
		
	on ended
	from <"[$tr name]">_test to idle


<'} else {'>
	
	on to_<"[$y name]">
	from <"[$tr name]">_test to <"[$y name]">_<'
	if {[$y name] in $nts} {'>wait<'}'>
	reset c 
	
<'}}}}}'>

<'for {set k 0} {$k < [expr [llength $invariants]-1]} {incr k 2} {'>
	
	invariant inv_<"[lindex $invariants $k]"> at <"[lindex $invariants $k]"> when (c <= <"[lindex $invariants [expr $k+1]]">)
	
<'}'>

	
end	
	
<'}
incr counter}'>

/* manage spawning of different tasks */

atom type READY_<"[$component name]">()
	clock c
	export port Port block()
	export port Port allow()

	place start, spawncntrl, finish

	initial to <'
	if {[llength $permanent($gcounter)]} {'>start<'
	} else {'>finish<'}'>

	on block
	from start to spawncntrl
	reset c

	on allow
	from spawncntrl to finish
	
	invariant inv_spawn at spawncntrl when (c == 0)

end

<'incr gcounter}'>


/* module */

<'
set compnames [list]
foreach c [dotgen components] {
	lappend compnames [$c name]
}

set outputname [join $compnames _]
'>

compound type <"$outputname">()

<'
set gcounter 0
foreach component [dotgen components] {'>
	
	component CLIENT_<"[$component name]"> client_<"[$component name]">()
	component CONTROL_<"[$component name]"> control_<"[$component name]">()
	component READY_<"[$component name]"> ready_<"[$component name]">()
	component SIGNAL <'
foreach t [$component tasks] {'>signal_<"[$t name]">_<"[$component name]">()<'
	if {$t != [lindex [$component tasks] end]} {'>, <'}}
foreach t [$component tasks] {'>
	
	
	component MANAGER_<"[$t name]">_<"[$component name]"> manager_<"[$t name]">_<"[$component name]">()



<'    if {![catch {$t period}]} {'>

	component TIMER_<"[$t name]">_<"[$component name]"> timer_<"[$t name]">_<"[$component name]">()
	
<'}'>
	
<'if {[llength [$t codels]]} {'>		
	component PERM_<"[$t name]">_<"[$component name]"> perm_<"[$t name]">_<"[$component name]">()<'}
foreach s [$t services] {'>
	
	component SERVICE_<"[$s name]">_<"[$component name]"> <"[$s name]">_<"[$component name]">_inst_1(), <"[$s name]">_<"[$component name]">_inst_2()<'}}'>

<'if {[llength $activities($gcounter)]} {'>
		
	component STATUS <'
	foreach a $activities($gcounter) {'><"[$a name]">_<"[$component name]">_status_1(), <"[$a name]">_<"[$component name]">_status_2()<'
		if {$a != [lindex $activities($gcounter) end]} {'>, <'}}'>

<'}'>

<'foreach l $locks($gcounter) {'>
	component LOCK_<"$l"> lock_<"$l">()
	
<'}
incr gcounter}

set gcounter 0
set taskcount 0
foreach component [dotgen components] {'>

						
/* connectors */

/* restart control task */

	connector sync2 restart_<"[$component name]">(control_<"[$component name]">.finished, client_<"[$component name]">.finished)
	
/* spawn control task */

	connector sync2 spawn_cntrl_task_<"[$component name]">(control_<"[$component name]">.spawn, ready_<"[$component name]">.allow)
	
/* empty box but at least one service ended */

	connector sync2 noreq_<"[$component name]">(control_<"[$component name]">.norequest, client_<"[$component name]">.norequest)
	
/* clear all (and send final replies) */

	connector trig<"[expr [llength $activities($gcounter)]*2+1]"> clearall_<"[$component name]">(control_<"[$component name]">.clear<'
	foreach a $activities($gcounter) {'>, <"[$a name]">_<"[$component name]">_status_1.clear, <"[$a name]">_<"[$component name]">_status_2.clear<'}'>)
	
<'
if {[llength $permanent($gcounter)]} {'>
	
/* spawn execution tasks */

<'set index 0
foreach p $permanent($gcounter) {'>
	
/* <"[$p name]"> */

<'	if {$p == [lindex $permanent($gcounter) 0]} {'>
	
connector <'if {![catch {$p period}]} {'>sync2<'
} else {'>singleton<'}'> begin_spawn_<"[$p name]">_<"[$component name]">(perm_<"[$p name]">_<"[$component name]">.begin
<'	    if {![catch {$p period}]} {'>, timer_<"[$p name]">_<"[$component name]">.init<'}'>)

<'}	

	foreach c [$p codels] {
	    foreach tr [$c triggers] {
		if {[$tr name] == "start"} {
		    foreach y [$c yields] {'>
		
connector <'
if {[$y kind] == "pause event"} {'>sync<'
} else {'>trig<'}'>
<'			if {$p == [lindex $permanent($gcounter) end] || [catch {[lindex $permanent($gcounter) [expr $index+1]] period}]} {'>2<'
			} else {'>3<'}'> spawn_<"[$y name]">_<"[$p name]">_<"[$component name]">(perm_<"[$p name]">_<"[$component name]">.to_<'
			if {[$y kind] == "pause event"} {'>pause_<'}'><"[$y name]">, <'
			if {$p == [lindex $permanent($gcounter) end]} {'>ready_<"[$component name]">.block<'
			} else {'>perm_<"[[lindex $permanent($gcounter) [expr $index+1]] name]">_<"[$component name]">.begin<'
			    if {![catch {[lindex $permanent($gcounter) [expr $index+1]] period}]} {'>, timer_<"[[lindex $permanent($gcounter) [expr $index+1]] name]">_<"[$component name]">.init<'}}'>)

<'		    }
		    break}}
	    break}
	incr index}

}

set counter 0
foreach t [$component tasks] {
'>
	
/* signal end of activities (or not) */
	
	connector trig3 end_<"[$t name]">_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.ended, timer_<"[$t name]">_<"[$component name]">.restart, signal_<"[$t name]">_<"[$component name]">.hold)
	
/* report end of activities */

	connector sync2 no_report_<"[$t name]">_<"[$component name]">(control_<"[$component name]">.check, signal_<"[$t name]">_<"[$component name]">.nosched)
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
	
<'if {[llength $resumeperm($gcounter,$counter)]} {'>
	
/* resume permanent activity */

	connector sync2 resume_perm_act_<"[$t name]">_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.begin_perm, perm_<"[$t name]">_<"[$component name]">.resume_)
	
/* pause permanent activity (asynchrone for first execution) */

<'foreach y $resumepermstart($gcounter,$counter) {'>
					
	connector trig2 pause_perm_act_<"$y">_<"[$t name]">_<"[$component name]">(perm_<"[$t name]">_<"[$component name]">.to_pause_<"$y">, manager_<"[$t name]">_<"[$component name]">.cycle_perm)

<'}	
foreach y $resumeperm($gcounter,$counter) {
	if {!($y in $resumepermstart($gcounter,$counter))} {'>
		
	connector trig2 pause_perm_act_<"$y">_<"[$t name]">_<"[$component name]">(perm_<"[$t name]">_<"[$component name]">.to_pause_<"$y">, manager_<"[$t name]">_<"[$component name]">.cycle_perm)
	
<'}}}}

	
	if {[llength [$t services]]} {'>
	
/* no (more) active instances */

<'if {!$pauseevents($taskcount)} {'>
	
	connector singleton to_finish_<"[$t name]">_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.no_more_services)
	
<'} else {'>
	
	connector trig<"[expr $pauseevents($taskcount)*2+1]"> to_finish_<"[$t name]">_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.no_more_services<'
	foreach s [$t services] {
	set check 0
	foreach c [$s codels] {
		foreach y [$c yields] {
		if {[$y kind] == "pause event"} {'>, <"[$s name]">_<"[$component name]">_status_1.ready, <"[$s name]">_<"[$component name]">_status_2.ready<'
			set check 1
			break}}
			if {$check} {break}}}'>)

<'}
	
foreach s [$t services] {
	set test [list]
	foreach c [$s codels] {
		foreach y [$c yields] {
		if {[$y kind] == "pause event"} {
			if {[$y name] in $test} {continue}
				lappend test [$y name]}}}
	if {[llength $test]} {'>
		
/* resume (paused last cycle) */

	connector sync3 resume_<"[$s name]">_1_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.begin, <"[$s name]">_<"[$component name]">_inst_1.resume_, <"[$s name]">_<"[$component name]">_status_1.run)
	connector sync3 resume_<"[$s name]">_2_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.begin, <"[$s name]">_<"[$component name]">_inst_2.resume_, <"[$s name]">_<"[$component name]">_status_2.run)

<'	foreach te $test {'>
	
/* pause events */

	connector sync3 pause_<"$te">_<"[$s name]">_1_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.cycle, <"[$s name]">_<"[$component name]">_inst_1.to_pause_<"$te">, <"[$s name]">_<"[$component name]">_status_1.not_ready)
	connector sync3 pause_<"$te">_<"[$s name]">_2_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.cycle, <"[$s name]">_<"[$component name]">_inst_2.to_pause_<"$te">, <"[$s name]">_<"[$component name]">_status_2.not_ready)
	
<'}}'>
	
	
/* <"[$s name]"> instance 1 & 2*/

/* nominal execution */

	connector sync3 nominal_<"[$s name]">_1_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.begin, <"[$s name]">_<"[$component name]">_inst_1.begin, <"[$s name]">_<"[$component name]">_status_1.run)
	connector sync3 nominal_<"[$s name]">_2_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.begin, <"[$s name]">_<"[$component name]">_inst_2.begin, <"[$s name]">_<"[$component name]">_status_2.run)
	
/* end */

	connector sync4 finished_<"[$s name]">_1_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.cycle, <"[$s name]">_<"[$component name]">_inst_1.ended, <"[$s name]">_<"[$component name]">_status_1.finished, signal_<"[$t name]">_<"[$component name]">.sig)
	connector sync4 finished_<"[$s name]">_2_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.cycle, <"[$s name]">_<"[$component name]">_inst_2.ended, <"[$s name]">_<"[$component name]">_status_2.finished, signal_<"[$t name]">_<"[$component name]">.sig)
	
/* interruption */


<'if {[$s name] in $nostopt($gcounter,$counter)} {'>
	
	connector sync2 interrupted_<"[$s name]">_1_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.inter_1, <"[$s name]">_<"[$component name]">_status_1.inter)
	connector sync4 interrupted_2_<"[$s name]">_1_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.inter_2, <"[$s name]">_<"[$component name]">_inst_1.interrupted, <"[$s name]">_<"[$component name]">_status_1.finished, signal_<"[$t name]">_<"[$component name]">.sig)
	
	connector sync2 interrupted_<"[$s name]">_2_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.inter_1, <"[$s name]">_<"[$component name]">_status_2.inter)
	connector sync4 interrupted_2_<"[$s name]">_2_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.inter_2, <"[$s name]">_<"[$component name]">_inst_2.interrupted, <"[$s name]">_<"[$component name]">_status_2.finished, signal_<"[$t name]">_<"[$component name]">.sig)   
	
<'} else {'>
	
	connector sync3 interrupted_<"[$s name]">_1_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.begin, <"[$s name]">_<"[$component name]">_inst_1.interrupted, <"[$s name]">_<"[$component name]">_status_1.inter)	
	connector sync3 interrupted_<"[$s name]">_2_<"[$component name]">(manager_<"[$t name]">_<"[$component name]">.begin, <"[$s name]">_<"[$component name]">_inst_2.interrupted, <"[$s name]">_<"[$component name]">_status_2.inter)
		
<'}}}
incr counter
incr taskcount}

foreach a $activities($gcounter) {'>
	
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

<'foreach n $nonactivities($gcounter) {'>
	
/* receive request */
	
	connector sync2 request_<"[$n name]">_<"[$component name]">(control_<"[$component name]">.req_<"[$n name]">, client_<"[$component name]">.req_<"[$n name]">)
	
<'if {[llength [$n interrupts]]} {'>
	
	connector trig<"[expr [llength [$n interrupts]]*2+1]"> inter_inc_<"[$n name]">_<"[$component name]">(control_<"[$component name]">.inter_<"[$n name]"><'
	foreach i [$n interrupts] {'>, <"[$i name]">_<"[$component name]">_status_1.interrupt, <"[$i name]">_<"[$component name]">_status_2.interrupt<'}'>)<'}'>

<'}

if {[llength $locks($gcounter)]} {'>
	
/* mutual exclusion connectors */<'}

foreach t [$component tasks] {
	foreach c [$t codels] {
		if {[llength [$c mutex]] || [llength [mutex-ports-basic dotgen $c]]} {'>
			
/* <"[$t name]"> permanent activity. codel <"[$c name]"> */

	connector sync2 wait_<"[$c name]">_<"[$t name]">_<"[$component name]">(perm_<"[$t name]">_<"[$component name]">.wait_<"[$c name]">, lock_<"[$c name]">_<"[$t name]">_<"[$component name]">.wait)

	connector sync<"[expr [llength [$c mutex]]+[llength [mutex-ports-basic dotgen $c]]+2]"> take_<"[$c name]">_<"[$t name]">_<"[$component name]">(perm_<"[$t name]">_<"[$component name]">.lock_<"[$c name]">, lock_<"[$c name]">_<"[$t name]">_<"[$component name]">.execute<'
	foreach m [$c mutex] {'>, lock_<'
		if {![catch {$m service}]} {
			if {$m == [[$m service] validate]} {'>validate_<'
			} else {
				if {[[$m service] kind] == "activity"} {'><"[$m name]">_<'
					}}'><"[[$m service] name]"><'
		} else {
			if {![catch {$m task}]} {'><"[$m name]">_<"[[$m task] name]"><'
			} else {'><"[$m name]"><'}}'>_<"[$component name]">.exec_ext<'}
	foreach m [mutex-ports-basic dotgen $c] {'>, <"$m">.exec_ext<'}'>)
	
	connector sync<"[expr [llength [$c mutex]]+[llength [mutex-ports-basic dotgen $c]]+2]"> give_<"[$c name]">_<"[$t name]">_<"[$component name]">(perm_<"[$t name]">_<"[$component name]">.unlock_<"[$c name]">, lock_<"[$c name]">_<"[$t name]">_<"[$component name]">.end_exec<'
	foreach m [$c mutex] {'>, lock_<'
		if {![catch {$m service}]} {
			if {$m == [[$m service] validate]} {'>validate_<'
			} else {
				if {[[$m service] kind] == "activity"} {'><"[$m name]">_<'
					}}'><"[[$m service] name]"><'
		} else {
			if {![catch {$m task}]} {'><"[$m name]">_<"[[$m task] name]"><'
			} else {'><"[$m name]"><'}}'>_<"[$component name]">.endexec_ext<'}
	foreach m [mutex-ports-basic dotgen $c] {'>, <"$m">.endexec_ext<'}'>)

<'}}}

foreach a $requests($gcounter) {
	
	foreach v [$a validate] {
	if {[llength [$v mutex]]} {'>

/* <"[$a name]"> validation codel */

	connector sync2 wait_<"[$v name]">_<"[$a name]">_<"[$component name]">(control_<"[$component name]">.wait_<"[$a name]">_validate, lock_validate_<"[$a name]">_<"[$component name]">.wait)

	connector sync<"[expr [llength [$v mutex]]+2]"> take_<"[$v name]">_<"[$a name]">_<"[$component name]">(control_<"[$component name]">.lock_<"[$a name]">_validate, lock_validate_<"[$a name]">_<"[$component name]">.execute<'
	foreach m [$v mutex] {'>, lock_<'
		if {![catch {$m service}]} {
			if {$m == [[$m service] validate]} {'>validate_<'
			} else {
				if {[[$m service] kind] == "activity"} {'><"[$m name]">_<'
					}}'><"[[$m service] name]"><'
		} else {
			if {![catch {$m task}]} {'><"[$m name]">_<"[[$m task] name]"><'
			} else {'><"[$m name]"><'}}'>_<"[$component name]">.exec_ext<'}'>)
		
	connector sync<"[expr [llength [$v mutex]]+2]"> give_<"[$v name]">_<"[$r name]">_<"[$component name]">(control_<"[$component name]">.unlock_<"[$a name]">_validate, lock_validate_<"[$a name]">_<"[$component name]">.end_exec<'
	foreach m [$v mutex] {'>, lock_<'
		if {![catch {$m service}]} {
			if {$m == [[$m service] validate]} {'>validate_<'
			} else {
				if {[[$m service] kind] == "activity"} {'><"[$m name]">_<'
					}}'><"[[$m service] name]"><'
		} else {
			if {![catch {$m task}]} {'><"[$m name]">_<"[[$m task] name]"><'
			} else {'><"[$m name]"><'}}'>_<"[$component name]">.endexec_ext<'}'>)
<'}}}


foreach a $activities($gcounter) {

foreach c [$a codels] {
	if {[llength [$c mutex]] || [llength [mutex-ports-basic dotgen $c]]} {
		
'>	/* codel <"[$c name]"> (activity <"[$a name]">) */

	connector sync2 wait_<"[$c name]">_<"[$a name]">_1_<"[$component name]">(<"[$a name]">_<"[$component name]">_inst_1.wait_<"[$c name]">, lock_<"[$c name]">_<"[$a name]">_<"[$component name]">.wait)
	connector sync2 wait_<"[$c name]">_<"[$a name]">_2_<"[$component name]">(<"[$a name]">_<"[$component name]">_inst_2.wait_<"[$c name]">, lock_<"[$c name]">_<"[$a name]">_<"[$component name]">.wait)

	connector sync<"[expr [llength [$c mutex]]+[llength [mutex-ports-basic dotgen $c]]+2]"> take_<"[$c name]">_<"[$a name]">_1_<"[$component name]">(<"[$a name]">_<"[$component name]">_inst_1.lock_<"[$c name]">, lock_<"[$c name]">_<"[$a name]">_<"[$component name]">.execute<'
	foreach m [$c mutex] {'>, lock_<'
		if {![catch {$m service}]} {
			if {$m == [[$m service] validate]} {'>validate_<'
			} else {
				if {[[$m service] kind] == "activity"} {'><"[$m name]">_<'
					}}'><"[[$m service] name]"><'
		} else {
			if {![catch {$m task}]} {'><"[$m name]">_<"[[$m task] name]"><'
			} else {'><"[$m name]"><'}}'>_<"[$component name]">.exec_ext<'}
	foreach m [mutex-ports-basic dotgen $c] {'>, <"$m">.exec_ext<'}'>)
			
	connector sync<"[expr [llength [$c mutex]]+[llength [mutex-ports-basic dotgen $c]]+2]"> take_<"[$c name]">_<"[$a name]">_2_<"[$component name]">(<"[$a name]">_<"[$component name]">_inst_2.lock_<"[$c name]">, lock_<"[$c name]">_<"[$a name]">_<"[$component name]">.execute<'
	foreach m [$c mutex] {'>, lock_<'
		if {![catch {$m service}]} {
			if {$m == [[$m service] validate]} {'>validate_<'
			} else {
				if {[[$m service] kind] == "activity"} {'><"[$m name]">_<'
					}}'><"[[$m service] name]"><'
		} else {
			if {![catch {$m task}]} {'><"[$m name]">_<"[[$m task] name]"><'
			} else {'><"[$m name]"><'}}'>_<"[$component name]">.exec_ext<'}
	foreach m [mutex-ports-basic dotgen $c] {'>, <"$m">.exec_ext<'}'>)
	
		
	connector sync<"[expr [llength [$c mutex]]+[llength [mutex-ports-basic dotgen $c]]+2]"> give_<"[$c name]">_<"[$a name]">_1_<"[$component name]">(<"[$a name]">_<"[$component name]">_inst_1.unlock_<"[$c name]">, lock_<"[$c name]">_<"[$a name]">_<"[$component name]">.end_exec<'
	foreach m [$c mutex] {'>, lock_<'
		if {![catch {$m service}]} {
			if {$m == [[$m service] validate]} {'>validate_<'
			} else {
				if {[[$m service] kind] == "activity"} {'><"[$m name]">_<'
					}}'><"[[$m service] name]"><'
		} else {
			if {![catch {$m task}]} {'><"[$m name]">_<"[[$m task] name]"><'
			} else {'><"[$m name]"><'}}'>_<"[$component name]">.endexec_ext<'}
	foreach m [mutex-ports-basic dotgen $c] {'>, <"$m">.endexec_ext<'}'>)
			
			
	connector sync<"[expr [llength [$c mutex]]+[llength [mutex-ports-basic dotgen $c]]+2]"> give_<"[$c name]">_<"[$a name]">_2_<"[$component name]">(<"[$a name]">_<"[$component name]">_inst_2.unlock_<"[$c name]">, lock_<"[$c name]">_<"[$a name]">_<"[$component name]">.end_exec<'
	foreach m [$c mutex] {'>, lock_<'
		if {![catch {$m service}]} {
			if {$m == [[$m service] validate]} {'>validate_<'
			} else {
				if {[[$m service] kind] == "activity"} {'><"[$m name]">_<'
					}}'><"[[$m service] name]"><'
		} else {
			if {![catch {$m task}]} {'><"[$m name]">_<"[[$m task] name]"><'
			} else {'><"[$m name]"><'}}'>_<"[$component name]">.endexec_ext<'}
	foreach m [mutex-ports-basic dotgen $c] {'>, <"$m">.endexec_ext<'}'>)
	
<'}}}


foreach a $nonactivities($gcounter) {
set conflicts [$a mutex]
foreach c [$a codels] {
	#workaround some stupidity (mutex giving two codels as one element)
	foreach mu [$c mutex] {
	lappend conflicts $mu}}
		if {[llength $conflicts]} {'>
			
/* service <"[$a name]"> */

	connector sync2 wait_<"[$a name]">_<"[$component name]">(control_<"[$component name]">.wait_<"[$a name]">, lock_<"[$a name]">_<"[$component name]">.wait)

	connector sync<"[expr [llength $conflicts] +2]"> take_<"[$a name]">_<"[$component name]">(control_<"[$component name]">.lock_<"[$a name]">, lock_<"[$a name]">_<"[$component name]">.execute<'
	foreach m $conflicts {'>, lock_<'
		if {![catch {$m service}]} {
			if {$m == [[$m service] validate]} {'>validate_<'
			} else {
				if {[[$m service] kind] == "activity"} {'><"[$m name]">_<'
					}}'><"[[$m service] name]"><'
		} else {
			if {![catch {$m task}]} {'><"[$m name]">_<"[[$m task] name]"><'
			} else {'><"[$m name]"><'}}'>_<"[$component name]">.exec_ext<'}'>)
			
	
	connector sync<"[expr [llength $conflicts] +2]"> give_<"[$a name]">_<"[$component name]">(control_<"[$component name]">.unlock_<"[$a name]">, lock_<"[$a name]">_<"[$component name]">.end_exec<'
	foreach m $conflicts {'>, lock_<'
		if {![catch {$m service}]} {
			if {$m == [[$m service] validate]} {'>validate_<'
			} else {
				if {[[$m service] kind] == "activity"} {'><"[$m name]">_<'
					}}'><"[[$m service] name]"><'
		} else {
			if {![catch {$m task}]} {'><"[$m name]">_<"[[$m task] name]"><'
			} else {'><"[$m name]"><'}}'>_<"[$component name]">.endexec_ext<'}'>)
	
		
<'}}
incr gcounter}

set gcounter 0
foreach component [dotgen components] {'>


/* priorities */

<'
foreach t [$component tasks] {
	if {[llength [$t codels]]} {'>
		
/* spawn */

	<' foreach c [$t codels] {
	foreach tr [$c triggers] {
	if {[$tr name] == "start"} { 
	foreach y [$c yields] {'>

	priority spawn_pr_<"[$y name]">_<"[$t name]">_<"[$component name]"> end_perm_act_<"[$t name]">_<"[$component name]">:* < spawn_<"[$y name]">_<"[$t name]">_<"[$component name]">:*

<'}
break}}
break}
	
}

	foreach s [$t services] {
	set test [list]
	foreach c [$s codels] {
		foreach y [$c yields] {
		if {[$y kind] == "pause event"} {
			if {[$y name] in $test} {continue}
				lappend test [$y name]}}}'>
		
/* no more sevices to execute (or not at all) */
		
	priority exec_nom_<"[$s name]">_1_<"[$component name]"> to_finish_<"[$t name]">_<"[$component name]">:* < nominal_<"[$s name]">_1_<"[$component name]">:*
	priority exec_nom_<"[$s name]">_2_<"[$component name]"> to_finish_<"[$t name]">_<"[$component name]">:* < nominal_<"[$s name]">_2_<"[$component name]">:*
	
<'if {[llength $test]} {'>
	
	priority exec_res_<"[$s name]">_1_<"[$component name]"> to_finish_<"[$t name]">_<"[$component name]">:* < resume_<"[$s name]">_1_<"[$component name]">:*
	priority exec_res_<"[$s name]">_2_<"[$component name]"> to_finish_<"[$t name]">_<"[$component name]">:* < resume_<"[$s name]">_2_<"[$component name]">:*

<'}'>

	priority exec_inter_<"[$s name]">_1_<"[$component name]"> to_finish_<"[$t name]">_<"[$component name]">:* < interrupted_<"[$s name]">_1_<"[$component name]">:*
	priority exec_inter_<"[$s name]">_2_<"[$component name]"> to_finish_<"[$t name]">_<"[$component name]">:* < interrupted_<"[$s name]">_2_<"[$component name]">:*
	
<'}}

foreach a $activities($gcounter) {'>
	
/* launch activity before restarting control task */

	priority launch_<"[$a name]">_first_<"[$component name]"> restart_<"[$component name]">:* < launch_<"[$a name]">_1_<"[$component name]">:*
	priority launch_<"[$a name]">_2_first_<"[$component name]"> restart_<"[$component name]">:* < launch_<"[$a name]">_2_<"[$component name]">:*
	
<'}
incr gcounter}'>

end

end



	

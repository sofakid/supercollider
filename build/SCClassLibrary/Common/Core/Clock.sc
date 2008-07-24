// clocks for timing threads.

Clock {
	// abstract class
	*play { arg task;
		var beats, seconds;
		task.clock = this;
		seconds = thisThread.seconds;
		beats = this.secs2beats(seconds);
		this.sched(task.value(beats, seconds, this), task)
	}
	seconds { ^thisThread.seconds }
}

SystemClock : Clock {
	*clear {
		_SystemClock_Clear
		^this.primitiveFailed
	}
	*sched { arg delta, item;
		_SystemClock_Sched
		^this.primitiveFailed
	}
	*schedAbs { arg time, item;
		_SystemClock_SchedAbs
		^this.primitiveFailed
	}
	
	*beats2secs { arg beats; ^beats }
	*secs2beats { arg secs; ^secs }
	*beats { ^thisThread.seconds }
	*seconds { ^thisThread.seconds }
	
	// tempo clock compatibility
	*beats2bars { ^0 }
	*bars2beats { ^0 }
	*timeToNextBeat { ^0 }
	*nextTimeOnGrid { arg quant = 1, phase = 0;
		^this.beats + phase
	}
	
}


TempoClock : Clock {
	classvar <all, <>default;

	var <queue, ptr;
	
	var <beatsPerBar=4.0, barsPerBeat=0.25;
	var <baseBarBeat=0.0, <baseBar=0.0;
	var <>permanent=false;

/*
You should only change the tempo 'now'. You can't set the tempo at some beat in the future or past, even though you might think so from the methods. 

There are several ideas of now:
	elapsed time, i.e. "real time"
	logical time in the current time base.
	logical time in another time base.

logical time is time that is incremented by exact amounts from the time you started. It is not affected by the actual time your task gets scheduled, which may shift around somewhat due to system load. By calculating using logical time instead of actual time, your process will not drift out of sync over long periods. every thread stores a clock and its current logical time in seconds and beats relative to that clock.

elapsed time is whatever the system clock says it is right now. elapsed time is always advancing. logical time only advances when your task yields or returns.

*/

	*new { arg tempo, beats, seconds, queueSize=256;
		^super.new.init(tempo, beats, seconds, queueSize)
	}
	
	*initClass {
		default = this.new.permanent_(true);
		CmdPeriod.add(this);
	}	
	
	*cmdPeriod {
		all.do({ arg item; item.clear(false) });
		all.do({ arg item; if (item.permanent.not, { item.stop })  })
	}	

	init { arg tempo, beats, seconds, queueSize;
		queue = Array.new(queueSize);
		this.prStart(tempo, beats, seconds);
		all = all.add(this);
	}

	stop {
		all.take(this);
		this.prStop;
	}
	
	play { arg task, quant = 1; 
		this.schedAbs(quant.nextTimeOnGrid(this), task) 
	}
	
	playNextBar { arg task; this.schedAbs(this.nextBar, task) }
	
	*play { ^this.shouldNotImplement(thisMethod) }
	
	tempo {
		_TempoClock_Tempo
		^this.primitiveFailed
	}
	beatDur {
		_TempoClock_BeatDur
		^this.primitiveFailed
	}
	elapsedBeats {
		_TempoClock_ElapsedBeats
		^this.primitiveFailed
		/* primitive does this:
			^this.secs2beats(Main.elapsedTime).
		*/
	}
	beats {
		// returns the appropriate beats for this clock from any thread
		_TempoClock_Beats
		^this.primitiveFailed
		/* primitive does this:
			if (thisThread.clock == this) { ^thisThread.beats }
			^this.secs2beats(thisThread.seconds)
		*/
	}

	sched { arg delta, item;
		_TempoClock_Sched
		^this.primitiveFailed
	}
	schedAbs { arg beat, item;
		_TempoClock_SchedAbs
		^this.primitiveFailed
	}
	clear { | releaseNodes = true |
		// flag tells EventStreamPlayers that CmdPeriod is removing them, so
		// nodes are already freed
		queue.pairsDo { arg time, item; item.removedFromScheduler(releaseNodes) };
		^this.prClear;
	}

		
	// for setting the tempo at the current logical time 
	// (even another TempoClock's logical time).
	tempo_ { arg newTempo;
		this.setTempoAtBeat(newTempo, this.beats);
	}
	beatsPerBar_ { arg newBeatsPerBar;
		if (thisThread.clock != this) { 
			"should only change beatsPerBar within the scheduling thread.".error;
			^this
		};
		this.setMeterAtBeat(newBeatsPerBar, thisThread.beats);
	}

	// for setting the tempo at the current elapsed time .
	etempo_ { arg newTempo;
		this.setTempoAtSec(newTempo, Main.elapsedTime);
	}

	beats2secs { arg beats; 
		_TempoClock_BeatsToSecs
		^this.primitiveFailed
	}
	secs2beats { arg secs; 
		_TempoClock_SecsToBeats
		^this.primitiveFailed
	}
	
	prDump { 
		_TempoClock_Dump
		^this.primitiveFailed
	}
	
	nextTimeOnGrid { arg quant = 1, phase = 0;
		if (quant == 0) { ^this.beats + phase };
		if (quant < 0) { quant = beatsPerBar * quant.neg };
		^roundUp(this.beats - baseBarBeat - (phase % quant), quant) + baseBarBeat + phase
	}
	
	timeToNextBeat { arg quant=1.0; // logical time to next beat
		^quant.nextTimeOnGrid(this) - this.beats
	}
	
	beats2bars { arg beats;
		^(beats - baseBarBeat) * barsPerBeat + baseBar;
	}
	bars2beats { arg bars;
		^(bars - baseBar) * beatsPerBar + baseBarBeat;
	}
	bar {
		// return the current bar.
		^this.beats2bars(this.beats).floor;
	}
	nextBar { arg beat;
		// given a number of beats, determine number beats at the next bar line.
		if (beat.isNil) { beat = this.beats };
		^this.bars2beats(this.beats2bars(beat).ceil);
	}
	beatInBar { 
		// return the beat of the bar, range is 0 to < t.beatsPerBar
		^this.beats - this.bars2beats(this.bar) 
	}
	
	
		
	// PRIVATE
	prStart { arg tempo;
		_TempoClock_New
		^this.primitiveFailed
	}
	prStop {
		_TempoClock_Free
		^this.primitiveFailed
	}
	prClear {
		_TempoClock_Clear
		^this.primitiveFailed
	}
	setTempoAtBeat { arg newTempo, beats;
		_TempoClock_SetTempoAtBeat
		^this.primitiveFailed
	}
	setTempoAtSec { arg newTempo, secs;
		_TempoClock_SetTempoAtTime
		^this.primitiveFailed
	}
	// meter should only be changed in the TempoClock's thread.
	setMeterAtBeat { arg newBeatsPerBar, beats;
		// bar must be integer valued when meter changes or confusion results later.
		baseBar = round((beats - baseBarBeat) * barsPerBeat + baseBar, 1);
		baseBarBeat = beats;
		beatsPerBar = newBeatsPerBar;
		barsPerBeat = beatsPerBar.reciprocal;
		this.changed; 
	}
}


AppClock : Clock {
	classvar scheduler, thread;
	*initClass {
		scheduler = Scheduler.new(this, true);
		thread =  Routine.new { 
			loop { 
				thisThread.clock = AppClock;
				yield(scheduler.seconds = Main.elapsedTime);
			}
		};
	}
	*sched { arg delta, item;
		scheduler.sched(delta, item)
	}
	*tick {
		thread.next;
	}
	*clear {
		scheduler.clear;
	}
	
	*beats2secs { arg beats; ^beats }
	*secs2beats { arg secs; ^secs }
	beats { ^thisThread.seconds }
	
	// tempo clock compatibility
	*beats { ^thisThread.seconds }
	*beats2bars { ^0 }
	*bars2beats { ^0 }
	*timeToNextBeat { ^0 }
	*nextTimeOnGrid { arg quant = 1, phase = 0;
		^this.beats + phase
	}
}


Scheduler {
	var clock, drift, beats = 0.0, <seconds = 0.0, queue;
	
	*new { arg clock, drift = false;
		^super.newCopyArgs(clock, drift).init;
	}
	init {
		beats = thisThread.beats;
		queue = PriorityQueue.new;
	}

	play { arg task;
		this.sched(task.value(thisThread.beats, thisThread.seconds, clock), task)
	}

	sched { arg delta, item;
		var fromTime;
		if (delta.notNil, { 
			fromTime = if (drift, { Main.elapsedTime },{ seconds });
			queue.put(fromTime + delta, item);
		});
	}
	clear { // adc: priorityqueue has no pairsDo method, array has
		if(queue.array.notNil,{
			queue.array.pairsDo { arg time, item; item.removedFromScheduler };
		});
		queue.clear 
	}

	isEmpty { ^queue.isEmpty }
	
	advance { arg delta;
		this.seconds = seconds + delta;
	}
	
	seconds_ { arg newSeconds;
		var delta, item;
		while ({ 
			seconds = queue.topPriority; 
			seconds.notNil and: { seconds <= newSeconds }
		},{ 
			item = queue.pop;
			delta = item.clock_(clock).awake( beats, seconds, clock );
			if (delta.isNumber, {
				this.sched(delta, item); 
			});
		});
		seconds = newSeconds;
		beats = clock.secs2beats(newSeconds);
	}
}


Demand : MultiOutUGen {
	*ar { arg trig, reset, demandUGens;
		^this.multiNewList(['audio', trig, reset] ++ demandUGens.asArray)
	}
	*kr { arg trig, reset, demandUGens;
		^this.multiNewList(['control', trig, reset] ++ demandUGens.asArray)
	}
	init { arg ... argInputs;
		inputs = argInputs;
		^this.initOutputs(inputs.size - 2, rate)
	}
 	checkInputs { ^this.checkSameRateAsFirstInput }	
}

Duty : UGen {
	
	*ar { arg dur=1.0, reset=0.0, level=1.0, doneAction=0;
		^this.multiNew('audio', dur, reset, doneAction, level)
	}
	*kr { arg dur=1.0, reset=0.0, level=1.0, doneAction=0;
		^this.multiNew('control', dur, reset, doneAction, level)
	}
	checkInputs {
		^if(inputs.at(0).rate === \demand) {
			if (inputs.at(1).rate !== \demand and: { inputs.at(1).rate !== \scalar } and: 
				{ inputs.at(1).rate !== rate }) { 
 				("reset input is not" + rate + "rate: " + inputs.at(1) + inputs.at(1).rate);
 			}
		} {
			this.checkValidInputs
		}
	}
}

TDuty : Duty {
	*ar { arg dur=1.0, reset=0.0, level=1.0, doneAction=0, gapFirst=0;
		^this.multiNew('audio', dur, reset, doneAction, level, gapFirst)
	}
	*kr { arg dur=1.0, reset=0.0, level=1.0, doneAction=0, gapFirst=0;
		^this.multiNew('control', dur, reset, doneAction, level, gapFirst)
	}
}

// old version with gap first
TDuty_old  {
	*ar { arg dur=1.0, reset=0.0, level=1.0, doneAction=0;
		^TDuty.ar(dur, reset, level, doneAction, 1)
	}
	*kr { arg dur=1.0, reset=0.0, level=1.0, doneAction=0;
		^TDuty.kr(dur, reset, level, doneAction, 1)
	}
}

DemandEnvGen : UGen {

	*kr { arg level, dur, shape=1, curve=0, gate=1.0, reset=1.0,
				levelScale = 1.0, levelBias = 0.0, timeScale = 1.0, doneAction=0;
		^this.multiNew('control', level, dur, shape, curve, gate, reset, 
				levelScale, levelBias, timeScale, doneAction)
	}
	*ar { arg level, dur, shape=1, curve=0, gate=1.0, reset=1.0, 
				levelScale = 1.0, levelBias = 0.0, timeScale = 1.0, doneAction=0;
					if(gate.rate === 'audio' or: { reset.rate === 'audio' }) {
						if(gate.rate !== 'audio') { gate = K2A.ar(gate) };
						if(reset.rate !== 'audio') { reset = K2A.ar(reset) };
					};
		^this.multiNew('audio', level, dur, shape, curve, gate, reset, 
				levelScale, levelBias, timeScale, doneAction)
	}
}

DUGen : UGen {
	init { arg ... argInputs;
		super.init(*argInputs);
		this.forceAudioRateInputsIntoUGenGraph;
	}
	forceAudioRateInputsIntoUGenGraph {
 		inputs.do { |in| if(in.rate == \audio) { in <! 0 } };	}
}

Dseries : DUGen {
	*new { arg start = 1, step = 1, length = 100;
		^this.multiNew('demand', length, start, step)
	}	
}

Dgeom : DUGen {
	*new { arg start = 1, grow = 2, length=100;
		^this.multiNew('demand', length, start, grow)
	}	
}

Dbufrd : DUGen {
	*new { arg bufnum=0, phase=0.0, loop=1.0;
		^this.multiNew('demand', bufnum, phase, loop)	
	}
}

Dbufwr : DUGen {
	*new { arg input=0.0, bufnum=0, phase=0.0, loop=1.0;
		^this.multiNew('demand', bufnum, phase, input, loop)
	}
}

ListDUGen : DUGen {
	*new { arg list, repeats=1;
		^this.multiNewList(['demand', repeats] ++ list)
	}
}

Dseq : ListDUGen {}
Dser : ListDUGen {}
Drand : ListDUGen {}
Dxrand : ListDUGen {}

Dswitch1 : DUGen {
	*new { arg list, index;
		^this.multiNewList(['demand', index] ++ list)
	}
}

Dswitch : Dswitch1 {}

Dwhite : DUGen {
	*new { arg lo = 0.0, hi = 1.0, length=inf;
		^this.multiNew('demand', length, lo, hi)
	}
}

Diwhite : Dwhite {}

Dbrown : DUGen {
	*new { arg lo = 0.0, hi = 1.0, step = 0.01, length=inf;
		^this.multiNew('demand', length, lo, hi, step)
	}
}

Dibrown : Dbrown {}

Dstutter : DUGen {
	*new { arg n, in;
		^this.multiNew('demand', n, in)
	}
}

Donce : DUGen {}

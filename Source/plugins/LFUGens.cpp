/*
	SuperCollider real time audio synthesis system
    Copyright (c) 2002 James McCartney. All rights reserved.
	http://www.audiosynth.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/


#include "SC_PlugIn.h"
#include <limits.h>

#ifdef NOVA_SIMD
#include "simd_memory.hpp"
#include "simd_ternary_arithmetic.hpp"
#endif

static InterfaceTable *ft;

struct Vibrato : public Unit
{
	double mPhase, m_attackSlope, m_attackLevel;
	float mFreqMul, m_scaleA, m_scaleB, mFreq;
	int m_delay, m_attack;
};

struct LFPulse : public Unit
{
	double mPhase;
	float mFreqMul, mDuty;
};

struct LFSaw : public Unit
{
	double mPhase;
	float mFreqMul;
};

struct LFPar : public Unit
{
	double mPhase;
	float mFreqMul;
};

struct LFCub : public Unit
{
	double mPhase;
	float mFreqMul;
};

struct LFTri : public Unit
{
	double mPhase;
	float mFreqMul;
};

struct LFGauss : public Unit
{
	double mPhase;
	float mDurMul;
};

struct Impulse : public Unit
{
	double mPhase, mPhaseOffset;
	float mFreqMul;
};

struct VarSaw : public Unit
{
	double mPhase;
	float mFreqMul, mDuty, mInvDuty, mInv1Duty;
};

struct SyncSaw : public Unit
{
	double mPhase1, mPhase2;
	float mFreqMul;
};

struct Line : public Unit
{
	double mLevel, mSlope;
	float mEndLevel;
	int mCounter;
};

struct XLine : public Unit
{
	double mLevel, mGrowth;
	float mEndLevel;
	int mCounter;
};

struct Cutoff : public Unit
{
	double mLevel, mSlope;
	int mWaitCounter;
};

struct LinExp : public Unit
{
	float m_dstratio, m_rsrcrange, m_rrminuslo, m_dstlo;
};

struct LinLin : public Unit
{
	float m_scale, m_offset;
};

struct Clip : public Unit
{
	float m_lo, m_hi;
};

struct Wrap : public Unit
{
	float m_lo, m_hi;
};

struct Fold : public Unit
{
	float m_lo, m_hi, m_range;
};

struct Unwrap : public Unit
{
	float m_range, m_half, m_offset, m_prev;
};

struct AmpComp : public Unit
{
	float m_rootmul, m_exponent;
};

struct AmpCompA : public Unit
{
	double m_scale, m_offset;
};

struct InRange : public Unit
{
	// nothing
};

struct InRect : public Unit
{
	// nothing
};

//struct Trapezoid : public Unit
//{
//	float m_leftScale, m_rightScale, m_a, m_b, m_c, m_d;
//};

struct K2A : public Unit
{
	float mLevel;
};

struct A2K : public Unit
{

};

struct T2K : public Unit
{

};

struct T2A : public Unit
{
	float mLevel;
};

struct DC : public Unit
{
	float m_val;
};

struct Silent : public Unit
{
};

struct EnvGen : public Unit
{
	double m_a1, m_a2, m_b1, m_y1, m_y2, m_grow, m_level, m_endLevel;
	int m_counter, m_stage, m_shape, m_releaseNode;
	float m_prevGate;
	bool m_released;
};

struct BufEnvGen : public EnvGen
{
	SndBuf *m_buf;
	float m_fbufnum;
};

struct Linen : public Unit
{
	float m_endLevel;
	double m_slope, m_level;
	int m_counter, m_stage;
	float m_prevGate;
};

struct ADSR : public Unit
{
	double m_a2, m_b1, m_grow, m_level, m_endLevel;
	int m_counter, m_stage;
	float m_prevGate;
};

//////////////////////////////////////////////////////////////////////////////////////////////////


extern "C"
{
	void load(InterfaceTable *inTable);

	void Vibrato_next(Vibrato *unit, int inNumSamples);
	void Vibrato_Ctor(Vibrato* unit);

	void LFPulse_next_a(LFPulse *unit, int inNumSamples);
	void LFPulse_next_k(LFPulse *unit, int inNumSamples);
	void LFPulse_Ctor(LFPulse* unit);

	void LFSaw_next_a(LFSaw *unit, int inNumSamples);
	void LFSaw_next_k(LFSaw *unit, int inNumSamples);
	void LFSaw_Ctor(LFSaw* unit);

	void LFTri_next_a(LFTri *unit, int inNumSamples);
	void LFTri_next_k(LFTri *unit, int inNumSamples);
	void LFTri_Ctor(LFTri* unit);

	void LFPar_next_a(LFPar *unit, int inNumSamples);
	void LFPar_next_k(LFPar *unit, int inNumSamples);
	void LFPar_Ctor(LFPar* unit);

	void LFCub_next_a(LFCub *unit, int inNumSamples);
	void LFCub_next_k(LFCub *unit, int inNumSamples);
	void LFCub_Ctor(LFCub* unit);

	void LFGauss_next_a(LFGauss *unit, int inNumSamples);
	void LFGauss_next_k(LFGauss *unit, int inNumSamples);
	void LFGauss_next_aa(LFGauss *unit, int inNumSamples);
	void LFGauss_Ctor(LFGauss* unit);
	
	void VarSaw_next_a(VarSaw *unit, int inNumSamples);
	void VarSaw_next_k(VarSaw *unit, int inNumSamples);
	void VarSaw_Ctor(VarSaw* unit);

	void Impulse_next_a(Impulse *unit, int inNumSamples);
	void Impulse_next_kk(Impulse *unit, int inNumSamples);
	void Impulse_next_k(Impulse *unit, int inNumSamples);
	void Impulse_Ctor(Impulse* unit);

	void SyncSaw_next_aa(SyncSaw *unit, int inNumSamples);
	void SyncSaw_next_ak(SyncSaw *unit, int inNumSamples);
	void SyncSaw_next_ka(SyncSaw *unit, int inNumSamples);
	void SyncSaw_next_kk(SyncSaw *unit, int inNumSamples);
	void SyncSaw_Ctor(SyncSaw* unit);

	void K2A_next(K2A *unit, int inNumSamples);
	void K2A_Ctor(K2A* unit);
	
	void A2K_next(A2K *unit, int inNumSamples);
	void A2K_Ctor(A2K* unit);
	
	void T2K_next(T2K *unit, int inNumSamples);
	void T2K_Ctor(T2K* unit);
	
	void T2A_next(T2A *unit, int inNumSamples);
	void T2A_Ctor(T2A* unit);
	
	void DC_next(DC *unit, int inNumSamples);
	void DC_Ctor(DC* unit);

	void Silent_next(Silent *unit, int inNumSamples);
	void Silent_Ctor(Silent* unit);

	void Line_next(Line *unit, int inNumSamples);
	void Line_Ctor(Line* unit);

	void XLine_next(XLine *unit, int inNumSamples);
	void XLine_Ctor(XLine* unit);

	void Wrap_next_kk(Wrap *unit, int inNumSamples);
	void Wrap_next_ak(Wrap *unit, int inNumSamples);
	void Wrap_next_ka(Wrap *unit, int inNumSamples);
	void Wrap_next_aa(Wrap *unit, int inNumSamples);
	void Wrap_Ctor(Wrap* unit);

	void Fold_next_kk(Fold *unit, int inNumSamples);
	void Fold_next_ak(Fold *unit, int inNumSamples);
	void Fold_next_ka(Fold *unit, int inNumSamples);
	void Fold_next_aa(Fold *unit, int inNumSamples);
	void Fold_Ctor(Fold* unit);

	void Clip_next_kk(Clip *unit, int inNumSamples);
	void Clip_next_ka(Clip *unit, int inNumSamples);
	void Clip_next_ak(Clip *unit, int inNumSamples);
	void Clip_next_aa(Clip *unit, int inNumSamples);
	void Clip_Ctor(Clip* unit);

	void Unwrap_next(Unwrap* unit, int inNumSamples);
	void Unwrap_Ctor(Unwrap* unit);

	void AmpComp_next(AmpComp *unit, int inNumSamples);
	void AmpComp_Ctor(AmpComp* unit);
	
	void AmpCompA_next(AmpCompA *unit, int inNumSamples);
	void AmpCompA_Ctor(AmpCompA* unit);

	void InRange_next(InRange *unit, int inNumSamples);
	void InRange_Ctor(InRange* unit);

	void InRect_next(InRect *unit, int inNumSamples);
	void InRect_Ctor(InRect* unit);

	void LinExp_next(LinExp *unit, int inNumSamples);
	void LinExp_next_kk(LinExp *unit, int inNumSamples);
	void LinExp_next_ak(LinExp *unit, int inNumSamples);
	void LinExp_next_ka(LinExp *unit, int inNumSamples);
	void LinExp_Ctor(LinExp* unit);

	void LinLin_next(LinLin *unit, int inNumSamples);
	void LinLin_Ctor(LinLin* unit);

	void EnvGen_next_k(EnvGen *unit, int inNumSamples);
	void EnvGen_next_aa(EnvGen *unit, int inNumSamples);
	void EnvGen_next_ak(EnvGen *unit, int inNumSamples);
	void EnvGen_Ctor(EnvGen *unit);

	void BufEnvGen_next_k(BufEnvGen *unit, int inNumSamples);
	void BufEnvGen_next_aa(BufEnvGen *unit, int inNumSamples);
	void BufEnvGen_next_ak(BufEnvGen *unit, int inNumSamples);
	void BufEnvGen_Ctor(BufEnvGen *unit);

	void Linen_next_k(Linen *unit, int inNumSamples);
	void Linen_Ctor(Linen *unit);

	void ADSR_next_k(ADSR *unit, int inNumSamples);
	void ADSR_Ctor(ADSR *unit);
}


//////////////////////////////////////////////////////////////////////////////////////////////////

// in, rate, depth, rateVariation, depthVariation
// 0   1     2      3              4

void Vibrato_next(Vibrato *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in = ZIN(0);
	
	double ffreq = unit->mFreq;
	double phase = unit->mPhase;
	float scaleA = unit->m_scaleA;
	float scaleB = unit->m_scaleB;
	if (unit->m_delay > 0) 
	{
		int remain = sc_min(inNumSamples, unit->m_delay);
		unit->m_delay -= remain;
		inNumSamples -= remain;
		LOOP(remain, 
			ZXP(out) = ZXP(in);
		);
		if (unit->m_delay <= 0 && inNumSamples > 0) {
			if (unit->m_attack > 0) goto doAttack;
			else goto doNormal;
		}
	}
	else if (unit->m_attack) 
	{
doAttack:
		int remain = sc_min(inNumSamples, unit->m_attack);
		unit->m_attack -= remain;
		inNumSamples -= remain;
		double attackSlope = unit->m_attackSlope;
		double attackLevel = unit->m_attackLevel;
		LOOP(remain, 
			if (phase < 1.f) 
			{
				float z = phase;
				ZXP(out) = ZXP(in) * (1.f + (float)attackLevel * scaleA * (1.f - z * z)) ;
			} 
			else if (phase < 3.f) 
			{
				float z = phase - 2.f;
				ZXP(out) = ZXP(in) * (1.f + (float)attackLevel * scaleB * (z * z - 1.f)) ;
			} 
			else 
			{
				phase -= 4.f;
				float z = phase;
				
				float depth = ZIN0(2);
				float rateVariation = ZIN0(5);
				float depthVariation = ZIN0(6);
				
				float rate = ZIN0(1) * unit->mFreqMul;
				RGen& rgen = *unit->mParent->mRGen;
				ffreq  = rate  * (1.f + rateVariation  * rgen.frand2());
				scaleA = depth * (1.f + depthVariation * rgen.frand2());
				scaleB = depth * (1.f + depthVariation * rgen.frand2());
	
				ZXP(out) = ZXP(in) * (1.f + (float)attackLevel * scaleA * (1.f - z * z)) ;
			}
			phase += ffreq;
			attackLevel += attackSlope;
		);
		unit->m_attackLevel = attackLevel;
		if (unit->m_attack <= 0 && inNumSamples > 0) goto doNormal;
	}
	else 
	{
doNormal:
		LOOP(inNumSamples, 
			if (phase < 1.f) 
			{
				float z = phase;
				ZXP(out) = ZXP(in) * (1.f + scaleA * (1.f - z * z)) ;
			} 
			else if (phase < 3.f) 
			{
				float z = phase - 2.f;
				ZXP(out) = ZXP(in) * (1.f + scaleB * (z * z - 1.f)) ;
			} 
			else 
			{
				phase -= 4.f;
				float z = phase;
				
				float depth = ZIN0(2);
				float rateVariation = ZIN0(5);
				float depthVariation = ZIN0(6);
				
				float rate = ZIN0(1) * unit->mFreqMul;
				RGen& rgen = *unit->mParent->mRGen;
				ffreq  = rate  * (1.f + rateVariation  * rgen.frand2());
				scaleA = depth * (1.f + depthVariation * rgen.frand2());
				scaleB = depth * (1.f + depthVariation * rgen.frand2());
	
				ZXP(out) = ZXP(in) * (1.f + scaleA * (1.f - z * z)) ;
			}
			phase += ffreq;
		);
	}
	unit->mPhase = phase;
	unit->mFreq = ffreq;
	unit->m_scaleA = scaleA;
	unit->m_scaleB = scaleB;
	
}

void Vibrato_Ctor(Vibrato* unit)
{	
	unit->mFreqMul = 4.0 * SAMPLEDUR;
	unit->mPhase = 4.0 * sc_wrap(ZIN0(7), 0.f, 1.f) - 1.0;

	RGen& rgen = *unit->mParent->mRGen;
	float rate = ZIN0(1) * unit->mFreqMul;
	float depth = ZIN0(2);
	float rateVariation = ZIN0(5);
	float depthVariation = ZIN0(6);
	unit->mFreq    = rate  * (1.f + rateVariation  * rgen.frand2());
	unit->m_scaleA = depth * (1.f + depthVariation * rgen.frand2());
	unit->m_scaleB = depth * (1.f + depthVariation * rgen.frand2());
	unit->m_delay = (int)(ZIN0(3) * SAMPLERATE);
	unit->m_attack = (int)(ZIN0(4) * SAMPLERATE);
	unit->m_attackSlope = 1. / (double)(1 + unit->m_attack);
	unit->m_attackLevel = unit->m_attackSlope;
	
	SETCALC(Vibrato_next);
	Vibrato_next(unit, 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////


void LFPulse_next_a(LFPulse *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *freq = ZIN(0);
	float nextDuty = ZIN0(2);
	float duty = unit->mDuty;
	
	float freqmul = unit->mFreqMul;
	double phase = unit->mPhase;
	LOOP(inNumSamples, 
		float z;
		if (phase >= 1.f) {
			phase -= 1.f;
			duty = unit->mDuty = nextDuty;
			// output at least one sample from the opposite polarity
			z = duty < 0.5 ? 1.f : 0.f;
		} else {
			z = phase < duty ? 1.f : 0.f;
		}
		phase += ZXP(freq) * freqmul;
		ZXP(out) = z;
	);

	unit->mPhase = phase;
}

void LFPulse_next_k(LFPulse *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float freq = ZIN0(0) * unit->mFreqMul;
	float nextDuty = ZIN0(2);
	float duty = unit->mDuty;
	
	double phase = unit->mPhase;
	LOOP(inNumSamples, 
		float z;
		if (phase >= 1.f) {
			phase -= 1.f;
			duty = unit->mDuty = nextDuty;
			// output at least one sample from the opposite polarity
			z = duty < 0.5 ? 1.f : 0.f;
		} else {
			z = phase < duty ? 1.f : 0.f;
		}
		phase += freq;
		ZXP(out) = z;
	);

	unit->mPhase = phase;
}

void LFPulse_Ctor(LFPulse* unit)
{
	if (INRATE(0) == calc_FullRate) {
		SETCALC(LFPulse_next_a);
	} else {
		SETCALC(LFPulse_next_k);
	}

	unit->mFreqMul = unit->mRate->mSampleDur;
	unit->mPhase = ZIN0(1);
	unit->mDuty = ZIN0(2);
	
	LFPulse_next_k(unit, 1);

}


//////////////////////////////////////////////////////////////////////////////////////////////////

void LFSaw_next_a(LFSaw *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *freq = ZIN(0);
	
	float freqmul = unit->mFreqMul;
	double phase = unit->mPhase;
	LOOP(inNumSamples, 
		float z = phase; // out must be written last for in place operation
		phase += ZXP(freq) * freqmul;
		if (phase >= 1.f) phase -= 2.f;
		else if (phase <= -1.f) phase += 2.f;
		ZXP(out) = z;
	);

	unit->mPhase = phase;
	
}

void LFSaw_next_k(LFSaw *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float freq = ZIN0(0) * unit->mFreqMul;
	
	double phase = unit->mPhase;
	if (freq >= 0.f) {
		LOOP(inNumSamples, 
			ZXP(out) = phase;
			phase += freq;
			if (phase >= 1.f) phase -= 2.f;
		);
	} else {
		LOOP(inNumSamples, 
			ZXP(out) = phase;
			phase += freq;
			if (phase <= -1.f) phase += 2.f;
		);
	}

	unit->mPhase = phase;
}

void LFSaw_Ctor(LFSaw* unit)
{
	if (INRATE(0) == calc_FullRate) {
		SETCALC(LFSaw_next_a);
	} else {
		SETCALC(LFSaw_next_k);
	}

	unit->mFreqMul = 2.0 * unit->mRate->mSampleDur;
	unit->mPhase = ZIN0(1);
	
	LFSaw_next_k(unit, 1);

}



//////////////////////////////////////////////////////////////////////////////////////////////////

void LFPar_next_a(LFPar *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *freq = ZIN(0);
	
	float freqmul = unit->mFreqMul;
	double phase = unit->mPhase;
	LOOP(inNumSamples, 
		if (phase < 1.f) {
			float z = phase;
			ZXP(out) = 1.f - z*z;
		} else if (phase < 3.f) {
			float z = phase - 2.f;
			ZXP(out) = z*z - 1.f;
		} else {
			phase -= 4.f;
			float z = phase;
			ZXP(out) = 1.f - z*z;
		}
		phase += ZXP(freq) * freqmul;
	);

	unit->mPhase = phase;
	
}

void LFPar_next_k(LFPar *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float freq = ZIN0(0) * unit->mFreqMul;
	
	double phase = unit->mPhase;
	LOOP(inNumSamples, 
		if (phase < 1.f) {
			float z = phase;
			ZXP(out) = 1.f - z*z;
		} else if (phase < 3.f) {
			float z = phase - 2.f;
			ZXP(out) = z*z - 1.f;
		} else {
			phase -= 4.f;
			float z = phase;
			ZXP(out) = 1.f - z*z;
		}
		phase += freq;
	);

	unit->mPhase = phase;
}

void LFPar_Ctor(LFPar* unit)
{
	if (INRATE(0) == calc_FullRate) {
		SETCALC(LFPar_next_a);
	} else {
		SETCALC(LFPar_next_k);
	}

	unit->mFreqMul = 4.0 * unit->mRate->mSampleDur;
	unit->mPhase = ZIN0(1);
	
	LFPar_next_k(unit, 1);

}



//////////////////////////////////////////////////////////////////////////////////////////////////

void LFCub_next_a(LFCub *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *freq = ZIN(0);
	
	float freqmul = unit->mFreqMul;
	double phase = unit->mPhase;
	LOOP(inNumSamples, 
		float z;
		if (phase < 1.f) {
			z = phase;
		} else if (phase < 2.f) {
			z = 2.f - phase;
		} else {
			phase -= 2.f;
			z = phase;
		}
		ZXP(out) = z * z * (6.f - 4.f * z) - 1.f;
		phase += ZXP(freq) * freqmul;
	);

	unit->mPhase = phase;
	
}

void LFCub_next_k(LFCub *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float freq = ZIN0(0) * unit->mFreqMul;
	
	double phase = unit->mPhase;
	LOOP(inNumSamples, 
		float z;
		if (phase < 1.f) {
			z = phase;
		} else if (phase < 2.f) {
			z = 2.f - phase;
		} else {
			phase -= 2.f;
			z = phase;
		}
		ZXP(out) = z * z * (6.f - 4.f * z) - 1.f;
		phase += freq;
	);

	unit->mPhase = phase;
}

void LFCub_Ctor(LFCub* unit)
{
	if (INRATE(0) == calc_FullRate) {
		SETCALC(LFCub_next_a);
	} else {
		SETCALC(LFCub_next_k);
	}

	unit->mFreqMul = 2.0 * unit->mRate->mSampleDur;
	unit->mPhase = ZIN0(1) + 0.5;
	
	LFCub_next_k(unit, 1);

}



//////////////////////////////////////////////////////////////////////////////////////////////////

void LFTri_next_a(LFTri *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *freq = ZIN(0);
	
	float freqmul = unit->mFreqMul;
	double phase = unit->mPhase;
	LOOP(inNumSamples, 
		float z = phase > 1.f ? 2.f - phase : phase;
		phase += ZXP(freq) * freqmul;
		if (phase >= 3.f) phase -= 4.f;
		ZXP(out) = z;
	);

	unit->mPhase = phase;
	
}

void LFTri_next_k(LFTri *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float freq = ZIN0(0) * unit->mFreqMul;
	
	double phase = unit->mPhase;
	LOOP(inNumSamples, 
		float z = phase > 1.f ? 2.f - phase : phase;
		phase += freq;
		if (phase >= 3.f) phase -= 4.f;
		ZXP(out) = z;
	);

	unit->mPhase = phase;
}

void LFTri_Ctor(LFTri* unit)
{
	if (INRATE(0) == calc_FullRate) {
		SETCALC(LFTri_next_a);
	} else {
		SETCALC(LFTri_next_k);
	}

	unit->mFreqMul = 4.0 * unit->mRate->mSampleDur;
	unit->mPhase = ZIN0(1);
	
	LFTri_next_k(unit, 1);

}


//////////////////////////////////////////////////////////////////////////////////////////////////

void LFGauss_next_k(LFGauss *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	
	float dur = ZIN0(0);
	float c = ZIN0(1);
	float b = ZIN0(2);
	float loop = ZIN0(3);
	
	// offset phase by b
	double x = unit->mPhase - b;
	
	// for a full cycle from -1 to 1 in duration, double the step.
	float step = 2.f / (dur * unit->mRate->mSampleRate);
	
	// calculate exponent only once per loop
	float factor = -1.f / (2.f * c * c);
		
	LOOP(inNumSamples, 
		 
		if (x > 1.f) {
			if(loop) { x -= 2.f; } else { DoneAction(ZIN0(4), unit); }
		}
		ZXP(out) = exp(x * x * factor);
		x += step;
	);

	unit->mPhase = x + b;
	
}

void LFGauss_next_a(LFGauss *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	
	float *dur = ZIN(0);
	
	float c = ZIN0(1);
	float b = ZIN0(2);
	float loop = ZIN0(3);
	float sr = unit->mRate->mSampleRate;
	
	// offset phase by b
	double x = unit->mPhase - b;
	float factor = -1.f / (2.f * c * c);
	
	LOOP(inNumSamples, 
		
		if (x > 1.f) {
			if(loop) { x -= 2.f; } else { DoneAction(ZIN0(4), unit); }
		}
		
		// for a full cycle from -1 to 1 in duration, double the step.
		float step = 2.f / (ZXP(dur) * sr);
		
		ZXP(out) = exp(x * x * factor);
		
		x += step;
	);

	unit->mPhase = x + b;
	
}



void LFGauss_next_aa(LFGauss *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	
	float *dur = ZIN(0);
	float *c = ZIN(1);
	
	float b = ZIN0(2);
	float loop = ZIN0(3);
	float sr = unit->mRate->mSampleRate;
	
	// offset phase by b
	double x = unit->mPhase - b;
	
	LOOP(inNumSamples, 
		
		if (x > 1.f) {
			if(loop) { x -= 2.f; } else { DoneAction(ZIN0(4), unit); }
		}
		
		// for a full cycle from -1 to 1 in duration, double the step.
		float step = 2.f / (ZXP(dur) * sr);
		
		float cval = ZXP(c);
		
		float factor = -1.f / (2.f * cval * cval);
		ZXP(out) = exp(x * x * factor);
		
		x += step;
	);

	unit->mPhase = x + b;
	
}



void LFGauss_Ctor(LFGauss* unit)
{
	
	if (INRATE(0) == calc_FullRate) {
		if (INRATE(1) == calc_FullRate) {
				SETCALC(LFGauss_next_aa);
		} else {
				SETCALC(LFGauss_next_a);
				printf("LFGauss_next_a\n");
		}
	} else {
		SETCALC(LFGauss_next_k);
	}
	unit->mPhase = -1.0;
	
	//LFGauss_next_k(unit, 1);

}


//////////////////////////////////////////////////////////////////////////////////////////////////

void Impulse_next_a(Impulse *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *freq = ZIN(0);
	
	float freqmul = unit->mFreqMul;
	double phase = unit->mPhase;
	LOOP(inNumSamples, 
		float z;
		if (phase >= 1.f) {
			phase -= 1.f;
			z = 1.f;
		} else {
			z = 0.f;
		}
		phase += ZXP(freq) * freqmul;
		ZXP(out) = z;
	);

	unit->mPhase = phase;
	
}

/* phase mod - jrh 03 */

void Impulse_next_ak(Impulse *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *freq = ZIN(0);
	double phaseOffset =  ZIN0(1);
	
	float freqmul = unit->mFreqMul;
	double phase = unit->mPhase;
	double prev_phaseOffset = unit->mPhaseOffset;
	double phaseSlope = CALCSLOPE(phaseOffset, prev_phaseOffset);
	phase += prev_phaseOffset;
	
	LOOP(inNumSamples, 
		float z;
		phase += phaseSlope;
		if (phase >= 1.f) {
			phase -= 1.f;
			z = 1.f;
		} else {
			z = 0.f;
		}
		phase += ZXP(freq) * freqmul;
		ZXP(out) = z;
	);

	unit->mPhase = phase - phaseOffset;
	unit->mPhaseOffset = phaseOffset;
}

void Impulse_next_kk(Impulse *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float freq = ZIN0(0) * unit->mFreqMul;
	double phaseOffset =  ZIN0(1);
	
	double phase = unit->mPhase;
	double prev_phaseOffset = unit->mPhaseOffset;
	double phaseSlope = CALCSLOPE(phaseOffset, prev_phaseOffset);
	phase += prev_phaseOffset;
	
	LOOP(inNumSamples, 
		float z;
		phase += phaseSlope;
		if (phase >= 1.f) {
			phase -= 1.f;
			z = 1.f;
		} else {
			z = 0.f;
		}
		phase += freq;
		ZXP(out) = z;
	);

	unit->mPhase = phase - phaseOffset;
	unit->mPhaseOffset = phaseOffset;
}


void Impulse_next_k(Impulse *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float freq = ZIN0(0) * unit->mFreqMul;
	
	double phase = unit->mPhase;
	LOOP(inNumSamples, 
		float z;
		if (phase >= 1.f) {
			phase -= 1.f;
			z = 1.f;
		} else {
			z = 0.f;
		}
		phase += freq;
		ZXP(out) = z;
	);

	unit->mPhase = phase;
}

void Impulse_Ctor(Impulse* unit)
{
	
	unit->mPhase = ZIN0(1);
	
	if (INRATE(0) == calc_FullRate) {
		if(INRATE(1) != calc_ScalarRate) {
			SETCALC(Impulse_next_ak);
			unit->mPhase = 1.f;
		} else {
			SETCALC(Impulse_next_a);
		}
	} else {
		if(INRATE(1) != calc_ScalarRate) {
			SETCALC(Impulse_next_kk);
			unit->mPhase = 1.f;
		} else {
			SETCALC(Impulse_next_k);
		}
	}
	
	
	unit->mPhaseOffset = 0.f;
	unit->mFreqMul = unit->mRate->mSampleDur;
	if (unit->mPhase == 0.f) unit->mPhase = 1.f;
	
	ZOUT0(0) = 0.f;

}

//////////////////////////////////////////////////////////////////////////////////////////////////

void VarSaw_next_a(VarSaw *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *freq = ZIN(0);
	float nextDuty = ZIN0(2);
	float duty = unit->mDuty;
	float invduty = unit->mInvDuty;
	float inv1duty = unit->mInv1Duty;
	
	float freqmul = unit->mFreqMul;
	double phase = unit->mPhase;
	
	LOOP(inNumSamples, 
		if (phase >= 1.f) {
			phase -= 1.f;
			duty = unit->mDuty = sc_clip(nextDuty, 0.001, 0.999);
			invduty = unit->mInvDuty = 2.f / duty;
			inv1duty = unit->mInv1Duty = 2.f / (1.f - duty);
		}
		float z = phase < duty ? phase * invduty : (1.f - phase) * inv1duty;
		phase += ZXP(freq) * freqmul;
		ZXP(out) = z - 1.f;
	);

	unit->mPhase = phase;
	
}

void VarSaw_next_k(VarSaw *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float freq = ZIN0(0) * unit->mFreqMul;
	float nextDuty = ZIN0(2);
	float duty = unit->mDuty;
	float invduty = unit->mInvDuty;
	float inv1duty = unit->mInv1Duty;
	
	double phase = unit->mPhase;
	
	LOOP(inNumSamples, 
		if (phase >= 1.f) {
			phase -= 1.f;
			duty = unit->mDuty = sc_clip(nextDuty, 0.001, 0.999);
			invduty = unit->mInvDuty = 2.f / duty;
			inv1duty = unit->mInv1Duty = 2.f / (1.f - duty);
		}
		float z = phase < duty ? phase * invduty : (1.f - phase) * inv1duty;
		phase += freq;
		ZXP(out) = z - 1.f;
	);

	unit->mPhase = phase;
}

void VarSaw_Ctor(VarSaw* unit)
{
	if (INRATE(0) == calc_FullRate) {
		SETCALC(VarSaw_next_a);
	} else {
		SETCALC(VarSaw_next_k);
	}

	unit->mFreqMul = unit->mRate->mSampleDur;
	unit->mPhase = ZIN0(1);
	float duty = ZIN0(2);
	duty = unit->mDuty = sc_clip(duty, 0.001, 0.999);
	unit->mInvDuty = 2.f / duty;
	unit->mInv1Duty = 2.f / (1.f - duty);
	
	ZOUT0(0) = 0.f;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SyncSaw_next_aa(SyncSaw *unit, int inNumSamples)
{
	float freqmul = unit->mFreqMul;
	float *out = ZOUT(0);
	float *freq1 = ZIN(0);
	float *freq2 = ZIN(1);
	
	double phase1 = unit->mPhase1;
	double phase2 = unit->mPhase2;
	
	LOOP(inNumSamples, 
		float freq1x = ZXP(freq1) * freqmul;
		float freq2x = ZXP(freq2) * freqmul;
		float z = phase2;
		phase2 += freq2x;
		if (phase2 >= 1.f) phase2 -= 2.f;
		phase1 += freq1x;
		if (phase1 >= 1.f) {
			phase1 -= 2.f;
			phase2 = (phase1 + 1.f) * freq2x / freq1x - 1.f;
		}
		ZXP(out) = z;
	);

	unit->mPhase1 = phase1;
	unit->mPhase2 = phase2;
}

void SyncSaw_next_ak(SyncSaw *unit, int inNumSamples)
{
	float freqmul = unit->mFreqMul;
	float *out = ZOUT(0);
	float *freq1 = ZIN(0);
	float freq2x = ZIN0(1) * freqmul;
	
	double phase1 = unit->mPhase1;
	double phase2 = unit->mPhase2;
	
	LOOP(inNumSamples, 
		float freq1x = ZXP(freq1) * freqmul;
		float z = phase2;
		phase2 += freq2x;
		if (phase2 >= 1.f) phase2 -= 2.f;
		phase1 += freq1x;
		if (phase1 >= 1.f) {
			phase1 -= 2.f;
			phase2 = (phase1 + 1.f) * freq2x / freq1x - 1.f;
		}
		ZXP(out) = z;
	);

	unit->mPhase1 = phase1;
	unit->mPhase2 = phase2;
}

void SyncSaw_next_ka(SyncSaw *unit, int inNumSamples)
{
	float freqmul = unit->mFreqMul;
	float *out = ZOUT(0);
	float freq1x = ZIN0(0) * freqmul;
	float *freq2 = ZIN(1);
	
	double phase1 = unit->mPhase1;
	double phase2 = unit->mPhase2;
	
	LOOP(inNumSamples, 
		float freq2x = ZXP(freq2) * freqmul;
		float z = phase2;
		phase2 += freq2x;
		if (phase2 >= 1.f) phase2 -= 2.f;
		phase1 += freq1x;
		if (phase1 >= 1.f) {
			phase1 -= 2.f;
			phase2 = (phase1 + 1.f) * freq2x / freq1x - 1.f;
		}
		ZXP(out) = z;
	);

	unit->mPhase1 = phase1;
	unit->mPhase2 = phase2;
}

void SyncSaw_next_kk(SyncSaw *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float freq1x = ZIN0(0) * unit->mFreqMul;
	float freq2x = ZIN0(1) * unit->mFreqMul;
	double phase1 = unit->mPhase1;
	double phase2 = unit->mPhase2;
	
	LOOP(inNumSamples, 
		float z = phase2;
		phase2 += freq2x;
		if (phase2 >= 1.f) phase2 -= 2.f;
		phase1 += freq1x;
		if (phase1 >= 1.f) {
			phase1 -= 2.f;
			phase2 = (phase1 + 1.f) * freq2x / freq1x - 1.f;
		}
		ZXP(out) = z;
	);

	unit->mPhase1 = phase1;
	unit->mPhase2 = phase2;

}

void SyncSaw_Ctor(SyncSaw* unit)
{
	if (INRATE(0) == calc_FullRate) {
		if (INRATE(1) == calc_FullRate) {
			SETCALC(SyncSaw_next_aa);
		} else {
			SETCALC(SyncSaw_next_ak);
		}
	} else {
		if (INRATE(1) == calc_FullRate) {
			SETCALC(SyncSaw_next_ka);
		} else {
			SETCALC(SyncSaw_next_kk);
		}
	}
	unit->mFreqMul = 2.0 * unit->mRate->mSampleDur;
	unit->mPhase1 = 0.;
	unit->mPhase2 = 0.;
	
	SyncSaw_next_kk(unit, 1);

}

//////////////////////////////////////////////////////////////////////////////////////////////////

void K2A_next(K2A *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float in = ZIN0(0);
	
	float level = unit->mLevel;
	float slope = CALCSLOPE(in, level);
	
	LOOP(inNumSamples, 
		ZXP(out) = level += slope;
	);
	unit->mLevel = level;
}

#ifdef NOVA_SIMD
void K2A_next_nova(K2A *unit, int inNumSamples)
{
	float in = ZIN0(0);
	float level = unit->mLevel;

	if (level == in)
		nova::setvec_simd(OUT(0), level, inNumSamples);
	else
	{
		float slope = CALCSLOPE(in, level);
		nova::set_slope_vec_simd(OUT(0), level, slope, inNumSamples);
	}

	unit->mLevel = in;
}
#endif

void K2A_Ctor(K2A* unit)
{
#ifdef NOVA_SIMD
	if (!(BUFLENGTH & 15))
		SETCALC(K2A_next_nova);
	else
#endif
	SETCALC(K2A_next);
	unit->mLevel = ZIN0(0);
	
	ZOUT0(0) = unit->mLevel;

}

//////////////////////////////////////////////////////////////////////////////////////////////////

void A2K_next(A2K *unit, int inNumSamples)
{
	ZOUT0(0) = ZIN0(0); // return first sample in block
}

void A2K_Ctor(A2K* unit)
{
	SETCALC(A2K_next);
	A2K_next(unit, 1);
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void T2K_next(T2K *unit, int inNumSamples)
{
	float max = 0.f, zout = 0.f;
	int n;
	n = (int)unit->mWorld->mBufRate.mSampleRate;
	for (int i=0; i<n; ++i) {
			float zin = IN(0)[i];
			if(fabs(zin) > max) { 
				zout = zin; 
				max = fabs(zin); 
			}
	}
	ZOUT0(0) = zout;
}

void T2K_Ctor(T2K* unit)
{
	SETCALC(T2K_next);
	T2K_next(unit, 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void T2A_next(T2A *unit, int inNumSamples)
{
	float level = IN0(0);
	int offset = (int) IN0(1);
	float *out = ZOUT(0);


	if((unit->mLevel <= 0.f && level > 0.f)) {
		for(int i = 0; i < inNumSamples; i++) {
			if(i == offset) { 
				ZXP(out) = level; 
			} else { 
				ZXP(out) = 0.f; 
			}
		}
	} else {
		LOOP(inNumSamples, ZXP(out) = 0.f;)
	}
	
	unit->mLevel = level;
	
}

#ifdef NOVA_SIMD
void T2A_next_nova(T2A *unit, int inNumSamples)
{
	float level = IN0(0);
	int offset = (int) IN0(1);

	if((unit->mLevel <= 0.f && level > 0.f)) {
		float *out = ZOUT(0);
		for(int i = 0; i < inNumSamples; i++) {
			if(i == offset) {
				ZXP(out) = level;
			} else {
				ZXP(out) = 0.f;
			}
		}
	} else {
		nova::zerovec_simd(OUT(0), inNumSamples);
	}

	unit->mLevel = level;
}
#endif

void T2A_Ctor(T2A* unit)
{
#ifdef NOVA_SIMD
	if (!(BUFLENGTH & 15))
		SETCALC(T2A_next_nova);
	else
#endif
    SETCALC(T2A_next);
	T2A_next(unit, 1);
}


//////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef NOVA_SIMD
void DC_next_nova(DC *unit, int inNumSamples)
{
	float val = unit->m_val;
	nova::setvec_simd(OUT(0), val, inNumSamples);
}
#endif

void DC_Ctor(DC* unit)
{
	unit->m_val = IN0(0);
#ifdef NOVA_SIMD
	if (!(BUFLENGTH & 15))
		SETCALC(DC_next_nova);
	else
#endif
	SETCALC(DC_next);
	ZOUT0(0) = unit->m_val;
}

void DC_next(DC *unit, int inNumSamples)
{
	float val = unit->m_val;
	float *out = ZOUT(0);
	LOOP(inNumSamples, ZXP(out) = val;)
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Silent_Ctor(Unit* unit)
{
	SETCALC(ClearUnitOutputs);
	ZOUT0(0) = 0.f;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Line_next(Line *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	
	double slope = unit->mSlope;
	double level = unit->mLevel;
	int counter = unit->mCounter;
	
	int remain = inNumSamples;
	do {
		if (counter==0) {
			int nsmps = remain;
			remain = 0;
			float endlevel = unit->mEndLevel;
			LOOP(nsmps, 
				ZXP(out) = endlevel;
			);
		} else {
			int nsmps = sc_min(remain, counter);
			counter -= nsmps;
			remain -= nsmps;
			LOOP(nsmps, 
				ZXP(out) = level;
				level += slope;
			);
			if (counter == 0) {
				unit->mDone = true;
				int doneAction = (int)ZIN0(3);
				DoneAction(doneAction, unit);
			}
		}
	} while (remain);
	unit->mCounter = counter;
	unit->mLevel = level;

}

#ifdef NOVA_SIMD
void Line_next_nova(Line *unit, int inNumSamples)
{
	float *out = ZOUT(0);

	double slope = unit->mSlope;
	double level = unit->mLevel;
	int counter = unit->mCounter;

	int remain = inNumSamples;
	if (counter == 0)
	{
		nova::setvec_simd(OUT(0), unit->mEndLevel, inNumSamples);
		return;
	}

	if (counter > inNumSamples)
	{
		nova::set_slope_vec_simd(OUT(0), (float)level, (float)slope, inNumSamples);
		unit->mLevel = level + inNumSamples * slope;
		unit->mCounter = counter - inNumSamples;
		return;
	}

	do {
		if (counter==0) {
			int nsmps = remain;
			remain = 0;
			float endlevel = unit->mEndLevel;
			LOOP(nsmps,
				ZXP(out) = endlevel;
			);
		} else {
			int nsmps = sc_min(remain, counter);
			counter -= nsmps;
			remain -= nsmps;
			LOOP(nsmps,
				ZXP(out) = level;
				level += slope;
			);
			if (counter == 0) {
				unit->mDone = true;
				int doneAction = (int)ZIN0(3);
				DoneAction(doneAction, unit);
			}
		}
	} while (remain);
	unit->mCounter = counter;
	unit->mLevel = level;

}
#endif

void Line_Ctor(Line* unit)
{
#ifdef NOVA_SIMD
	if (!(BUFLENGTH & 15))
		SETCALC(Line_next_nova);
	else
#endif
	SETCALC(Line_next);
	double start = ZIN0(0);
	double end = ZIN0(1);
	double dur = ZIN0(2);
	
	int counter = (int)(dur * unit->mRate->mSampleRate + .5f);
	unit->mCounter = sc_max(1, counter);
	if(counter == 0){
	    unit->mLevel = end;
	    unit->mSlope = 0.;
	    } else {
	    unit->mLevel = start;
	    unit->mSlope = (end - start) / unit->mCounter;
	    }
	unit->mEndLevel = end;
	ZOUT0(0) = unit->mLevel;
	unit->mLevel += unit->mSlope;

}

//////////////////////////////////////////////////////////////////////////////////////////////////


void XLine_next(XLine *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	
	double grow = unit->mGrowth;
	double level = unit->mLevel;
	int counter = unit->mCounter;
	
	int remain = inNumSamples;
	do {
		if (counter==0) {
			int nsmps = remain;
			remain = 0;
			float endlevel = unit->mEndLevel;
			LOOP(nsmps, 
				ZXP(out) = endlevel;
			);
		} else {
			int nsmps = sc_min(remain, counter);
			counter -= nsmps;
			remain -= nsmps;
			LOOP(nsmps, 
				ZXP(out) = level;
				level *= grow;
			);
			if (counter == 0) {
				unit->mDone = true;
				int doneAction = (int)ZIN0(3);
				DoneAction(doneAction, unit);
			}
		}
	} while (remain);
	unit->mCounter = counter;
	unit->mLevel = level;
}

#ifdef NOVA_SIMD
void XLine_next_nova(XLine *unit, int inNumSamples)
{
	float *out = ZOUT(0);

	double grow = unit->mGrowth;
	double level = unit->mLevel;
	int counter = unit->mCounter;

	int remain = inNumSamples;

	if (counter == 0)
	{
		nova::setvec_simd(OUT(0), level, inNumSamples);
		return;
	}
	if (counter >= inNumSamples)
	{
		nova::set_exp_vec_simd(OUT(0), (float)level, (float)grow, inNumSamples);
		level *= sc_powi(grow, inNumSamples);
		counter -= inNumSamples;
	}
	else
	{
		do {
			if (counter==0) {
				int nsmps = remain;
				remain = 0;
				float endlevel = unit->mEndLevel;
				LOOP(nsmps,
					ZXP(out) = endlevel;
					);
			} else {
				int nsmps = sc_min(remain, counter);
				counter -= nsmps;
				remain -= nsmps;
				LOOP(nsmps,
					ZXP(out) = level;
					 level *= grow;
					);
				if (counter == 0) {
					unit->mDone = true;
					int doneAction = (int)ZIN0(3);
					DoneAction(doneAction, unit);
				}
			}
		} while (remain);
	}
	unit->mCounter = counter;
	unit->mLevel = level;
}
#endif

void XLine_Ctor(XLine* unit)
{
#ifdef NOVA_SIMD
	if (!(BUFLENGTH & 15))
		SETCALC(XLine_next_nova);
	else
#endif

	SETCALC(XLine_next);
	double start = ZIN0(0);
	double end = ZIN0(1);
	double dur = ZIN0(2);
	
	int counter = (int)(dur * unit->mRate->mSampleRate + .5f);
	unit->mCounter = sc_max(1, counter);
	unit->mGrowth = std::pow(end / start, 1.0 / counter);
	unit->mLevel = start;
	unit->mEndLevel = end;
	
	ZOUT0(0) = unit->mLevel;
	unit->mLevel *= unit->mGrowth;

}

//////////////////////////////////////////////////////////////////////////////////////////////////
/*
void Wrap_next(Wrap* unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in   = ZIN(0);
	float lo = unit->m_lo;
	float hi = unit->m_hi;
	float range = unit->m_range;
	
	LOOP(inNumSamples, 
		ZXP(out) = sc_wrap(ZXP(in), lo, hi, range);
	);
}

void Wrap_Ctor(Wrap* unit)
{

	SETCALC(Wrap_next);
	unit->m_lo = ZIN0(1);
	unit->m_hi = ZIN0(2);
	
	if (unit->m_lo > unit->m_hi) {
		float temp = unit->m_lo;
		unit->m_lo = unit->m_hi;
		unit->m_hi = temp;
	}
	unit->m_range = unit->m_hi - unit->m_lo;
	
	Wrap_next(unit, 1);
}
*/


void Wrap_next_kk(Wrap* unit, int inNumSamples)
{
    float *out = OUT(0);
    float *in   = IN(0);
    float next_lo = IN0(1);
    float next_hi = IN0(2);
    float lo = unit->m_lo;
    float lo_slope = CALCSLOPE(next_lo, lo);
    float hi = unit->m_hi;
    float hi_slope = CALCSLOPE(next_hi, hi);
    float range;
    for(int i = 0; i < inNumSamples; i++){
	range = hi - lo;
	out[i] = sc_wrap(in[i], lo, hi, range);
	lo += lo_slope;
	hi += hi_slope;
    };
    unit->m_lo = lo;
    unit->m_hi = hi;
}

void Wrap_next_ka(Wrap* unit, int inNumSamples)
{
    float *out = OUT(0);
    float *in   = IN(0);
    float next_lo = IN0(1);
    float *hi = IN(2);
    float lo = unit->m_lo;
    float lo_slope = CALCSLOPE(next_lo, lo);
    float curhi;
    for(int i = 0; i < inNumSamples; i++){ 
	curhi = hi[i];
	out[i] = sc_wrap(in[i], lo, curhi, curhi - lo);
	lo += lo_slope;
    };
    unit->m_lo = lo;
}

void Wrap_next_ak(Wrap* unit, int inNumSamples)
{
    float *out = OUT(0);
    float *in   = IN(0);
    float *lo = IN(1);
    float next_hi = IN0(2);
    float hi = unit->m_hi;
    float hi_slope = CALCSLOPE(next_hi, hi);
    float curlo;
    
    for(int i = 0; i < inNumSamples; i++){ 
	curlo = lo[i];
	out[i] = sc_wrap(in[i], curlo, hi, hi - curlo);
	hi += hi_slope;
    };
    unit->m_hi = hi;
}

void Wrap_next_aa(Wrap* unit, int inNumSamples)
{
    float *out = OUT(0);
    float *in   = IN(0);
    float *lo = IN(1);
    float *hi = IN(2);
    float curlo, curhi;
    
    for(int i = 0; i < inNumSamples; i++){ 
	curhi = hi[i];
	curlo = lo[i];
	out[i] = sc_wrap(in[i], curlo, curhi, curhi - curlo);
    };
}

void Wrap_Ctor(Wrap* unit)
{
    
    if(INRATE(1) == calc_FullRate){
	if(INRATE(2) == calc_FullRate) {
	    SETCALC(Wrap_next_aa);
	} else {
	    SETCALC(Wrap_next_ak);
	} 
    } else {
	if(INRATE(2) == calc_FullRate) {
	    SETCALC(Wrap_next_ka);
	} else {
	    SETCALC(Wrap_next_kk);
	}
    }
    
    unit->m_lo = ZIN0(1);
    unit->m_hi = ZIN0(2);
    
    Wrap_next_kk(unit, 1);
}


//////////////////////////////////////////////////////////////////////////////////////////////////
/*
void Fold_next(Fold* unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in   = ZIN(0);
	float lo = unit->m_lo;
	float hi = unit->m_hi;
	float range = unit->m_range;
	float range2 = unit->m_range2;
	
	LOOP(inNumSamples, 
		ZXP(out) = sc_fold(ZXP(in), lo, hi, range, range2);
	);
}

void Fold_Ctor(Fold* unit)
{

	SETCALC(Fold_next);
	unit->m_lo = ZIN0(1);
	unit->m_hi = ZIN0(2);
	
	if (unit->m_lo > unit->m_hi) {
		float temp = unit->m_lo;
		unit->m_lo = unit->m_hi;
		unit->m_hi = temp;
	}
	unit->m_range = unit->m_hi - unit->m_lo;
	unit->m_range2 = 2.f * unit->m_range;
	
	Fold_next(unit, 1);
}
*/
void Fold_next_kk(Fold* unit, int inNumSamples)
{
    float *out = OUT(0);
    float *in   = IN(0);
    float next_lo = IN0(1);
    float next_hi = IN0(2);
    float lo = unit->m_lo;
    float lo_slope = CALCSLOPE(next_lo, lo);
    float hi = unit->m_hi;
    float hi_slope = CALCSLOPE(next_hi, hi);
    float range, range2;
    
    for(int i = 0; i < inNumSamples; i++){
	range = hi - lo;
	range2 = range * 2.0;
	out[i] = sc_fold(in[i], lo, hi, range, range2);
	
	lo += lo_slope;
	hi += hi_slope;
    };
    unit->m_lo = lo;
    unit->m_hi = hi;
}

void Fold_next_ka(Fold* unit, int inNumSamples)
{
    float *out = OUT(0);
    float *in   = IN(0);
    float next_lo = IN0(1);
    float *hi = IN(2);
    float lo = unit->m_lo;
    float lo_slope = CALCSLOPE(next_lo, lo);
    float curhi, range, range2;
    for(int i = 0; i < inNumSamples; i++){ 
	curhi = hi[i];
	range = curhi - lo;
	range2 = range * 2.0;
	out[i] = sc_fold(in[i], lo, curhi, range, range2);
	lo += lo_slope;
    };
    unit->m_lo = lo;
}

void Fold_next_ak(Fold* unit, int inNumSamples)
{
    float *out = OUT(0);
    float *in   = IN(0);
    float *lo = IN(1);
    float next_hi = IN0(2);
    float hi = unit->m_hi;
    float hi_slope = CALCSLOPE(next_hi, hi);
    float range, range2, curlo;
    for(int i = 0; i < inNumSamples; i++){ 
	curlo = lo[i];
	range = hi - curlo;
	range2 = range * 2.0;
	out[i] = sc_fold(in[i], curlo, hi, range, range2);
	hi += hi_slope;
    };
    unit->m_hi = hi;
}

void Fold_next_aa(Fold* unit, int inNumSamples)
{
    float *out = OUT(0);
    float *in   = IN(0);
    float *lo = IN(1);
    float *hi = IN(2);
    float curhi, curlo, range, range2;
    for(int i = 0; i < inNumSamples; i++){
	curhi = hi[i];
	curlo = lo[i];
	range = curhi - curlo; 
	range2 = range * 2.0;
	out[i] = sc_fold(in[i], curlo, curhi, range, range2);
    };
}

void Fold_Ctor(Fold* unit)
{
    
    if(INRATE(1) == calc_FullRate){
	if(INRATE(2) == calc_FullRate) {
	    SETCALC(Fold_next_aa);
	} else {
	    SETCALC(Fold_next_ak);
	} 
    } else {
	if(INRATE(2) == calc_FullRate) {
	    SETCALC(Fold_next_ka);
	} else {
	    SETCALC(Fold_next_kk);
	}
    }
    
    unit->m_lo = ZIN0(1);
    unit->m_hi = ZIN0(2);
    
    Fold_next_kk(unit, 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/*
void Clip_next(Clip* unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in   = ZIN(0);
	float lo = unit->m_lo;
	float hi = unit->m_hi;
	
	LOOP(inNumSamples, 
		float zin = ZXP(in);
		ZXP(out) = sc_clip(zin, lo, hi);
	);
}

#ifdef NOVA_SIMD
void Clip_next_nova(Clip* unit, int inNumSamples)
{
	float lo = unit->m_lo;
	float hi = unit->m_hi;

	nova::clip_vec_simd(OUT(0), IN(0), lo, hi, inNumSamples);
}
#endif

void Clip_Ctor(Clip* unit)
{
#ifdef NOVA_SIMD
	if (!(BUFLENGTH & 15))
		SETCALC(Clip_next_nova);
	else
#endif
	SETCALC(Clip_next);
	unit->m_lo = ZIN0(1);
	unit->m_hi = ZIN0(2);
	
	if (unit->m_lo > unit->m_hi) {
		float temp = unit->m_lo;
		unit->m_lo = unit->m_hi;
		unit->m_hi = temp;
	}
	
	Clip_next(unit, 1);
}
*/

void Clip_next_kk(Clip* unit, int inNumSamples)
{
    float *out = OUT(0);
    float *in   = IN(0);
    float next_lo = IN0(1);
    float next_hi = IN0(2);
    float lo = unit->m_lo;
    float lo_slope = CALCSLOPE(next_lo, lo);
    float hi = unit->m_hi;
    float hi_slope = CALCSLOPE(next_hi, hi);
    
    for(int i = 0; i < inNumSamples; i++){
	out[i] = sc_clip(in[i], lo, hi);
	lo += lo_slope;
	hi += hi_slope;
    };
    unit->m_lo = lo;
    unit->m_hi = hi;
}

void Clip_next_ka(Clip* unit, int inNumSamples)
{
    float *out = OUT(0);
    float *in   = IN(0);
    float next_lo = IN0(1);
    float *hi = IN(2);
    float lo = unit->m_lo;
    float lo_slope = CALCSLOPE(next_lo, lo);
    
    for(int i = 0; i < inNumSamples; i++){ 
	out[i] = sc_clip(in[i], lo, hi[i]);
	lo += lo_slope;
    };
    unit->m_lo = lo;
}

void Clip_next_ak(Clip* unit, int inNumSamples)
{
    float *out = OUT(0);
    float *in   = IN(0);
    float *lo = IN(1);
    float next_hi = IN0(2);
    float hi = unit->m_hi;
    float hi_slope = CALCSLOPE(next_hi, hi);
    
    for(int i = 0; i < inNumSamples; i++){ 
	out[i] = sc_clip(in[i], lo[i], hi);
	hi += hi_slope;
    };
    unit->m_hi = hi;
}

void Clip_next_aa(Clip* unit, int inNumSamples)
{
    float *out = OUT(0);
    float *in   = IN(0);
    float *lo = IN(1);
    float *hi = IN(2);
    
    for(int i = 0; i < inNumSamples; i++){ 
	 out[i] = sc_clip(in[i], lo[i], hi[i]);
    };
}

void Clip_Ctor(Clip* unit)
{
    
    if(INRATE(1) == calc_FullRate){
	if(INRATE(2) == calc_FullRate) {
	    SETCALC(Clip_next_aa);
	} else {
	    SETCALC(Clip_next_ak);
	} 
    } else {
	if(INRATE(2) == calc_FullRate) {
	    SETCALC(Clip_next_ka);
	    } else {
	    SETCALC(Clip_next_kk);
	}
    }
    
    unit->m_lo = ZIN0(1);
    unit->m_hi = ZIN0(2);
    
    Clip_next_kk(unit, 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Unwrap_next(Unwrap* unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in   = ZIN(0);
	float range = unit->m_range;
	float half = unit->m_half;
	float prev = unit->m_prev;
	float offset = unit->m_offset;
	
	LOOP(inNumSamples, 
		float zin = ZXP(in);
		float diff = zin - prev;
		if (fabs(diff) > half) {
			if (zin < prev) offset += range;
			else offset -= range;
		}
		ZXP(out) = zin + offset;
		prev = zin;
	);
	unit->m_prev = prev;
	unit->m_offset = offset;
}

void Unwrap_Ctor(Unwrap* unit)
{

	SETCALC(Unwrap_next);
	float in   = ZIN0(0);
	float lo = ZIN0(1);
	float hi = ZIN0(2);
	
	if (lo > hi) {
		float temp = lo;
		lo = hi;
		hi = temp;
	}
	unit->m_range = fabs(hi - lo);
	unit->m_half = unit->m_range * 0.5f;
	
	if (in < lo || in >= hi) unit->m_offset = floor((lo - in)/unit->m_range) * unit->m_range;
	else unit->m_offset = 0.f;
	
	Unwrap_next(unit, 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void AmpComp_next(AmpComp *unit, int inNumSamples)
{
	
	float *out = ZOUT(0);
	float *freq = ZIN(0);
	float rootmul = unit->m_rootmul;
	float xb = unit->m_exponent;
	
	LOOP(inNumSamples, 
		float xa = ZXP(freq);
		ZXP(out) = xa >= 0.f ? std::pow(xa, xb) * rootmul : -std::pow(-xa, xb) * rootmul;
	);
}

void AmpComp_next_kk(AmpComp *unit, int inNumSamples)
{
	
	float *out = ZOUT(0);
	float *freq = ZIN(0);
	float root = ZIN0(1);
	float xb = ZIN0(2);
	
	LOOP(inNumSamples, 
		float xa = root / ZXP(freq);
		ZXP(out) = xa >= 0.f ? std::pow(xa, xb) : -std::pow(-xa, xb);
	);
}

void AmpComp_Ctor(AmpComp* unit)
{
	if(INRATE(1) != calc_ScalarRate || INRATE(2) != calc_ScalarRate) {
		SETCALC(AmpComp_next_kk);
	} else {
		float exp = ZIN0(2);
		unit->m_rootmul = std::pow(ZIN0(1), exp);
		unit->m_exponent = -1.f * exp;
		SETCALC(AmpComp_next);
	}
	AmpComp_next(unit, 1);
	
}


//////////////////////////////////////////////////////////////////////////////////////////////////

const double AMPCOMP_K = 3.5041384 * 10e15;
const double AMPCOMP_C1 = 20.598997 * 20.598997;
const double AMPCOMP_C2 = 107.65265 * 107.65265;
const double AMPCOMP_C3 = 737.86223 * 737.86223;
const double AMPCOMP_C4 = 12194.217 * 12194.217;
const double AMPCOMP_MINLEVEL = -0.1575371167435;

double AmpCompA_calcLevel(double freq)
{
		double r = freq * freq;
		double level = (AMPCOMP_K * r * r * r * r);
		double n1 = AMPCOMP_C1 + r;
		double n2 = AMPCOMP_C4 + r;
		level = level / (
							n1 * n1 *
							(AMPCOMP_C2 + r) * 
							(AMPCOMP_C3 + r) *
							n2 * n2
						);
		level = 1. - sqrt(level);
		return level;
}

void AmpCompA_next(AmpCompA *unit, int inNumSamples)
{
	
	float *out = ZOUT(0);
	float *freq = ZIN(0);
	
	double scale = unit->m_scale;
	double offset = unit->m_offset;
	
	LOOP(inNumSamples,
		ZXP(out) = AmpCompA_calcLevel(ZXP(freq)) * scale + offset;
	);
}

void AmpCompA_Ctor(AmpCompA* unit)
{
	double rootFreq = ZIN0(1);
	double rootLevel = AmpCompA_calcLevel(rootFreq);
	float minLevel = ZIN0(2);
	unit->m_scale = (ZIN0(3) - minLevel) / (rootLevel - AMPCOMP_MINLEVEL);
	unit->m_offset = minLevel - unit->m_scale * AMPCOMP_MINLEVEL;
	
	SETCALC(AmpCompA_next);
	AmpCompA_next(unit, 1);
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void InRange_next(InRange *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in   = ZIN(0);
	float lo   = ZIN0(1);
	float hi   = ZIN0(2);
	
	LOOP(inNumSamples, 
		float zin = ZXP(in);
		ZXP(out) = zin >= lo && zin <= hi ? 1.f : 0.f;
	);
}

void InRange_Ctor(InRange* unit)
{
	SETCALC(InRange_next);
	InRange_next(unit, 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void InRect_next(InRect* unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *inx   = ZIN(0);
	float *iny   = ZIN(1);
	float left   = ZIN0(2);
	float top    = ZIN0(3);
	float right  = ZIN0(4);
	float bottom = ZIN0(5);
	
	LOOP(inNumSamples, 
		float x = ZXP(inx);
		float y = ZXP(iny);
		ZXP(out) = x >= left && x <= right && y >= top && y <= bottom ? 1.f : 0.f;
	);
}

void InRect_Ctor(InRect* unit)
{
	SETCALC(InRect_next);
	InRect_next(unit, 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void LinExp_next(LinExp *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in   = ZIN(0);
	float dstlo = unit->m_dstlo;
	
	float dstratio = unit->m_dstratio;
	float rsrcrange = unit->m_rsrcrange;
	float rrminuslo = unit->m_rrminuslo;

	LOOP(inNumSamples, 
		ZXP(out) = dstlo * std::pow(dstratio, ZXP(in) * rsrcrange + rrminuslo);
	);
}

void LinExp_next_kk(LinExp *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in   = ZIN(0);
	float srclo = ZIN0(1);
	float srchi = ZIN0(2);
	float dstlo = ZIN0(3);
	float dsthi = ZIN0(4);
	float dstratio = dsthi/dstlo;
	float rsrcrange = 1. / (srchi - srclo);
	float rrminuslo = rsrcrange * -srclo;

	LOOP(inNumSamples, 
		ZXP(out) = dstlo * std::pow(dstratio, ZXP(in) * rsrcrange + rrminuslo);
	);
}

void LinExp_next_aa(LinExp *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in   = ZIN(0);
	float *srclo = ZIN(1);
	float *srchi = ZIN(2);
	float *dstlo = ZIN(3);
	float *dsthi = ZIN(4);
	

	LOOP(inNumSamples,
		float zdsthi = ZXP(dsthi);
		float zdstlo = ZXP(dstlo);
		float zsrchi = ZXP(srchi);
		float zsrclo = ZXP(srclo);
		float dstratio = zdsthi/zdstlo;
		float rsrcrange = 1. / (zsrchi - zsrclo);
		float rrminuslo = rsrcrange * -zsrclo;
		ZXP(out) = zdstlo * std::pow(dstratio, ZXP(in) * rsrcrange + rrminuslo);
	);
}

void LinExp_next_ak(LinExp *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in   = ZIN(0);
	float *srclo = ZIN(1);
	float *srchi = ZIN(2);
	float dstlo = ZIN0(3);
	float dsthi = ZIN0(4);
	float dstratio = dsthi/dstlo;

	LOOP(inNumSamples,
		float zsrchi = ZXP(srchi);
		float zsrclo = ZXP(srclo);
		
		float rsrcrange = 1. / (zsrchi - zsrclo);
		float rrminuslo = rsrcrange * -zsrclo;
		ZXP(out) = dstlo * std::pow(dstratio, ZXP(in) * rsrcrange + rrminuslo);
	);
}

void LinExp_next_ka(LinExp *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in   = ZIN(0);
	float srclo = ZIN0(1);
	float srchi = ZIN0(2);
	float *dstlo = ZIN(3);
	float *dsthi = ZIN(4);
	float rsrcrange = 1. / (srchi - srclo);
	float rrminuslo = rsrcrange * -srclo;
	
	LOOP(inNumSamples,
		float zdsthi = ZXP(dsthi);
		float zdstlo = ZXP(dstlo);
		float dstratio = zdsthi/zdstlo;
		ZXP(out) = zdstlo * std::pow(dstratio, ZXP(in) * rsrcrange + rrminuslo);
	);
}


void LinExp_SetCalc(LinExp* unit)
{

	if(INRATE(1) == calc_FullRate || INRATE(2) == calc_FullRate) {
		if(INRATE(3) == calc_FullRate || INRATE(4) == calc_FullRate) {
			SETCALC(LinExp_next_aa); return;
		} else {
			SETCALC(LinExp_next_ak); return;
		}
	} else {
		if(INRATE(3) == calc_FullRate || INRATE(4) == calc_FullRate) {
			SETCALC(LinExp_next_ka); return;
		}
	}
	for(int i = 1; i<5; i++) {
		if(INRATE(i) != calc_ScalarRate) {
			SETCALC(LinExp_next_kk); return;
		}
	};
	SETCALC(LinExp_next);
	float srclo = ZIN0(1);
	float srchi = ZIN0(2);
	float dstlo = ZIN0(3);
	float dsthi = ZIN0(4);
	unit->m_dstlo = dstlo;
	unit->m_dstratio = dsthi/dstlo;
	unit->m_rsrcrange = 1. / (srchi - srclo);	
	unit->m_rrminuslo = unit->m_rsrcrange * -srclo;

}

void LinExp_Ctor(LinExp* unit)
{
	LinExp_SetCalc(unit);
	float srclo = ZIN0(1);
	float srchi = ZIN0(2);
	float dstlo = ZIN0(3);
	float dsthi = ZIN0(4);
	unit->m_dstlo = dstlo;
	unit->m_dstratio = dsthi/dstlo;
	unit->m_rsrcrange = 1. / (srchi - srclo);	
	unit->m_rrminuslo = unit->m_rsrcrange * -srclo;
	LinExp_next(unit, 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void LinLin_next(LinLin *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in   = ZIN(0);
	
	float scale = unit->m_scale;
	float offset = unit->m_offset;

	LOOP(inNumSamples, 
		ZXP(out) = scale * ZXP(in) + offset;
	);
}

void LinLin_next_kk(LinLin *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in   = ZIN(0);
	float srclo = ZIN0(1);
	float srchi = ZIN0(2);
	float dstlo = ZIN0(3);
	float dsthi = ZIN0(4);
	float scale = (dsthi - dstlo) / (srchi - srclo);
	float offset = dstlo - scale * srclo;

	LOOP(inNumSamples, 
		ZXP(out) = scale * ZXP(in) + offset;
	);
}

void LinLin_next_aa(LinLin *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in   = ZIN(0);
	float *srclo = ZIN(1);
	float *srchi = ZIN(2);
	float *dstlo = ZIN(3);
	float *dsthi = ZIN(4);
	

	LOOP(inNumSamples,
		float zdsthi = ZXP(dsthi);
		float zdstlo = ZXP(dstlo);
		float zsrchi = ZXP(srchi);
		float zsrclo = ZXP(srclo);
		
		float scale = (zdsthi - zdstlo) / (zsrchi - zsrclo);
		float offset = zdstlo - scale * zsrclo;
		ZXP(out) = scale * ZXP(in) + offset;
	);
}


void LinLin_next_ak(LinLin *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in   = ZIN(0);
	float *srclo = ZIN(1);
	float *srchi = ZIN(2);
	float dstlo = ZIN0(3);
	float dsthi = ZIN0(4);

	LOOP(inNumSamples,
		float zsrchi = ZXP(srchi);
		float zsrclo = ZXP(srclo);
		
		float scale = (dsthi - dstlo) / (zsrchi - zsrclo);
		float offset = dstlo - scale * zsrclo;
		ZXP(out) = scale * ZXP(in) + offset;
	);
}

void LinLin_next_ka(LinLin *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *in   = ZIN(0);
	float srclo = ZIN0(1);
	float srchi = ZIN0(2);
	float *dstlo = ZIN(3);
	float *dsthi = ZIN(4);
	
	LOOP(inNumSamples,
		float zdsthi = ZXP(dsthi);
		float zdstlo = ZXP(dstlo);
		
		float scale = (zdsthi - zdstlo) / (srchi - srclo);
		float offset = zdstlo - scale * srclo;
		ZXP(out) = scale * ZXP(in) + offset;
	);
}

#ifdef NOVA_SIMD
void LinLin_next_nova(LinLin *unit, int inNumSamples)
{
	float scale = unit->m_scale;
	float offset = unit->m_offset;

	nova::muladd_vec_simd(OUT(0), IN(0), scale, offset, inNumSamples);
}

void LinLin_next_kk_nova(LinLin *unit, int inNumSamples)
{
	float srclo = ZIN0(1);
	float srchi = ZIN0(2);
	float dstlo = ZIN0(3);
	float dsthi = ZIN0(4);
	float scale = (dsthi - dstlo) / (srchi - srclo);
	float offset = dstlo - scale * srclo;

	nova::muladd_vec_simd(OUT(0), IN(0), scale, offset, inNumSamples);
}
#endif

void LinLin_SetCalc(LinLin* unit)
{
#ifdef NOVA_SIMD
	if (!(BUFLENGTH & 15))
	{
		if(INRATE(1) == calc_FullRate || INRATE(2) == calc_FullRate) {
			if(INRATE(3) == calc_FullRate || INRATE(4) == calc_FullRate) {
				SETCALC(LinLin_next_aa); return;
			} else {
				SETCALC(LinLin_next_ak); return;
			}
		} else {
			if(INRATE(3) == calc_FullRate || INRATE(4) == calc_FullRate) {
				SETCALC(LinLin_next_ka); return;
			}
		}
		for(int i = 1; i<5; i++) {
			if(INRATE(i) != calc_ScalarRate) {
				SETCALC(LinLin_next_kk_nova); return;
			}
		};
		SETCALC(LinLin_next_nova);
		float srclo = ZIN0(1);
		float srchi = ZIN0(2);
		float dstlo = ZIN0(3);
		float dsthi = ZIN0(4);

		unit->m_scale = (dsthi - dstlo) / (srchi - srclo);
		unit->m_offset = dstlo - unit->m_scale * srclo;
	}
#endif

	if(INRATE(1) == calc_FullRate || INRATE(2) == calc_FullRate) {
		if(INRATE(3) == calc_FullRate || INRATE(4) == calc_FullRate) {
			SETCALC(LinLin_next_aa); return;
		} else {
			SETCALC(LinLin_next_ak); return;
		}
	} else {
		if(INRATE(3) == calc_FullRate || INRATE(4) == calc_FullRate) {
			SETCALC(LinLin_next_ka); return;
		}
	}
	for(int i = 1; i<5; i++) {
		if(INRATE(i) != calc_ScalarRate) {
			SETCALC(LinLin_next_kk); return;
		}
	};
	SETCALC(LinLin_next);

}

void LinLin_Ctor(LinLin* unit)
{
	float srclo = ZIN0(1);
	float srchi = ZIN0(2);
	float dstlo = ZIN0(3);
	float dsthi = ZIN0(4);
	
	unit->m_scale = (dsthi - dstlo) / (srchi - srclo);
	unit->m_offset = dstlo - unit->m_scale * srclo;
	LinLin_SetCalc(unit);
	LinLin_next(unit, 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////


enum {
	kEnvGen_gate,
	kEnvGen_levelScale,
	kEnvGen_levelBias,
	kEnvGen_timeScale,
	kEnvGen_doneAction,
	kEnvGen_initLevel,
	kEnvGen_numStages,
	kEnvGen_releaseNode,
	kEnvGen_loopNode,
		// 'kEnvGen_nodeOffset' must always be last
		// if you need to add an arg, put it before this one
	kEnvGen_nodeOffset
};

#ifdef NOVA_SIMD
void EnvGen_next_ak_nova(EnvGen *unit, int inNumSamples);
#endif

void EnvGen_Ctor(EnvGen *unit)
{
	//Print("EnvGen_Ctor A\n");
	if (unit->mCalcRate == calc_FullRate) {
		if (INRATE(0) == calc_FullRate) {
			SETCALC(EnvGen_next_aa);
		} else {
#ifdef NOVA_SIMD
			if (!(BUFLENGTH & 15))
				SETCALC(EnvGen_next_ak_nova);
			else
#endif
			SETCALC(EnvGen_next_ak);
		}
	} else {
		SETCALC(EnvGen_next_k);
	}

	// gate = 1.0, levelScale = 1.0, levelBias = 0.0, timeScale
	// level0, numstages, releaseNode, loopNode,
	// [level, dur, shape, curve]
		
	unit->m_endLevel = unit->m_level = ZIN0(kEnvGen_initLevel) * ZIN0(kEnvGen_levelScale) + ZIN0(kEnvGen_levelBias);
	unit->m_counter = 0;
	unit->m_stage = 1000000000;
	unit->m_prevGate = 0.f;
	unit->m_released = false;
	unit->m_releaseNode = (int)ZIN0(kEnvGen_releaseNode);
	EnvGen_next_k(unit, 1);
}

enum {
	shape_Step,
	shape_Linear,
	shape_Exponential,
	shape_Sine,
	shape_Welch,
	shape_Curve,
	shape_Squared,
	shape_Cubed,
	shape_Sustain = 9999
};

void EnvGen_next_k(EnvGen *unit, int inNumSamples)
{
	float *out = OUT(0);
	float gate = ZIN0(kEnvGen_gate); 
	//Print("->EnvGen_next_k gate %g\n", gate);
	int counter = unit->m_counter;
	double level = unit->m_level;

	if (unit->m_prevGate <= 0. && gate > 0.) {
		unit->m_stage = -1;
		unit->mDone = false;
		unit->m_released = false;
		counter = 0;
	} else if (gate <= -1.f && unit->m_prevGate > -1.f) {
		// cutoff
		int numstages = (int)ZIN0(kEnvGen_numStages);
		float dur = -gate - 1.f;
		counter  = (int32)(dur * SAMPLERATE);
		counter  = sc_max(1, counter);
		unit->m_stage = numstages;
		unit->m_shape = shape_Linear;
			// first ZIN0 gets the last envelope node's level, then apply levelScale and levelBias
		unit->m_endLevel = ZIN0(unit->mNumInputs - 4) * ZIN0(kEnvGen_levelScale) + ZIN0(kEnvGen_levelBias);
		unit->m_grow = (unit->m_endLevel - level) / counter;
	} else if (unit->m_prevGate > 0.f && gate <= 0.f 
			&& unit->m_releaseNode >= 0 && !unit->m_released) {
		counter = 0;
		unit->m_stage = unit->m_releaseNode - 1;
		unit->m_released = true;
	}
	unit->m_prevGate = gate;
	

	// gate = 1.0, levelScale = 1.0, levelBias = 0.0, timeScale
	// level0, numstages, releaseNode, loopNode,
	// [level, dur, shape, curve]

	if (counter <= 0) {
		//Print("stage %d rel %d\n", unit->m_stage, (int)ZIN0(kEnvGen_releaseNode));
		int numstages = (int)ZIN0(kEnvGen_numStages);
		
		//Print("stage %d   numstages %d\n", unit->m_stage, numstages);
		if (unit->m_stage+1 >= numstages) { // num stages
		//Print("stage+1 > num stages\n");
			counter = INT_MAX;
			unit->m_shape = 0;
			level = unit->m_endLevel;
			unit->mDone = true;
			int doneAction = (int)ZIN0(kEnvGen_doneAction);
			DoneAction(doneAction, unit);
		} else if (unit->m_stage+1 == unit->m_releaseNode && !unit->m_released) { // sustain stage
			int loopNode = (int)ZIN0(kEnvGen_loopNode);
			if (loopNode >= 0 && loopNode < numstages) {
				unit->m_stage = loopNode;
				goto initSegment;
			} else {
				counter = INT_MAX;
				unit->m_shape = shape_Sustain;
				level = unit->m_endLevel;
			}
		//Print("sustain\n");
		} else {
			unit->m_stage++;
	initSegment:
		//Print("stage %d\n", unit->m_stage);
		//Print("initSegment\n");
			//out = unit->m_level;
			int stageOffset = (unit->m_stage << 2) + kEnvGen_nodeOffset;
			
			if (stageOffset + 4 > unit->mNumInputs) {
				// oops.
				Print("envelope went past end of inputs.\n");
				ClearUnitOutputs(unit, 1);
				NodeEnd(&unit->mParent->mNode);
				return;
			}

			float** envPtr  = unit->mInBuf + stageOffset;
			double endLevel = *envPtr[0] * ZIN0(kEnvGen_levelScale) + ZIN0(kEnvGen_levelBias); // scale levels
			double dur      = *envPtr[1] * ZIN0(kEnvGen_timeScale);
			unit->m_shape   = (int32)*envPtr[2];
			double curve    = *envPtr[3];
			unit->m_endLevel = endLevel;
					
			counter  = (int32)(dur * SAMPLERATE);
			counter  = sc_max(1, counter);
		//Print("stageOffset %d   level %g   endLevel %g   dur %g   shape %d   curve %g\n", stageOffset, level, endLevel, dur, unit->m_shape, curve);
		//Print("SAMPLERATE %g\n", SAMPLERATE);
			if (counter == 1) unit->m_shape = 1; // shape_Linear
		//Print("new counter = %d  shape = %d\n", counter, unit->m_shape);
			switch (unit->m_shape) {
				case shape_Step : {
					level = endLevel;
				} break;
				case shape_Linear : {
					unit->m_grow = (endLevel - level) / counter;
					//Print("grow %g\n", unit->m_grow);
				} break;
				case shape_Exponential : {
					unit->m_grow = std::pow(endLevel / level, 1.0 / counter);
				} break;
				case shape_Sine : {
					double w = pi / counter;
		
					unit->m_a2 = (endLevel + level) * 0.5;
					unit->m_b1 = 2. * cos(w);
					unit->m_y1 = (endLevel - level) * 0.5;
					unit->m_y2 = unit->m_y1 * sin(pi * 0.5 - w);
					level = unit->m_a2 - unit->m_y1;
				} break;
				case shape_Welch : {
					double w = (pi * 0.5) / counter;
					
					unit->m_b1 = 2. * cos(w);
					
					if (endLevel >= level) {
						unit->m_a2 = level;
						unit->m_y1 = 0.;
						unit->m_y2 = -sin(w) * (endLevel - level);
					} else {
						unit->m_a2 = endLevel;
						unit->m_y1 = level - endLevel;
						unit->m_y2 = cos(w) * (level - endLevel);
					}
					level = unit->m_a2 + unit->m_y1;
				} break;
				case shape_Curve : {
					if (fabs(curve) < 0.001) {
						unit->m_shape = 1; // shape_Linear
						unit->m_grow = (endLevel - level) / counter;
					} else {
						double a1 = (endLevel - level) / (1.0 - exp(curve));	
						unit->m_a2 = level + a1;
						unit->m_b1 = a1; 
						unit->m_grow = exp(curve / counter);
					}
				} break;
				case shape_Squared : {
					unit->m_y1 = sqrt(level); 
					unit->m_y2 = sqrt(endLevel); 
					unit->m_grow = (unit->m_y2 - unit->m_y1) / counter;
				} break;
				case shape_Cubed : {
					unit->m_y1 = std::pow(level, 0.33333333);
					unit->m_y2 = std::pow(endLevel, 0.33333333);
					unit->m_grow = (unit->m_y2 - unit->m_y1) / counter;
				} break;
			}
		}
	}

	
	switch (unit->m_shape) {
		case shape_Step : {
		} break;
		case shape_Linear : {
			double grow = unit->m_grow;
					//Print("level %g\n", level);
				level += grow;
		} break;
		case shape_Exponential : {
			double grow = unit->m_grow;
				level *= grow;
		} break;
		case shape_Sine : {
			double a2 = unit->m_a2;
			double b1 = unit->m_b1;
			double y2 = unit->m_y2;
			double y1 = unit->m_y1;
				double y0 = b1 * y1 - y2; 
				level = a2 - y0;
				y2 = y1; 
				y1 = y0;
			unit->m_y1 = y1;
			unit->m_y2 = y2;
		} break;
		case shape_Welch : {
			double a2 = unit->m_a2;
			double b1 = unit->m_b1;
			double y2 = unit->m_y2;
			double y1 = unit->m_y1;
				double y0 = b1 * y1 - y2; 
				level = a2 + y0;
				y2 = y1; 
				y1 = y0;
			unit->m_y1 = y1;
			unit->m_y2 = y2;
		} break;
		case shape_Curve : {
			double a2 = unit->m_a2;
			double b1 = unit->m_b1;
			double grow = unit->m_grow;
				b1 *= grow;
				level = a2 - b1;
			unit->m_b1 = b1;
		} break;
		case shape_Squared : {
			double grow = unit->m_grow;
			double y1 = unit->m_y1;
				y1 += grow;
				level = y1*y1;
			unit->m_y1 = y1;
		} break;
		case shape_Cubed : {
			double grow = unit->m_grow;
			double y1 = unit->m_y1;
				y1 += grow;
				level = y1*y1*y1;
			unit->m_y1 = y1;
		} break;
		case shape_Sustain : {
		} break;
	}
	*out = level;
	//Print("x %d %d %d %g\n", unit->m_stage, counter, unit->m_shape, *out);
	unit->m_level = level;
	unit->m_counter = counter - 1;

}

void EnvGen_next_ak(EnvGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float gate = ZIN0(kEnvGen_gate);
	int counter = unit->m_counter;
	double level = unit->m_level;
	
	if (unit->m_prevGate <= 0. && gate > 0.) {
		unit->m_stage = -1;
		unit->mDone = false;
		unit->m_released = false;
		counter = 0;
	} else if (gate <= -1.f && unit->m_prevGate > -1.f) {
		// cutoff
		int numstages = (int)ZIN0(kEnvGen_numStages);
		float dur = -gate - 1.f;
		counter  = (int32)(dur * SAMPLERATE);
		counter  = sc_max(1, counter);
		unit->m_stage = numstages;
		unit->m_shape = shape_Linear;
		unit->m_endLevel = ZIN0(unit->mNumInputs - 4) * ZIN0(kEnvGen_levelScale) + ZIN0(kEnvGen_levelBias);
		unit->m_grow = (unit->m_endLevel - level) / counter;
	} else if (unit->m_prevGate > 0.f && gate <= 0.f 
			&& unit->m_releaseNode >= 0 && !unit->m_released) {
		counter = 0;
		unit->m_stage = unit->m_releaseNode - 1;
		unit->m_released = true;
	}
	unit->m_prevGate = gate;
	
	int remain = inNumSamples;
	while (remain)
	{
		if (counter == 0) {
			int numstages = (int)ZIN0(kEnvGen_numStages);
			
			if (unit->m_stage+1 >= numstages) { // num stages
				counter = INT_MAX;
				unit->m_shape = 0;
				level = unit->m_endLevel;
				unit->mDone = true;
				int doneAction = (int)ZIN0(kEnvGen_doneAction);
				DoneAction(doneAction, unit);
			} else if (unit->m_stage+1 == (int)ZIN0(kEnvGen_releaseNode) && !unit->m_released) { // sustain stage
				int loopNode = (int)ZIN0(kEnvGen_loopNode);
				if (loopNode >= 0 && loopNode < numstages) {
					unit->m_stage = loopNode;
					goto initSegment;
				} else {
					counter = INT_MAX;
					unit->m_shape = shape_Sustain;
					level = unit->m_endLevel;
				}
			} else {
				unit->m_stage++;
	initSegment:
				int stageOffset = (unit->m_stage << 2) + kEnvGen_nodeOffset;
				
				if (stageOffset + 4 > unit->mNumInputs) {
					// oops.
					Print("envelope went past end of inputs.\n");
					ClearUnitOutputs(unit, 1);
					NodeEnd(&unit->mParent->mNode);
					return;
				}

				float** envPtr  = unit->mInBuf + stageOffset;
				double endLevel = *envPtr[0] * ZIN0(kEnvGen_levelScale) + ZIN0(kEnvGen_levelBias); // scale levels
				double dur      = *envPtr[1] * ZIN0(kEnvGen_timeScale);
				unit->m_shape   = (int32)*envPtr[2];
				double curve    = *envPtr[3];
				unit->m_endLevel = endLevel;
						
				counter  = (int32)(dur * SAMPLERATE);
				counter  = sc_max(1, counter);

				if (counter == 1) unit->m_shape = 1; // shape_Linear
				switch (unit->m_shape) {
					case shape_Step : {
						level = endLevel;
					} break;
					case shape_Linear : {
						unit->m_grow = (endLevel - level) / counter;
					} break;
					case shape_Exponential : {
						unit->m_grow = std::pow(endLevel / level, 1.0 / counter);
					} break;
					case shape_Sine : {
						double w = pi / counter;
			
						unit->m_a2 = (endLevel + level) * 0.5;
						unit->m_b1 = 2. * cos(w);
						unit->m_y1 = (endLevel - level) * 0.5;
						unit->m_y2 = unit->m_y1 * sin(pi * 0.5 - w);
						level = unit->m_a2 - unit->m_y1;
					} break;
					case shape_Welch : {
						double w = (pi * 0.5) / counter;
						
						unit->m_b1 = 2. * cos(w);
						
						if (endLevel >= level) {
							unit->m_a2 = level;
							unit->m_y1 = 0.;
							unit->m_y2 = -sin(w) * (endLevel - level);
						} else {
							unit->m_a2 = endLevel;
							unit->m_y1 = level - endLevel;
							unit->m_y2 = cos(w) * (level - endLevel);
						}
						level = unit->m_a2 + unit->m_y1;
					} break;
					case shape_Curve : {
						if (fabs(curve) < 0.001) {
							unit->m_shape = 1; // shape_Linear
							unit->m_grow = (endLevel - level) / counter;
						} else {
							double a1 = (endLevel - level) / (1.0 - exp(curve));	
							unit->m_a2 = level + a1;
							unit->m_b1 = a1; 
							unit->m_grow = exp(curve / counter);
						}
					} break;
					case shape_Squared : {
						unit->m_y1 = sqrt(level); 
						unit->m_y2 = sqrt(endLevel); 
						unit->m_grow = (unit->m_y2 - unit->m_y1) / counter;
					} break;
					case shape_Cubed : {
						unit->m_y1 = std::pow(level, 0.33333333);
						unit->m_y2 = std::pow(endLevel, 0.33333333);
						unit->m_grow = (unit->m_y2 - unit->m_y1) / counter;
					} break;
				}
			}
		}

		int nsmps = sc_min(remain, counter);
		switch (unit->m_shape) {
			case shape_Step : {
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
				}
			} break;
			case shape_Linear : {
				double grow = unit->m_grow;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					level += grow;
				}
			} break;
			case shape_Exponential : {
				double grow = unit->m_grow;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					level *= grow;
				}
			} break;
			case shape_Sine : {
				double a2 = unit->m_a2;
				double b1 = unit->m_b1;
				double y2 = unit->m_y2;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					double y0 = b1 * y1 - y2; 
					level = a2 - y0;
					y2 = y1; 
					y1 = y0;
				}
				unit->m_y1 = y1;
				unit->m_y2 = y2;
			} break;
			case shape_Welch : {
				double a2 = unit->m_a2;
				double b1 = unit->m_b1;
				double y2 = unit->m_y2;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					double y0 = b1 * y1 - y2; 
					level = a2 + y0;
					y2 = y1; 
					y1 = y0;
				}
				unit->m_y1 = y1;
				unit->m_y2 = y2;
			} break;
			case shape_Curve : {
				double a2 = unit->m_a2;
				double b1 = unit->m_b1;
				double grow = unit->m_grow;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					b1 *= grow;
					level = a2 - b1;
				}
				unit->m_b1 = b1;
			} break;
			case shape_Squared : {
				double grow = unit->m_grow;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					y1 += grow;
					level = y1*y1;
				}
				unit->m_y1 = y1;
			} break;
			case shape_Cubed : {
				double grow = unit->m_grow;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					y1 += grow;
					level = y1*y1*y1;
				}
				unit->m_y1 = y1;
			} break;
			case shape_Sustain : {
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
				}
			} break;
		}
		remain -= nsmps;
		counter -= nsmps;
	}
	//Print("x %d %d %d %g\n", unit->m_stage, counter, unit->m_shape, ZOUT0(0));
	unit->m_level = level;
	unit->m_counter = counter;

}

#ifdef NOVA_SIMD
void EnvGen_next_ak_nova(EnvGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float gate = ZIN0(kEnvGen_gate);
	int counter = unit->m_counter;
	double level = unit->m_level;

	if (unit->m_prevGate <= 0. && gate > 0.) {
		unit->m_stage = -1;
		unit->mDone = false;
		unit->m_released = false;
		counter = 0;
	} else if (gate <= -1.f && unit->m_prevGate > -1.f) {
		// cutoff
		int numstages = (int)ZIN0(kEnvGen_numStages);
		float dur = -gate - 1.f;
		counter	 = (int32)(dur * SAMPLERATE);
		counter	 = sc_max(1, counter);
		unit->m_stage = numstages;
		unit->m_shape = shape_Linear;
		unit->m_endLevel = ZIN0(unit->mNumInputs - 4) * ZIN0(kEnvGen_levelScale) + ZIN0(kEnvGen_levelBias);
		unit->m_grow = (unit->m_endLevel - level) / counter;
	} else if (unit->m_prevGate > 0.f && gate <= 0.f
			&& unit->m_releaseNode >= 0 && !unit->m_released) {
		counter = 0;
		unit->m_stage = unit->m_releaseNode - 1;
		unit->m_released = true;
	}
	unit->m_prevGate = gate;

	int remain = inNumSamples;
	if (counter > inNumSamples)
	{
		switch (unit->m_shape) {
		case shape_Step :
		case shape_Sustain :
			nova::setvec_simd(OUT(0), (float)level, inNumSamples);
			remain = 0;
			counter -= inNumSamples;
			break;
		case shape_Linear : {
			double slope = unit->m_grow;
			nova::set_slope_vec_simd(OUT(0), (float)level, (float)slope, inNumSamples);
			level += 64 * slope;
			remain = 0;
			counter -= inNumSamples;
		} break;
        case shape_Exponential : {
            double grow = unit->m_grow;
            nova::set_exp_vec_simd(OUT(0), (float)level, (float)grow, inNumSamples);
            level *= sc_powi(grow, inNumSamples);
            remain = 0;
            counter -= inNumSamples;
        } break;
        }
	}

	while (remain)
	{
		if (counter == 0) {
			int numstages = (int)ZIN0(kEnvGen_numStages);

			if (unit->m_stage+1 >= numstages) { // num stages
				counter = INT_MAX;
				unit->m_shape = 0;
				level = unit->m_endLevel;
				unit->mDone = true;
				int doneAction = (int)ZIN0(kEnvGen_doneAction);
				DoneAction(doneAction, unit);
			} else if (unit->m_stage+1 == (int)ZIN0(kEnvGen_releaseNode) && !unit->m_released) { // sustain stage
				int loopNode = (int)ZIN0(kEnvGen_loopNode);
				if (loopNode >= 0 && loopNode < numstages) {
					unit->m_stage = loopNode;
					goto initSegment;
				} else {
					counter = INT_MAX;
					unit->m_shape = shape_Sustain;
					level = unit->m_endLevel;
				}
			} else {
				unit->m_stage++;
	initSegment:
				int stageOffset = (unit->m_stage << 2) + kEnvGen_nodeOffset;

				if (stageOffset + 4 > unit->mNumInputs) {
					// oops.
					Print("envelope went past end of inputs.\n");
					ClearUnitOutputs(unit, 1);
					NodeEnd(&unit->mParent->mNode);
					return;
				}

				float** envPtr	= unit->mInBuf + stageOffset;
				double endLevel = *envPtr[0] * ZIN0(kEnvGen_levelScale) + ZIN0(kEnvGen_levelBias); // scale levels
				double dur		= *envPtr[1] * ZIN0(kEnvGen_timeScale);
				unit->m_shape	= (int32)*envPtr[2];
				double curve	= *envPtr[3];
				unit->m_endLevel = endLevel;

				counter	 = (int32)(dur * SAMPLERATE);
				counter	 = sc_max(1, counter);

				if (counter == 1) unit->m_shape = 1; // shape_Linear
				switch (unit->m_shape) {
					case shape_Step : {
						level = endLevel;
					} break;
					case shape_Linear : {
						unit->m_grow = (endLevel - level) / counter;
					} break;
					case shape_Exponential : {
						unit->m_grow = std::pow(endLevel / level, 1.0 / counter);
					} break;
					case shape_Sine : {
						double w = pi / counter;

						unit->m_a2 = (endLevel + level) * 0.5;
						unit->m_b1 = 2. * cos(w);
						unit->m_y1 = (endLevel - level) * 0.5;
						unit->m_y2 = unit->m_y1 * sin(pi * 0.5 - w);
						level = unit->m_a2 - unit->m_y1;
					} break;
					case shape_Welch : {
						double w = (pi * 0.5) / counter;

						unit->m_b1 = 2. * cos(w);

						if (endLevel >= level) {
							unit->m_a2 = level;
							unit->m_y1 = 0.;
							unit->m_y2 = -sin(w) * (endLevel - level);
						} else {
							unit->m_a2 = endLevel;
							unit->m_y1 = level - endLevel;
							unit->m_y2 = cos(w) * (level - endLevel);
						}
						level = unit->m_a2 + unit->m_y1;
					} break;
					case shape_Curve : {
						if (fabs(curve) < 0.001) {
							unit->m_shape = 1; // shape_Linear
							unit->m_grow = (endLevel - level) / counter;
						} else {
							double a1 = (endLevel - level) / (1.0 - exp(curve));
							unit->m_a2 = level + a1;
							unit->m_b1 = a1;
							unit->m_grow = exp(curve / counter);
						}
					} break;
					case shape_Squared : {
						unit->m_y1 = sqrt(level);
						unit->m_y2 = sqrt(endLevel);
						unit->m_grow = (unit->m_y2 - unit->m_y1) / counter;
					} break;
					case shape_Cubed : {
						unit->m_y1 = std::pow(level, 0.33333333);
						unit->m_y2 = std::pow(endLevel, 0.33333333);
						unit->m_grow = (unit->m_y2 - unit->m_y1) / counter;
					} break;
				}
			}
		}

		int nsmps = sc_min(remain, counter);
		switch (unit->m_shape) {
			case shape_Step : {
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
				}
			} break;
			case shape_Linear : {
				double grow = unit->m_grow;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					level += grow;
				}
			} break;
			case shape_Exponential : {
				double grow = unit->m_grow;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					level *= grow;
				}
			} break;
			case shape_Sine : {
				double a2 = unit->m_a2;
				double b1 = unit->m_b1;
				double y2 = unit->m_y2;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					double y0 = b1 * y1 - y2;
					level = a2 - y0;
					y2 = y1;
					y1 = y0;
				}
				unit->m_y1 = y1;
				unit->m_y2 = y2;
			} break;
			case shape_Welch : {
				double a2 = unit->m_a2;
				double b1 = unit->m_b1;
				double y2 = unit->m_y2;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					double y0 = b1 * y1 - y2;
					level = a2 + y0;
					y2 = y1;
					y1 = y0;
				}
				unit->m_y1 = y1;
				unit->m_y2 = y2;
			} break;
			case shape_Curve : {
				double a2 = unit->m_a2;
				double b1 = unit->m_b1;
				double grow = unit->m_grow;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					b1 *= grow;
					level = a2 - b1;
				}
				unit->m_b1 = b1;
			} break;
			case shape_Squared : {
				double grow = unit->m_grow;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					y1 += grow;
					level = y1*y1;
				}
				unit->m_y1 = y1;
			} break;
			case shape_Cubed : {
				double grow = unit->m_grow;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					y1 += grow;
					level = y1*y1*y1;
				}
				unit->m_y1 = y1;
			} break;
			case shape_Sustain : {
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
				}
			} break;
		}
		remain -= nsmps;
		counter -= nsmps;
	}
	//Print("x %d %d %d %g\n", unit->m_stage, counter, unit->m_shape, ZOUT0(0));
	unit->m_level = level;
	unit->m_counter = counter;

}
#endif

#define CHECK_GATE \
        prevGate = gate; \
        gate = ZXP(gatein); \
        if (prevGate <= 0.f && gate > 0.f) { \
                gatein--; \
                unit->m_stage = -1; \
                unit->m_released = false; \
                unit->mDone = false; \
                counter = i; \
                nsmps = i; \
                break; \
        } else if (gate <= -1.f && unit->m_prevGate > -1.f) { \
                int numstages = (int)ZIN0(kEnvGen_numStages); \
                float dur = -gate - 1.f; \
                gatein--; \
                counter  = (int32)(dur * SAMPLERATE); \
                counter  = sc_max(1, counter) + i; \
                unit->m_stage = numstages; \
                unit->m_shape = shape_Linear; \
				unit->m_endLevel = ZIN0(unit->mNumInputs - 4) * ZIN0(kEnvGen_levelScale) + ZIN0(kEnvGen_levelBias); \
				unit->m_grow = (unit->m_endLevel - level) / counter; \
                nsmps = i; \
                break; \
        } else if (prevGate > 0.f && gate <= 0.f \
                        && unit->m_releaseNode >= 0 && !unit->m_released) { \
                gatein--; \
                counter = i; \
                unit->m_stage = unit->m_releaseNode - 1; \
                unit->m_released = true; \
                nsmps = i; \
                break; \
        } \


void EnvGen_next_aa(EnvGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *gatein = ZIN(kEnvGen_gate);
	int counter = unit->m_counter;
	double level = unit->m_level;
	float gate = 0.;
	float prevGate = unit->m_prevGate;
	int remain = inNumSamples;
	while (remain)
	{
		if (counter == 0) {

			int numstages = (int)ZIN0(kEnvGen_numStages);
			
			if (unit->m_stage+1 >= numstages) { // num stages
				counter = INT_MAX;
				unit->m_shape = 0;
				level = unit->m_endLevel;
				unit->mDone = true;
				int doneAction = (int)ZIN0(kEnvGen_doneAction);
				DoneAction(doneAction, unit);
			} else if (unit->m_stage+1 == (int)ZIN0(kEnvGen_releaseNode) && !unit->m_released) { // sustain stage
				int loopNode = (int)ZIN0(kEnvGen_loopNode);
				if (loopNode >= 0 && loopNode < numstages) {
					unit->m_stage = loopNode;
					goto initSegment;
				} else {
					counter = INT_MAX;
					unit->m_shape = shape_Sustain;
					level = unit->m_endLevel;
				}
			} else {
				unit->m_stage++;
	initSegment:
				int stageOffset = (unit->m_stage << 2) + kEnvGen_nodeOffset;
				
				if (stageOffset + 4 > unit->mNumInputs) {
					// oops.
					Print("envelope went past end of inputs.\n");
					ClearUnitOutputs(unit, 1);
					NodeEnd(&unit->mParent->mNode);
					return;
				}

				float** envPtr  = unit->mInBuf + stageOffset;
				double endLevel = *envPtr[0] * ZIN0(kEnvGen_levelScale) + ZIN0(kEnvGen_levelBias); // scale levels
				double dur      = *envPtr[1] * ZIN0(kEnvGen_timeScale);
				unit->m_shape   = (int32)*envPtr[2];
				double curve    = *envPtr[3];
				unit->m_endLevel = endLevel;
						
				counter  = (int32)(dur * SAMPLERATE);
				counter  = sc_max(1, counter);
				if (counter == 1) unit->m_shape = 1; // shape_Linear
				switch (unit->m_shape) {
					case shape_Step : {
						level = endLevel;
					} break;
					case shape_Linear : {
						unit->m_grow = (endLevel - level) / counter;
					} break;
					case shape_Exponential : {
						unit->m_grow = std::pow(endLevel / level, 1.0 / counter);
					} break;
					case shape_Sine : {
						double w = pi / counter;
			
						unit->m_a2 = (endLevel + level) * 0.5;
						unit->m_b1 = 2. * cos(w);
						unit->m_y1 = (endLevel - level) * 0.5;
						unit->m_y2 = unit->m_y1 * sin(pi * 0.5 - w);
						level = unit->m_a2 - unit->m_y1;
					} break;
					case shape_Welch : {
						double w = (pi * 0.5) / counter;
						
						unit->m_b1 = 2. * cos(w);
						
						if (endLevel >= level) {
							unit->m_a2 = level;
							unit->m_y1 = 0.;
							unit->m_y2 = -sin(w) * (endLevel - level);
						} else {
							unit->m_a2 = endLevel;
							unit->m_y1 = level - endLevel;
							unit->m_y2 = cos(w) * (level - endLevel);
						}
						level = unit->m_a2 + unit->m_y1;
					} break;
					case shape_Curve : {
						if (fabs(curve) < 0.001) {
							unit->m_shape = 1; // shape_Linear
							unit->m_grow = (endLevel - level) / counter;
						} else {
							double a1 = (endLevel - level) / (1.0 - exp(curve));	
							unit->m_a2 = level + a1;
							unit->m_b1 = a1; 
							unit->m_grow = exp(curve / counter);
						}
					} break;
					case shape_Squared : {
						unit->m_y1 = sqrt(level); 
						unit->m_y2 = sqrt(endLevel); 
						unit->m_grow = (unit->m_y2 - unit->m_y1) / counter;
					} break;
					case shape_Cubed : {
						unit->m_y1 = std::pow(level, 0.33333333);
						unit->m_y2 = std::pow(endLevel, 0.33333333);
						unit->m_grow = (unit->m_y2 - unit->m_y1) / counter;
					} break;
				}
			}
		}

		int nsmps = sc_min(remain, counter);

		switch (unit->m_shape) {
			case shape_Step : {
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
				}
			} break;
			case shape_Linear : {
				double grow = unit->m_grow;
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
					level += grow;
				}
			} break;
			case shape_Exponential : {
				double grow = unit->m_grow;
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
					level *= grow;
				}
			} break;
			case shape_Sine : {
				double a2 = unit->m_a2;
				double b1 = unit->m_b1;
				double y2 = unit->m_y2;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
					double y0 = b1 * y1 - y2; 
					level = a2 - y0;
					y2 = y1; 
					y1 = y0;
				}
				unit->m_y1 = y1;
				unit->m_y2 = y2;
			} break;
			case shape_Welch : {
				double a2 = unit->m_a2;
				double b1 = unit->m_b1;
				double y2 = unit->m_y2;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
					double y0 = b1 * y1 - y2; 
					level = a2 + y0;
					y2 = y1; 
					y1 = y0;
				}
				unit->m_y1 = y1;
				unit->m_y2 = y2;
			} break;
			case shape_Curve : {
				double a2 = unit->m_a2;
				double b1 = unit->m_b1;
				double grow = unit->m_grow;
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
					b1 *= grow;
					level = a2 - b1;
				}
				unit->m_b1 = b1;
			} break;
			case shape_Squared : {
				double grow = unit->m_grow;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
					y1 += grow;
					level = y1*y1;
				}
				unit->m_y1 = y1;
			} break;
			case shape_Cubed : {
				double grow = unit->m_grow;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
					y1 += grow;
					level = y1*y1*y1;
				}
				unit->m_y1 = y1;
			} break;
			case shape_Sustain : {
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
				}
			} break;
		}
		remain -= nsmps;
		counter -= nsmps;
	}
	unit->m_level = level;
	unit->m_counter = counter;
	unit->m_prevGate = gate;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
void BufEnvGen_Ctor(BufEnvGen *unit)
{
	//Print("BufEnvGen_Ctor A\n");
	if (unit->mCalcRate == calc_FullRate) {
		if (INRATE(1) == calc_FullRate) {
			SETCALC(BufEnvGen_next_aa);
		} else {
			SETCALC(BufEnvGen_next_ak);
		}
	} else {
		SETCALC(BufEnvGen_next_k);
	}
	//Print("BufEnvGen_Ctor B\n");
	// get table
	float fbufnum = ZIN0(0);
	uint32 bufnum = (int)fbufnum;
	World *world = unit->mWorld;
	if (bufnum >= world->mNumSndBufs) bufnum = 0;
	unit->m_buf = world->mSndBufs + bufnum;
	SndBuf *buf = unit->m_buf;
	int tableSize = buf->samples;
	if (!buf || !buf->data || buf->samples < 8) {
		Print("Envelope not allocated.\n");
		//Print("bufnum %d\n", bufnum);
		//Print("buf %08X\n", buf);
		if (buf) {
			//Print("buf->data %08X   buf->samples %d\n", buf->data, buf->samples);
		}
		SETCALC(ClearUnitOutputs);
		return;
	}
	//Print("BufEnvGen_Ctor C\n");
	
	unit->m_level = buf->data[0];
	unit->m_counter = 0;
	unit->m_stage = -1;
	unit->m_prevGate = 0.f;
	unit->m_released = false;
	//Print("BufEnvGen_Ctor D\n");
	//ZOUT0(0) = unit->m_level;
	BufEnvGen_next_k(unit, 1);
}

void BufEnvGen_next_k(BufEnvGen *unit, int inNumSamples)
{
	float *out = OUT(0);
	float gate = ZIN0(1); 
	//Print("->BufEnvGen_next_k gate %g\n", gate);
	int counter = unit->m_counter;
	double level = unit->m_level;

	if (unit->m_prevGate <= 0. && gate > 0.) {
		unit->m_stage = -1;
		unit->mDone = false;
		unit->m_released = false;
		counter = 0;
	}
	unit->m_prevGate = gate;
	
	if (counter <= 0) {
		//Print("counter == 0\n");
		// get table
		float fbufnum = ZIN0(0);
		if (fbufnum != unit->m_fbufnum) {
			unit->m_fbufnum = fbufnum;
			uint32 bufnum = (int)fbufnum;
			World *world = unit->mWorld;
			if (bufnum >= world->mNumSndBufs) bufnum = 0;
			unit->m_buf = world->mSndBufs + bufnum;
		}
		SndBuf *buf = unit->m_buf;
		int tableSize = buf->samples;

		if (unit->m_stage+1 >= buf->data[1]) { // num stages
		//Print("stage > num stages\n");
			counter = INT_MAX;
			unit->m_shape = 0;
			level = unit->m_endLevel;
			unit->mDone = true;
		} else if (unit->m_stage == buf->data[2] && !unit->m_released) { // sustain stage
		//Print("sustain\n");
			counter = INT_MAX;
			unit->m_shape = shape_Sustain;
			level = unit->m_endLevel;
		} else {
			unit->m_stage++;
		//Print("stage %d\n", unit->m_stage);
		//Print("initSegment\n");
			//out = unit->m_level;
			int stageOffset = (unit->m_stage << 2) + 4;
			if (stageOffset + 4 > tableSize) {
				// oops.
				Print("envelope went past end of buffer.\n");
				ClearUnitOutputs(unit, 1);
				NodeEnd(&unit->mParent->mNode);
				return;
			}
			float* table = buf->data + stageOffset;
			double endLevel = table[0] * ZIN0(2) + ZIN0(3); // scale levels
			double dur      = table[1] * ZIN0(4);
			unit->m_shape   = (int32)table[2];
			double curve    = table[3];
			unit->m_endLevel = endLevel;
					
			counter  = (int32)(dur * SAMPLERATE);
			counter  = sc_max(1, counter);
		//Print("stageOffset %d   endLevel %g   dur %g   shape %d   curve %g\n", stageOffset, endLevel, dur, unit->m_shape, curve);
		//Print("SAMPLERATE %g\n", SAMPLERATE);
			if (counter == 1) unit->m_shape = 1; // shape_Linear
		//Print("new counter = %d  shape = %d\n", counter, unit->m_shape);
			switch (unit->m_shape) {
				case shape_Step : {
					level = endLevel;
				} break;
				case shape_Linear : {
					unit->m_grow = (endLevel - level) / counter;
					//Print("grow %g\n", unit->m_grow);
				} break;
				case shape_Exponential : {
					unit->m_grow = std::pow(endLevel / level, 1.0 / counter);
				} break;
				case shape_Sine : {
					double w = pi / counter;
		
					unit->m_a2 = (endLevel + level) * 0.5;
					unit->m_b1 = 2. * cos(w);
					unit->m_y1 = (endLevel - level) * 0.5;
					unit->m_y2 = unit->m_y1 * sin(pi * 0.5 - w);
					level = unit->m_a2 - unit->m_y1;
				} break;
				case shape_Welch : {
					double w = (pi * 0.5) / counter;
					
					unit->m_b1 = 2. * cos(w);
					
					if (endLevel >= level) {
						unit->m_a2 = level;
						unit->m_y1 = 0.;
						unit->m_y2 = -sin(w) * (endLevel - level);
					} else {
						unit->m_a2 = endLevel;
						unit->m_y1 = level - endLevel;
						unit->m_y2 = cos(w) * (level - endLevel);
					}
					level = unit->m_a2 + unit->m_y1;
				} break;
				case shape_Curve : {
					if (fabs(curve) < 0.001) {
						unit->m_shape = 1; // shape_Linear
						unit->m_grow = (endLevel - level) / counter;
					} else {
						double a1 = (endLevel - level) / (1.0 - exp(curve));	
						unit->m_a2 = level + a1;
						unit->m_b1 = a1; 
						unit->m_grow = exp(curve / counter);
					}
				} break;
				case shape_Squared : {
					unit->m_y1 = sqrt(level); 
					unit->m_y2 = sqrt(endLevel); 
					unit->m_grow = (unit->m_y2 - unit->m_y1) / counter;
				} break;
				case shape_Cubed : {
					unit->m_y1 = std::pow(level, 0.33333333);
					unit->m_y2 = std::pow(endLevel, 0.33333333);
					unit->m_grow = (unit->m_y2 - unit->m_y1) / counter;
				} break;
			}
		}
	}

	*out = level;
	switch (unit->m_shape) {
		case shape_Step : {
		} break;
		case shape_Linear : {
			double grow = unit->m_grow;
					//Print("level %g\n", level);
				level += grow;
		} break;
		case shape_Exponential : {
			double grow = unit->m_grow;
				level *= grow;
		} break;
		case shape_Sine : {
			double a2 = unit->m_a2;
			double b1 = unit->m_b1;
			double y2 = unit->m_y2;
			double y1 = unit->m_y1;
				double y0 = b1 * y1 - y2; 
				level = a2 - y0;
				y2 = y1; 
				y1 = y0;
			unit->m_y1 = y1;
			unit->m_y2 = y2;
		} break;
		case shape_Welch : {
			double a2 = unit->m_a2;
			double b1 = unit->m_b1;
			double y2 = unit->m_y2;
			double y1 = unit->m_y1;
				double y0 = b1 * y1 - y2; 
				level = a2 + y0;
				y2 = y1; 
				y1 = y0;
			unit->m_y1 = y1;
			unit->m_y2 = y2;
		} break;
		case shape_Curve : {
			double a2 = unit->m_a2;
			double b1 = unit->m_b1;
			double grow = unit->m_grow;
				b1 *= grow;
				level = a2 - b1;
			unit->m_b1 = b1;
		} break;
		case shape_Squared : {
			double grow = unit->m_grow;
			double y1 = unit->m_y1;
				y1 += grow;
				level = y1*y1;
			unit->m_y1 = y1;
		} break;
		case shape_Cubed : {
			double grow = unit->m_grow;
			double y1 = unit->m_y1;
				y1 += grow;
				level = y1*y1*y1;
			unit->m_y1 = y1;
		} break;
		case shape_Sustain : {
			if (gate <= 0.) {
				//Print("gate off %g\n", level);
				unit->m_released = true;
				counter = 1;
				break;
			}
		} break;
	}
	//Print("x %d %d %d %g\n", unit->m_stage, counter, unit->m_shape, *out);
	unit->m_level = level;
	unit->m_counter = counter - 1;

}

void BufEnvGen_next_ak(BufEnvGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float gate = ZIN0(1);
	int counter = unit->m_counter;
	double level = unit->m_level;
	
	if (unit->m_prevGate <= 0. && gate > 0.) {
		unit->m_stage = -1;
		unit->mDone = false;
		unit->m_released = false;
		counter = 0;
	}
	unit->m_prevGate = gate;
	
	int remain = inNumSamples;
	while (remain)
	{
		if (counter == 0) {
			// get table
			float fbufnum = ZIN0(0);
			if (fbufnum != unit->m_fbufnum) {
				unit->m_fbufnum = fbufnum;
				uint32 bufnum = (int)fbufnum;
				World *world = unit->mWorld;
				if (bufnum >= world->mNumSndBufs) bufnum = 0;
				unit->m_buf = world->mSndBufs + bufnum;
			}
			SndBuf *buf = unit->m_buf;
			int tableSize = buf->samples;

			if (unit->m_stage+1 >= buf->data[1]) { // num stages
				counter = INT_MAX;
				unit->m_shape = 0;
				level = unit->m_endLevel;
				unit->mDone = true;
			} else if (unit->m_stage == buf->data[2] && !unit->m_released) { // sustain stage
				counter = INT_MAX;
				unit->m_shape = shape_Sustain;
				level = unit->m_endLevel;
			} else {
				unit->m_stage++;
				//out = unit->m_level;
				int stageOffset = (unit->m_stage << 2) + 4;
				if (stageOffset + 4 > tableSize) {
					// oops.
					Print("envelope went past end of buffer.\n");
					ClearUnitOutputs(unit, 1);
					NodeEnd(&unit->mParent->mNode);
					return;
				}
				float* table = buf->data + stageOffset;
				double endLevel = table[0] * ZIN0(2) + ZIN0(3); // scale levels
				double dur      = table[1] * ZIN0(4);
				unit->m_shape   = (int32)table[2];
				double curve    = table[3];
				unit->m_endLevel = endLevel;
						
				counter  = (int32)(dur * SAMPLERATE);
				counter  = sc_max(1, counter);

				if (counter == 1) unit->m_shape = 1; // shape_Linear
				switch (unit->m_shape) {
					case shape_Step : {
						level = endLevel;
					} break;
					case shape_Linear : {
						unit->m_grow = (endLevel - level) / counter;
					} break;
					case shape_Exponential : {
						unit->m_grow = std::pow(endLevel / level, 1.0 / counter);
					} break;
					case shape_Sine : {
						double w = pi / counter;
			
						unit->m_a2 = (endLevel + level) * 0.5;
						unit->m_b1 = 2. * cos(w);
						unit->m_y1 = (endLevel - level) * 0.5;
						unit->m_y2 = unit->m_y1 * sin(pi * 0.5 - w);
						level = unit->m_a2 - unit->m_y1;
					} break;
					case shape_Welch : {
						double w = (pi * 0.5) / counter;
						
						unit->m_b1 = 2. * cos(w);
						
						if (endLevel >= level) {
							unit->m_a2 = level;
							unit->m_y1 = 0.;
							unit->m_y2 = -sin(w) * (endLevel - level);
						} else {
							unit->m_a2 = endLevel;
							unit->m_y1 = level - endLevel;
							unit->m_y2 = cos(w) * (level - endLevel);
						}
						level = unit->m_a2 + unit->m_y1;
					} break;
					case shape_Curve : {
						if (fabs(curve) < 0.001) {
							unit->m_shape = 1; // shape_Linear
							unit->m_grow = (endLevel - level) / counter;
						} else {
							double a1 = (endLevel - level) / (1.0 - exp(curve));	
							unit->m_a2 = level + a1;
							unit->m_b1 = a1; 
							unit->m_grow = exp(curve / counter);
						}
					} break;
					case shape_Squared : {
						unit->m_y1 = sqrt(level); 
						unit->m_y2 = sqrt(endLevel); 
						unit->m_grow = (unit->m_y2 - unit->m_y1) / counter;
					} break;
					case shape_Cubed : {
						unit->m_y1 = std::pow(level, 0.33333333);
						unit->m_y2 = std::pow(endLevel, 0.33333333);
						unit->m_grow = (unit->m_y2 - unit->m_y1) / counter;
					} break;
				}
			}
		}

		int nsmps = sc_min(remain, counter);
		switch (unit->m_shape) {
			case shape_Step : {
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
				}
			} break;
			case shape_Linear : {
				double grow = unit->m_grow;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					level += grow;
				}
			} break;
			case shape_Exponential : {
				double grow = unit->m_grow;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					level *= grow;
				}
			} break;
			case shape_Sine : {
				double a2 = unit->m_a2;
				double b1 = unit->m_b1;
				double y2 = unit->m_y2;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					double y0 = b1 * y1 - y2; 
					level = a2 - y0;
					y2 = y1; 
					y1 = y0;
				}
				unit->m_y1 = y1;
				unit->m_y2 = y2;
			} break;
			case shape_Welch : {
				double a2 = unit->m_a2;
				double b1 = unit->m_b1;
				double y2 = unit->m_y2;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					double y0 = b1 * y1 - y2; 
					level = a2 + y0;
					y2 = y1; 
					y1 = y0;
				}
				unit->m_y1 = y1;
				unit->m_y2 = y2;
			} break;
			case shape_Curve : {
				double a2 = unit->m_a2;
				double b1 = unit->m_b1;
				double grow = unit->m_grow;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					b1 *= grow;
					level = a2 - b1;
				}
				unit->m_b1 = b1;
			} break;
			case shape_Squared : {
				double grow = unit->m_grow;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					y1 += grow;
					level = y1*y1;
				}
				unit->m_y1 = y1;
			} break;
			case shape_Cubed : {
				double grow = unit->m_grow;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
					y1 += grow;
					level = y1*y1*y1;
				}
				unit->m_y1 = y1;
			} break;
			case shape_Sustain : {
				if (gate <= 0.) {
					unit->m_released = true;
					counter = 0;
					nsmps = 0;
					break;
				}
				for (int i=0; i<nsmps; ++i) {
					ZXP(out) = level;
				}
			} break;
		}
		remain -= nsmps;
		counter -= nsmps;
	}
	//Print("x %d %d %d %g\n", unit->m_stage, counter, unit->m_shape, ZOUT0(0));
	unit->m_level = level;
	unit->m_counter = counter;

}

#define CHECK_GATE \
	gate = ZXP(gatein); \
	if (prevGate <= 0.f && gate > 0.f) { \
		unit->m_stage = -1; \
		unit->m_released = false; \
		unit->mDone = false; \
		counter = 0; \
		nsmps = i; \
		break; \
	}
	

void BufEnvGen_next_aa(BufEnvGen *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *gatein = ZIN(1);
	int counter = unit->m_counter;
	double level = unit->m_level;
	float gate;
	float prevGate = unit->m_prevGate;
	int remain = inNumSamples;
	while (remain)
	{
		if (counter == 0) {
			// get table
			float fbufnum = ZIN0(0);
			if (fbufnum != unit->m_fbufnum) {
				unit->m_fbufnum = fbufnum;
				uint32 bufnum = (int)fbufnum;
				World *world = unit->mWorld;
				if (bufnum >= world->mNumSndBufs) bufnum = 0;
				unit->m_buf = world->mSndBufs + bufnum;
			}
			SndBuf *buf = unit->m_buf;
			int tableSize = buf->samples;

			if (unit->m_stage+1 >= buf->data[1]) { // num stages
				counter = INT_MAX;
				unit->m_shape = 0;
				level = unit->m_endLevel;
				unit->mDone = true;
			} else if (unit->m_stage == buf->data[2] && !unit->m_released) { // sustain stage
				counter = INT_MAX;
				unit->m_shape = shape_Sustain;
				level = unit->m_endLevel;
			} else {
				unit->m_stage++;
				//out = unit->m_level;
				int stageOffset = (unit->m_stage << 2) + 4;
				if (stageOffset + 4 > tableSize) {
					// oops.
					Print("envelope went past end of buffer.\n");
					ClearUnitOutputs(unit, remain);
					NodeEnd(&unit->mParent->mNode);
					return;
				}
				float* table = buf->data + stageOffset;
				double endLevel = table[0] * ZIN0(2) + ZIN0(3); // scale levels
				double dur      = table[1] * ZIN0(4);
				unit->m_shape   = (int32)table[2];
				double curve    = table[3];
				unit->m_endLevel = endLevel;
						
				counter  = (int32)(dur * SAMPLERATE);
				counter  = sc_max(1, counter);

				if (counter == 1) unit->m_shape = 1; // shape_Linear
				switch (unit->m_shape) {
					case shape_Step : {
						level = endLevel;
					} break;
					case shape_Linear : {
						unit->m_grow = (endLevel - level) / counter;
					} break;
					case shape_Exponential : {
						unit->m_grow = std::pow(endLevel / level, 1.0 / counter);
					} break;
					case shape_Sine : {
						double w = pi / counter;
			
						unit->m_a2 = (endLevel + level) * 0.5;
						unit->m_b1 = 2. * cos(w);
						unit->m_y1 = (endLevel - level) * 0.5;
						unit->m_y2 = unit->m_y1 * sin(pi * 0.5 - w);
						level = unit->m_a2 - unit->m_y1;
					} break;
					case shape_Welch : {
						double w = (pi * 0.5) / counter;
						
						unit->m_b1 = 2. * cos(w);
						
						if (endLevel >= level) {
							unit->m_a2 = level;
							unit->m_y1 = 0.;
							unit->m_y2 = -sin(w) * (endLevel - level);
						} else {
							unit->m_a2 = endLevel;
							unit->m_y1 = level - endLevel;
							unit->m_y2 = cos(w) * (level - endLevel);
						}
						level = unit->m_a2 + unit->m_y1;
					} break;
					case shape_Curve : {
						if (fabs(curve) < 0.001) {
							unit->m_shape = 1; // shape_Linear
							unit->m_grow = (endLevel - level) / counter;
						} else {
							double a1 = (endLevel - level) / (1.0 - exp(curve));	
							unit->m_a2 = level + a1;
							unit->m_b1 = a1; 
							unit->m_grow = exp(curve / counter);
						}
					} break;
					case shape_Squared : {
						unit->m_y1 = sqrt(level); 
						unit->m_y2 = sqrt(endLevel); 
						unit->m_grow = (unit->m_y2 - unit->m_y1) / counter;
					} break;
					case shape_Cubed : {
						unit->m_y1 = std::pow(level, 0.33333333);
						unit->m_y2 = std::pow(endLevel, 0.33333333);
						unit->m_grow = (unit->m_y2 - unit->m_y1) / counter;
					} break;
				}
			}
		}

		int nsmps = sc_min(remain, counter);
		switch (unit->m_shape) {
			case shape_Step : {
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
				}
			} break;
			case shape_Linear : {
				double grow = unit->m_grow;
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
					level += grow;
				}
			} break;
			case shape_Exponential : {
				double grow = unit->m_grow;
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
					level *= grow;
				}
			} break;
			case shape_Sine : {
				double a2 = unit->m_a2;
				double b1 = unit->m_b1;
				double y2 = unit->m_y2;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
					double y0 = b1 * y1 - y2; 
					level = a2 - y0;
					y2 = y1; 
					y1 = y0;
				}
				unit->m_y1 = y1;
				unit->m_y2 = y2;
			} break;
			case shape_Welch : {
				double a2 = unit->m_a2;
				double b1 = unit->m_b1;
				double y2 = unit->m_y2;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
					double y0 = b1 * y1 - y2; 
					level = a2 + y0;
					y2 = y1; 
					y1 = y0;
				}
				unit->m_y1 = y1;
				unit->m_y2 = y2;
			} break;
			case shape_Curve : {
				double a2 = unit->m_a2;
				double b1 = unit->m_b1;
				double grow = unit->m_grow;
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
					b1 *= grow;
					level = a2 - b1;
				}
				unit->m_b1 = b1;
			} break;
			case shape_Squared : {
				double grow = unit->m_grow;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
					y1 += grow;
					level = y1*y1;
				}
				unit->m_y1 = y1;
			} break;
			case shape_Cubed : {
				double grow = unit->m_grow;
				double y1 = unit->m_y1;
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					ZXP(out) = level;
					y1 += grow;
					level = y1*y1*y1;
				}
				unit->m_y1 = y1;
			} break;
			case shape_Sustain : {
				for (int i=0; i<nsmps; ++i) {
					CHECK_GATE
					if (gate <= 0.) {
						unit->m_released = true;
						counter = 0;
						nsmps = i;
						break;
					}
					ZXP(out) = level;
				}
			} break;
		}
		remain -= nsmps;
		counter -= nsmps;
	}
	unit->m_level = level;
	unit->m_counter = counter;
	unit->m_prevGate = gate;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////

void Linen_Ctor(Linen *unit)
{
	// gate attack level release
	SETCALC(Linen_next_k);
	
	unit->m_level = 0.f;
	unit->m_stage = 4;
	unit->m_prevGate = 0.f;
	Linen_next_k(unit, 1);
}

void Linen_next_k(Linen *unit, int inNumSamples)
{
	float gate = ZIN0(0);
	float *out = OUT(0);
		
	if (unit->m_prevGate <= 0.f && gate > 0.f) {
		unit->mDone = false;
		unit->m_stage = 0;
		float attackTime = ZIN0(1);
		float susLevel = ZIN0(2);
		int counter = (int)(attackTime * SAMPLERATE);
		counter = sc_max(1, counter);
		unit->m_slope = (susLevel - unit->m_level) / counter;
		unit->m_counter = counter;
	}

	switch (unit->m_stage) {
		case 0 : 
		case 2 : 
			*out = unit->m_level;
			unit->m_level += unit->m_slope;
			if (--unit->m_counter == 0) unit->m_stage++;
			break;
		case 1 : 
			*out = unit->m_level;
			if (gate <= -1.f) {
				// cutoff
				unit->m_stage = 2;
				float releaseTime = -gate - 1.f;
				int counter  = (int)(releaseTime * SAMPLERATE);
				counter  = sc_max(1, counter);
				unit->m_slope = -unit->m_level / counter;
				unit->m_counter = counter;
			} else if (gate <= 0.f) {
				unit->m_stage = 2;
				float releaseTime = ZIN0(3);
				int counter = (int)(releaseTime * SAMPLERATE);
				counter = sc_max(1, counter);
				unit->m_slope = -unit->m_level / counter;
				unit->m_counter = counter;
				
				//Print("release %d %d\n", unit->mParent->mNode.mID, counter);
			}
			break;
		case 3 : {
			*out = 0.f;			
			//Print("done %d\n", unit->mParent->mNode.mID);
			unit->mDone = true;
			unit->m_stage++;
			int doneAction = (int)ZIN0(4);
			DoneAction(doneAction, unit);
		} break;
		case 4 :
			*out = 0.f;
			break;
	}
	unit->m_prevGate = gate;
}


#if 0

#define LINEN_GATE \
	if (unit->m_prevGate <= 0.f && gate > 0.f) { \
		unit->m_stage = 0; \
		float attackTime = ZIN0(1); \
		float susLevel = ZIN0(2); \
		int counter = (int)(attackTime * SAMPLERATE); \
		counter = sc_max(1, counter); \
		slope = (susLevel - level) / counter; \
	}

void Linen_next_ak(Linen *unit, int inNumSamples)
{
	float *gate = ZIN0(0);
	float *out = ZOUT(0);
	
	int stage = unit->m_stage;
	int counter = unit->m_counter;
	double level = unit->m_level;
	double slope = unit->m_slope;
	
	if (unit->m_prevGate <= 0.f && gate > 0.f) {
		unit->m_stage = 0;
		float attackTime = ZIN0(1);
		float susLevel = ZIN0(2);
		int counter = attackTime * SAMPLERATE;
		counter = sc_max(1, counter);
		slope = (susLevel - level) / counter;
	}

	int remain = inNumSamples;
	do {
		int nsmps = sc_min(remain, counter);
		switch (stage) {
			case 0 : 
				LOOP(nsmps, 
					ZXP(out) = level;
					level += slope;
				);
				counter -= nsmps;
				if (counter == 0) {
					stage = 1;
					counter = INT_MAX;
				}
				break;
			case 1 : 
				LOOP(nsmps, 
					ZXP(out) = level;
				);
				counter -= nsmps;
				if (gate <= 0.f) {
					unit->m_stage = 2;
					float releaseTime = ZIN0(3);
					int counter = releaseTime * SAMPLERATE;
					counter = sc_max(1, counter);
					unit->m_slope = -level / counter;
					counter = counter;
				} else if (gate <= -1.f && unit->m_prevGate > -1.f) {
					// cutoff
					unit->m_stage = 2;
					float releaseTime = -gate - 1.f;
					counter  = (int32)(releaseTime * SAMPLERATE);
					counter  = sc_max(1, counter);
					unit->m_shape = shape_Linear;
					unit->m_grow = -unit->m_level / counter;
					unit->m_counter = counter;
				}
				break;
			case 2 : 
				LOOP(nsmps, 
					ZXP(out) = level;
					level += slope;
				);
				counter -= nsmps;
				if (counter == 0) {
					stage = 3;
					counter = INT_MAX;
				}
				break;
			case 3 :
				level = 0.f;
				LOOP(nsmps, 
					ZXP(out) = level;
				);
				counter -= nsmps;
				break;
		}
	} while (remain);
	
	unit->m_stage = stage;
	unit->m_prevGate = gate;
	unit->m_counter = counter;
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////

void ADSR_Ctor(ADSR *unit)
{
	// gate attack level release
	SETCALC(ADSR_next_k);
	
	unit->m_level = 0.f;
	unit->m_stage = 5;
	unit->m_prevGate = 0.f;
	ADSR_next_k(unit, 1);
}

inline void ADSR_set(ADSR *unit, float endLevel, float time)
{
	float curve = ZIN0(6);
	int counter = (int)(time * SAMPLERATE);
	counter = sc_max(1, counter);
	unit->m_counter = counter;

	double a1 = (endLevel - unit->m_level) / (1.0 - std::exp(curve));
	unit->m_a2 = unit->m_level + a1;
	unit->m_b1 = a1; 
	unit->m_grow = std::exp(curve / counter);
}

inline void ADSR_next(ADSR *unit)
{
	unit->m_b1 *= unit->m_grow;
	unit->m_level = unit->m_a2 - unit->m_b1;
}

void ADSR_next_k(ADSR *unit, int inNumSamples)
{
	// gate a, peak level, d s r curve
	float gate = ZIN0(0); 
	float *out = OUT(0);
		
	if (unit->m_prevGate <= 0.f && gate > 0.f) {
		unit->mDone = false;
		unit->m_stage = 0;
		float attackTime = ZIN0(1);
		float peakLevel = ZIN0(2);
		ADSR_set(unit, peakLevel, attackTime);
	}
	switch (unit->m_stage) {
		case 0 : // attack
			*out = unit->m_level;
			ADSR_next(unit);
			if (--unit->m_counter == 0) {
				unit->m_stage++;
				float decayTime = ZIN0(3);
				float susLevel = ZIN0(4);
				ADSR_set(unit, susLevel, decayTime);
			}
			break;
		case 1 : // decay
		case 3 : // release
			*out = unit->m_level;
			ADSR_next(unit);
			if (--unit->m_counter == 0) {
				unit->m_stage++;
			}
			break;
		case 2 : // sustain -> release
			*out = unit->m_level;
			if (gate <= 0.f) {
				unit->m_stage++;
				float releaseTime = ZIN0(5);
				ADSR_set(unit, 0.f, releaseTime);
			}
			break;
		case 4 : { // done
				unit->mDone = true;
				*out = 0.f;
				unit->m_stage++;
				int doneAction = (int)ZIN0(6);
				DoneAction(doneAction, unit);
			} break;
		case 5 : // done
			*out = 0.f;
			break;
	}
	unit->m_prevGate = gate;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////


void EnvFill(World *world, struct SndBuf *buf, struct sc_msg_iter *msg)
{
	if (buf->channels != 1) return;
		
	int size = buf->samples;
	int byteSize = size * sizeof(float);
	float *data = (float*)malloc(byteSize);

	double level = msg->getf();
	int numStages = msg->geti();
	/*int releaseNode =*/ msg->geti(); // ignored
	/*int loopNode =*/ msg->geti(); // ignored
	
	double pos = 0.;
	int32 index = 0;
	int32 remain = size;
	
	for (int j=0; j < numStages; ++j)
	{
		double endLevel = msg->getf();
		double dur = msg->getf();
		int shape = msg->geti(); 
		double curve = msg->getf();

		int32 ipos = (int32)pos;
		double smpdur = dur * size;
		int32 nsmps = (int32)smpdur - ipos;
		nsmps = sc_min(nsmps, remain);

		switch (shape) {
			case shape_Step : {
				level = endLevel;
				for (int i=0; i<nsmps; ++i) {
					data[index++] = level;
				}
			} break;
			case shape_Linear : {
				double grow = (endLevel - level) / nsmps;
				for (int i=0; i<nsmps; ++i) {
					data[index++] = level;
					level += grow;
				}
			} break;
			case shape_Exponential : {
				double grow = std::pow(endLevel / level, 1.0 / nsmps);
				for (int i=0; i<nsmps; ++i) {
					data[index++] = level;
					level *= grow;
				}
			} break;
			case shape_Sine : {
				double w = pi / nsmps;
	
				double a2 = (endLevel + level) * 0.5;
				double b1 = 2. * cos(w);
				double y1 = (endLevel - level) * 0.5;
				double y2 = y1 * sin(pi * 0.5 - w);
				level = a2 - y1;
				for (int i=0; i<nsmps; ++i) {
					data[index++] = level;
					double y0 = b1 * y1 - y2; 
					level = a2 - y0;
					y2 = y1; 
					y1 = y0;
				}
			} break;
			case shape_Welch : {
				double w = (pi * 0.5) / nsmps;
				
				double b1 = 2. * cos(w);
				double a2, y1, y2;
				if (endLevel >= level) {
					a2 = level;
					y1 = 0.;
					y2 = -sin(w) * (endLevel - level);
				} else {
					a2 = endLevel;
					y1 = level - endLevel;
					y2 = cos(w) * (level - endLevel);
				}
				level = a2 + y1;
				for (int i=0; i<nsmps; ++i) {
					data[index++] = level;
					double y0 = b1 * y1 - y2; 
					level = a2 - y0;
					y2 = y1; 
					y1 = y0;
				}
			} break;
			case shape_Curve : {
				if (fabs(curve) < 0.001) {
					double grow = (endLevel - level) / nsmps;
					for (int i=0; i<nsmps; ++i) {
						data[index++] = level;
						level += grow;
					}
				} else {
					double a1 = (endLevel - level) / (1.0 - exp(curve));	
					double a2 = level + a1;
					double b1 = a1; 
					double grow = exp(curve / nsmps);
					for (int i=0; i<nsmps; ++i) {
						data[index++] = level;
						b1 *= grow;
						level = a2 - b1;
					}
				}
			} break;
			case shape_Squared : {
				double y1 = sqrt(level); 
				double y2 = sqrt(endLevel); 
				double grow = (y2 - y1) / nsmps;
				for (int i=0; i<nsmps; ++i) {
					data[index++] = level;
					y1 += grow;
					level = y1*y1;
				}
			} break;
			case shape_Cubed : {
				double y1 = std::pow(level, 0.33333333);
				double y2 = std::pow(endLevel, 0.33333333);
				double grow = (y2 - y1) / nsmps;
				for (int i=0; i<nsmps; ++i) {
					data[index++] = level;
					y1 += grow;
					level = y1*y1*y1;
				}
			} break;
		}

		pos += smpdur;
		level = endLevel;
		remain -= nsmps;
	}
	memcpy(buf->data, data, byteSize);
	free(data);
}

//////////////////// Add IEnvGen 06/06/2007 /////////////////////////////////


struct IEnvGen : public Unit
{
    float m_level, m_offset;
    float m_startpoint, m_numvals, m_pointin;
    float* m_envvals;
};


extern "C"
{
	void IEnvGen_next_a(IEnvGen *unit, int inNumSamples);
	void IEnvGen_next_k(IEnvGen *unit, int inNumSamples);
	void IEnvGen_Ctor(IEnvGen* unit);
	void IEnvGen_Dtor(IEnvGen* unit);
}

#define GET_ENV_VAL \
	switch (shape) \
	    { \
	    case shape_Step : \
		    level = unit->m_level = endLevel; \
		    break; \
	    case shape_Linear : \
	    default: \
		    level = unit->m_level = pos * (endLevel - begLevel) + begLevel; \
		    break; \
	    case shape_Exponential : \
			level = unit->m_level = begLevel * std::pow(endLevel / begLevel, pos); \
		    break; \
	    case shape_Sine : \
		    level = unit->m_level = begLevel + (endLevel - begLevel) * (-cos(pi * pos) * 0.5 + 0.5); \
		    break; \
	    case shape_Welch : \
	    { \
		    if (begLevel < endLevel) \
			    level = unit->m_level = begLevel + (endLevel - begLevel) * sin(pi2 * pos); \
		    else \
			    level = unit->m_level = endLevel - (endLevel - begLevel) * sin(pi2 - pi2 * pos); \
		    break; \
	    } \
	    case shape_Curve : \
		    if (fabs((float)curve) < 0.0001) { \
			    level = unit->m_level = pos * (endLevel - begLevel) + begLevel; \
		    } else { \
				double denom = 1. - std::exp((float)curve); \
				double numer = 1. - std::exp((float)(pos * curve)); \
			    level = unit->m_level = begLevel + (endLevel - begLevel) * (numer/denom); \
		    } \
		    break; \
	    case shape_Squared : \
	    { \
		    double sqrtBegLevel = sqrt(begLevel); \
		    double sqrtEndLevel = sqrt(endLevel); \
		    double sqrtLevel = pos * (sqrtEndLevel - sqrtBegLevel) + sqrtBegLevel; \
		    level = unit->m_level = sqrtLevel * sqrtLevel; \
		    break; \
	    } \
	    case shape_Cubed : \
	    { \
			double cbrtBegLevel = std::pow(begLevel, 0.3333333f); \
			double cbrtEndLevel = std::pow(endLevel, 0.3333333f); \
		    double cbrtLevel = pos * (cbrtEndLevel - cbrtBegLevel) + cbrtBegLevel; \
		    level = unit->m_level = cbrtLevel * cbrtLevel * cbrtLevel; \
		    break; \
	    } \
	} \


void IEnvGen_Ctor(IEnvGen *unit)
{

	if (INRATE(0) == calc_FullRate) {
		SETCALC(IEnvGen_next_a);
		} else {
		SETCALC(IEnvGen_next_k); 
		}
		
	// pointer, offset
	// initlevel, numstages, totaldur,
	// [dur, shape, curve, level] * numvals
	int numStages = (int)IN0(3);
	int numvals = numStages * 4; // initlevel + (levels, dur, shape, curves) * stages
	float offset = unit->m_offset = IN0(1);
	float point = unit->m_pointin = IN0(0) - offset;
	unit->m_envvals = (float*)RTAlloc(unit->mWorld, (int)(numvals + 1.) * sizeof(float));
	
	unit->m_envvals[0] = IN0(2);
//	Print("%3.3f\n", unit->m_envvals[0]);
	// fill m_envvals with the values;
	for (int i = 1; i <= numvals; i++) {
	    unit->m_envvals[i] = IN0(4 + i);
//	    Print("%3.3f\n", unit->m_envvals[i]);
	}
	
//	float out = OUT0(0);
	float totalDur = IN0(4);
	float level = 0.f;
	float newtime = 0.f;
	int stage = 0;
	float seglen = 0.f;
	if (point >= totalDur) {
	    unit->m_level = level = unit->m_envvals[numStages * 4]; // grab the last value
	    } else {
	    if (point <= 0.0) {
		unit->m_level = level = unit->m_envvals[0];
		} else {
		float segpos = point;
		// determine which segment the current time pointer needs calculated
		for(int j = 0; point >= newtime; j++) {
			seglen = unit->m_envvals[(j * 4) + 1];
			newtime += seglen;
			segpos -= seglen;
			stage = j;
		    }
			
		segpos = segpos + seglen;
		float begLevel = unit->m_envvals[(stage * 4)];
		int shape = (int)unit->m_envvals[(stage * 4) + 2];
		int curve = (int)unit->m_envvals[(stage * 4) + 3];
		float endLevel = unit->m_envvals[(stage * 4) + 4];
		float pos = (segpos / seglen);
			
		GET_ENV_VAL
		}
	    }
	OUT0(0) = level;
}

void IEnvGen_Dtor(IEnvGen *unit)
{
	RTFree(unit->mWorld, unit->m_envvals);
}


void IEnvGen_next_a(IEnvGen *unit, int inNumSamples)
{
    float* out = OUT(0);
    float level = unit->m_level;
    float* pointin = IN(0);
    float offset = unit->m_offset;
    int numStages = (int)IN0(3);
    float point; // = unit->m_pointin;

    float totalDur = IN0(4);

    int stagemul;
    // pointer, offset
    // level0, numstages, totaldur,    
    // [initval, [dur, shape, curve, level] * N ]
	
    for( int i = 0; i < inNumSamples; i++) {
	    if (pointin[i] == unit->m_pointin){
		out[i] = level;
		} else {
		unit->m_pointin = point = sc_max(pointin[i] - offset, 0.0);
		float newtime = 0.f;
		int stage = 0;
		float seglen = 0.f;
		if (point >= totalDur) {
		    unit->m_level = level = unit->m_envvals[numStages * 4]; // grab the last value
		    } else {
		    if (point <= 0.0) {
			unit->m_level = level = unit->m_envvals[0];
			} else {
			float segpos = point;
			// determine which segment the current time pointer needs
			for(int j = 0; point >= newtime; j++) {
				seglen = unit->m_envvals[(j * 4) + 1];
				newtime += seglen;
				segpos -= seglen;
				stage = j;
			    }
			stagemul = stage * 4;
			segpos = segpos + seglen;
			float begLevel = unit->m_envvals[stagemul];
			int shape = (int)unit->m_envvals[stagemul + 2];
			int curve = (int)unit->m_envvals[stagemul + 3];
			float endLevel = unit->m_envvals[stagemul + 4];
			float pos = (segpos / seglen); 
			
			GET_ENV_VAL
		    }
		}
	    }
	out[i] = level;
	}
}


void IEnvGen_next_k(IEnvGen *unit, int inNumSamples)
{
    float* out = OUT(0);
    float level = unit->m_level;
    float pointin = sc_max(IN0(0) - unit->m_offset, 0);
    int numStages = (int)IN0(3);

    float totalDur = IN0(4);
    // pointer, offset
    // level0, numstages, totaldur,
    float point = unit->m_pointin;

    // [initval, [dur, shape, curve, level] * N ]

    if (pointin == unit->m_pointin) {
	for(int i = 0; i < inNumSamples; i++){
	    out[i] = level;
	    }
	} else {
	float pointslope = CALCSLOPE(pointin, point);	
	for( int i = 0; i < inNumSamples; i++) {
		float newtime = 0.f;
		int stage = 0;
		float seglen = 0.f;
		if (point >= totalDur) {
		    unit->m_level = level = unit->m_envvals[numStages * 4]; // grab the last value
		    } else {
		    if (point <= 0.0) {
			unit->m_level = level = unit->m_envvals[0];
			} else {
			float segpos = point;
			// determine which segment the current time pointer needs calculated
			for(int j = 0; point >= newtime; j++) {
				seglen = unit->m_envvals[(j * 4) + 1];
				newtime += seglen;
				segpos -= seglen;
				stage = j;
			    }
			
			segpos = segpos + seglen;
			float begLevel = unit->m_envvals[(stage * 4)];
			int shape = (int)unit->m_envvals[(stage * 4) + 2];
			float curve = unit->m_envvals[(stage * 4) + 3];
			float endLevel = unit->m_envvals[(stage * 4) + 4];
			float pos = (segpos / seglen);
			
			GET_ENV_VAL
		    }
		}
	    out[i] = level;
	    point += pointslope;
	    }

   	unit->m_pointin = pointin;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////


PluginLoad(LF)
{
	ft = inTable;

	DefineSimpleUnit(Vibrato);
	DefineSimpleUnit(LFPulse);
	DefineSimpleUnit(LFSaw);
	DefineSimpleUnit(LFPar);
	DefineSimpleUnit(LFCub);
	DefineSimpleUnit(LFTri);
	DefineSimpleUnit(LFGauss);
	DefineSimpleUnit(Impulse);
	DefineSimpleUnit(VarSaw);
	DefineSimpleUnit(SyncSaw);
	DefineSimpleUnit(K2A);
	DefineSimpleUnit(A2K);
	DefineSimpleUnit(T2K);
	DefineSimpleUnit(T2A);
	DefineSimpleUnit(DC);
	DefineSimpleUnit(Silent);
	DefineSimpleUnit(Line);
	DefineSimpleUnit(XLine);
	
	DefineSimpleUnit(Wrap);
	DefineSimpleUnit(Fold);
	DefineSimpleUnit(Clip);
	DefineSimpleUnit(Unwrap);
	DefineSimpleUnit(AmpComp);
	DefineSimpleUnit(AmpCompA);
	DefineSimpleUnit(InRange);
	DefineSimpleUnit(InRect);
	DefineSimpleUnit(LinExp);
	DefineSimpleUnit(LinLin);
	DefineSimpleUnit(EnvGen);
	//DefineSimpleUnit(BufEnvGen);
	DefineSimpleUnit(Linen);

	DefineBufGen("env", EnvFill);
	
	DefineDtorUnit(IEnvGen);
}

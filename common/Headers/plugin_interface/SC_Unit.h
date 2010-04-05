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


#ifndef _SC_Unit_
#define _SC_Unit_

#include "SC_Types.h"

typedef void (*UnitCtorFunc)(struct Unit* inUnit);
typedef void (*UnitDtorFunc)(struct Unit* inUnit);

typedef void (*UnitCalcFunc)(struct Unit *inThing, int inNumSamples);

struct SC_Unit_Extensions {
	float * todo; 	
};

struct Unit
{
	struct World *mWorld;
	struct UnitDef *mUnitDef;
	struct Graph *mParent;
	uint16 mNumInputs, mNumOutputs;
	int16 mCalcRate;
	int16 mSpecialIndex;		// used by unary and binary ops
	int16 mParentIndex;
	int16 mDone;
	struct Wire **mInput, **mOutput;
	struct Rate *mRate;
	SC_Unit_Extensions* mExtensions; //future proofing and backwards compatibility; used to be SC_Dimension struct pointer
	float **mInBuf, **mOutBuf;

	UnitCalcFunc mCalcFunc;
	int mBufLength;
};

typedef struct Unit Unit;

enum {
	kUnitDef_CantAliasInputsToOutputs = 1
};

// easy macros, the unit variable must be named 'unit'.
#ifndef SC_WIN32

// These return float* pointers to input and output buffers.
#define IN(index)  (unit->mInBuf[index])
#define OUT(index) (unit->mOutBuf[index])

// These return a float value. Used for control rate inputs and outputs.
#define IN0(index)  (IN(index)[0])
#define OUT0(index) (OUT(index)[0])

#else

// Win32 headers (included by C std library headers) define IN and OUT macros
// for their own purposes. To avoid problems we don't define IN and OUT here
// but define SC_IN and SC_OUT instead. Source files that use IN and OUT need
// to include definitions of IN, and OUT referencing SC_IN and SC_OUT after
// all headers have been included.
#define SC_IN(index)  (unit->mInBuf[index])
#define SC_OUT(index) (unit->mOutBuf[index])
#define IN0(index)  (SC_IN(index)[0])
#define OUT0(index) (SC_OUT(index)[0])

#endif

// get the rate of the input.
#define INRATE(index) (unit->mInput[index]->mCalcRate)

// get the blocksize of the input
#define INBUFLENGTH(index) (unit->mInput[index]->mFromUnit->mBufLength)

// set the calculation function
#define SETCALC(func) (unit->mCalcFunc = (UnitCalcFunc)&func)

// calculate a slope for control rate interpolation to audio rate.
#define CALCSLOPE(next,prev) ((next - prev) * sc_typeof_cast(next)unit->mRate->mSlopeFactor)

// get useful values
#define SAMPLERATE (unit->mRate->mSampleRate)
#define SAMPLEDUR (unit->mRate->mSampleDur)
#define BUFLENGTH (unit->mBufLength)
#define BUFRATE (unit->mRate->mBufRate)
#define BUFDUR (unit->mRate->mBufDuration)
#define FULLRATE (unit->mWorld->mFullRate.mSampleRate)
#define FULLBUFLENGTH (unit->mWorld->mFullRate.mBufLength)

// macros to grab a Buffer reference from the buffer indicated by the UGen's FIRST input
#define GET_BUF \
	float fbufnum  = ZIN0(0); \
	if (fbufnum < 0.f) { fbufnum = 0.f; } \
	if (fbufnum != unit->m_fbufnum) { \
		uint32 bufnum = (int)fbufnum; \
		World *world = unit->mWorld; \
		if (bufnum >= world->mNumSndBufs) { \
			int localBufNum = bufnum - world->mNumSndBufs; \
			Graph *parent = unit->mParent; \
			if(localBufNum <= parent->localBufNum) { \
				unit->m_buf = parent->mLocalSndBufs + localBufNum; \
			} else { \
				bufnum = 0; \
				unit->m_buf = world->mSndBufs + bufnum; \
			} \
		} else { \
			unit->m_buf = world->mSndBufs + bufnum; \
		} \
		unit->m_fbufnum = fbufnum; \
	} \
	SndBuf *buf = unit->m_buf; \
	float *bufData __attribute__((__unused__)) = buf->data; \
	uint32 bufChannels __attribute__((__unused__)) = buf->channels; \
	uint32 bufSamples __attribute__((__unused__)) = buf->samples; \
	uint32 bufFrames = buf->frames; \
	int mask __attribute__((__unused__)) = buf->mask; \
	int guardFrame __attribute__((__unused__)) = bufFrames - 2;


#define SIMPLE_GET_BUF \
	float fbufnum  = ZIN0(0); \
	fbufnum = sc_max(0.f, fbufnum); \
	if (fbufnum != unit->m_fbufnum) { \
		uint32 bufnum = (int)fbufnum; \
		World *world = unit->mWorld; \
		if (bufnum >= world->mNumSndBufs) { \
			int localBufNum = bufnum - world->mNumSndBufs; \
			Graph *parent = unit->mParent; \
			if(localBufNum <= parent->localBufNum) { \
				unit->m_buf = parent->mLocalSndBufs + localBufNum; \
			} else { \
				bufnum = 0; \
				unit->m_buf = world->mSndBufs + bufnum; \
			} \
		} else { \
			unit->m_buf = world->mSndBufs + bufnum; \
		} \
		unit->m_fbufnum = fbufnum; \
	} \
	SndBuf *buf = unit->m_buf; \

// macros to get pseudo-random number generator, and put its state in registers
#define RGET \
	RGen& rgen = *unit->mParent->mRGen; \
	uint32 s1 = rgen.s1; \
	uint32 s2 = rgen.s2; \
	uint32 s3 = rgen.s3;
#define RPUT \
	rgen.s1 = s1; \
	rgen.s2 = s2; \
	rgen.s3 = s3;

typedef void (*UnitCmdFunc)(struct Unit *unit, struct sc_msg_iter *args);
typedef void (*PlugInCmdFunc)(World *inWorld, void* inUserData, struct sc_msg_iter *args, void *replyAddr);

#endif

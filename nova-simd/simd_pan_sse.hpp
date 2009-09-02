//  simd functions for panning, sse implementation
//  Copyright (C) 2009 Tim Blechmann
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; see the file COPYING.  If not, write to
//  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.


#ifndef SIMD_PAN_SSE_HPP
#define SIMD_PAN_SSE_HPP

#include <xmmintrin.h>

namespace nova
{
template <>
void pan2_vec_simd(float * out0, float * out1, const float * in,
                   float factor0, float factor1, unsigned int n)
{
    n /= 8;

    const __m128 f0 = _mm_set_ps1(factor0);
    const __m128 f1 = _mm_set_ps1(factor1);

    do
    {
        __m128 sig0 = _mm_load_ps(in);
        __m128 sig1 = _mm_load_ps(in+4);
        _mm_store_ps(out0, _mm_mul_ps(sig0, f0));
        _mm_store_ps(out1, _mm_mul_ps(sig0, f1));

        _mm_store_ps(out0+4, _mm_mul_ps(sig1, f0));
        _mm_store_ps(out1+4, _mm_mul_ps(sig1, f1));

        in += 8;
        out0 += 8;
        out1 += 8;
    } while(--n);
}

template <>
void pan2_vec_simd(float * out0, float * out1, const float * in, float factor0, float slope0,
                   float factor1, float slope1, unsigned int n)
{
    n /= 8;

    __m128 f0 = _mm_setr_ps(factor0, factor0 + slope0,
                            factor0 + 2*slope0, factor0 + 3*slope0);
    __m128 f1 = _mm_setr_ps(factor1, factor1 + slope1,
                            factor1 + 2*slope1, factor1 + 3*slope1);

    const __m128 vslope0 = _mm_set_ps1(4*slope0);
    const __m128 vslope1 = _mm_set_ps1(4*slope1);

    do
    {
        __m128 sig0 = _mm_load_ps(in);
        __m128 sig1 = _mm_load_ps(in+4);
        _mm_store_ps(out0, _mm_mul_ps(sig0, f0));
        _mm_store_ps(out1, _mm_mul_ps(sig0, f1));
        f0 = _mm_add_ps(f0, vslope0);
        f1 = _mm_add_ps(f1, vslope1);

        _mm_store_ps(out0+4, _mm_mul_ps(sig1, f0));
        _mm_store_ps(out1+4, _mm_mul_ps(sig1, f1));
        f0 = _mm_add_ps(f0, vslope0);
        f1 = _mm_add_ps(f1, vslope1);

        in += 8;
        out0 += 8;
        out1 += 8;
    } while(--n);
}

} /* namespace nova */

#endif /* SIMD_PAN_SSE_HPP */

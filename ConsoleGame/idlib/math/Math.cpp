#pragma hdrstop
#include "../precompiled.h"

const int SMALLEST_NON_DENORMAL = 1 << IEEE_FLT_MANTISSA_BITS;

const float	idMath::M_SEC2MS = 1000.0f;
const float	idMath::M_MS2SEC = 0.001f;
const float	idMath::INFINITY = 1e30f;
const float idMath::FLT_SMALLEST_NON_DENORMAL = *reinterpret_cast<const float*>(&SMALLEST_NON_DENORMAL);	// 1.1754944e-038f
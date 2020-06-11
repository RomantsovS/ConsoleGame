#ifndef MATH_H
#define MATH_H

#include <math.h>

#ifdef INFINITY
#undef INFINITY
#endif

#define MS2SEC(t)				( (t) * idMath::M_MS2SEC )

/*
================================================================================================

	floating point bit layouts according to the IEEE 754-1985 and 754-2008 standard

================================================================================================
*/

const size_t IEEE_FLT_MANTISSA_BITS = 23;

template<class T> inline T	Square(T x) { return x * x; }

class idMath {
public:
	static float InvSqrt(float x); // inverse square root with 32 bits precision, returns huge number when x == 0.0

	static float Sqrt(float x); // square root with 32 bits precision

	static float Fabs(float f); // returns the absolute value of the floating point value

	static int FloatHash(const float* array, const int numFloats);

	template<typename T>
	static int ValueInRange(T val, T min, T max);

	static const float M_MS2SEC; // milliseconds to seconds multiplier
	static const float INFINITY; // huge number which should be larger than any valid number used
	static const float FLT_SMALLEST_NON_DENORMAL;	// smallest non-denormal 32-bit floating point value
};

/*
========================
idMath::InvSqrt
========================
*/
inline float idMath::InvSqrt(float x) {
#ifdef ID_WIN_X86_SSE_INTRIN

	return (x > FLT_SMALLEST_NON_DENORMAL) ? sqrtf(1.0f / x) : INFINITY;

#else

	return (x > FLT_SMALLEST_NON_DENORMAL) ? sqrtf(1.0f / x) : INFINITY;

#endif
}

/*
========================
idMath::Sqrt
========================
*/
inline float idMath::Sqrt(float x) {
#ifdef ID_WIN_X86_SSE_INTRIN
	return (x >= 0.0f) ? x * InvSqrt(x) : 0.0f;
#else
	return (x >= 0.0f) ? sqrtf(x) : 0.0f;
#endif
}

/*
========================
idMath::Fabs
========================
*/
inline float idMath::Fabs(float f) {
	return fabsf(f);
}

/*
========================
idMath::FloatHash
========================
*/
inline int idMath::FloatHash(const float* array, const int numFloats) {
	int i, hash = 0;
	const int* ptr;

	ptr = reinterpret_cast<const int*>(array);
	for (i = 0; i < numFloats; i++) {
		hash ^= ptr[i];
	}
	return hash;
}

template<typename T>
inline int idMath::ValueInRange(T val, T min, T max)
{
	if (val < min)
		return -1;
	else if (val > max)
		return 1;

	return 0;
}

#endif // !MATH_H

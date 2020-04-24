#ifndef MATH_H
#define MATH_H

#include <math.h>

#define MS2SEC(t)				( (t) * idMath::M_MS2SEC )

template<class T> inline T	Square(T x) { return x * x; }

class idMath {
public:
	static float Sqrt(float x); // square root with 32 bits precision

	static float Fabs(float f); // returns the absolute value of the floating point value

	static int FloatHash(const float* array, const int numFloats);

	static const float M_MS2SEC; // milliseconds to seconds multiplier
};

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

#endif // !MATH_H

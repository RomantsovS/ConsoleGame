#ifndef MATH_H
#define MATH_H

#ifdef INFINITY
#undef INFINITY
#endif

#define SEC2MS(t)				( static_cast<float>( (t) * idMath::M_SEC2MS ) )
#define MS2SEC(t)				( (t) * idMath::M_MS2SEC )

/*
========================
IEEE_FLT_IS_NAN
========================
*/
extern inline bool IEEE_FLT_IS_NAN(float x) {
	return x != x;
}

/*
========================
IEEE_FLT_IS_INF
========================
*/
extern inline bool IEEE_FLT_IS_INF(float x) {
	return x == x && x * 0 != x * 0;
}

/*
========================
IEEE_FLT_IS_INF_NAN
========================
*/
extern inline bool IEEE_FLT_IS_INF_NAN(float x) {
	return x * 0 != x * 0;
}

/*
========================
IEEE_FLT_IS_IND
========================
*/
extern inline bool IEEE_FLT_IS_IND(float x) {
	return	(reinterpret_cast<const unsigned int&>(x) == 0xffc00000);
}

/*
========================
IEEE_FLT_IS_DENORMAL
========================
*/
extern inline bool IEEE_FLT_IS_DENORMAL(float x) {
	return ((reinterpret_cast<const unsigned int&>(x) & 0x7f800000) == 0x00000000 &&
		(reinterpret_cast<const unsigned int&>(x) & 0x007fffff) != 0x00000000);
}

/*
========================
IsValid
========================
*/
template<class type>
inline bool IsValid(const type& v) {
	for (int i = 0; i < v.GetDimension(); i++) {
		const float f = v.ToFloatPtr()[i];
		if (IEEE_FLT_IS_NAN(f) || IEEE_FLT_IS_INF(f) || IEEE_FLT_IS_IND(f) || IEEE_FLT_IS_DENORMAL(f)) {
			return false;
		}
	}
	return true;
}

/*
========================
IsValid
========================
*/
template<>
inline bool IsValid(const float& f) {	// these parameter must be a reference for the function to be considered a specialization
	return !(IEEE_FLT_IS_NAN(f) || IEEE_FLT_IS_INF(f) || IEEE_FLT_IS_IND(f) || IEEE_FLT_IS_DENORMAL(f));
}

/*
================================================================================================

	floating point bit layouts according to the IEEE 754-1985 and 754-2008 standard

================================================================================================
*/

const size_t IEEE_FLT_MANTISSA_BITS = 23;

template<class T> inline T	Square(T x) noexcept { return x * x; }

class idMath {
public:
	static float InvSqrt(float x) noexcept; // inverse square root with 32 bits precision, returns huge number when x == 0.0

	static float Sqrt(float x) noexcept; // square root with 32 bits precision

	static float Fabs(float f) noexcept; // returns the absolute value of the floating point value

	static signed char ClampChar(int i) noexcept;
	static int ClampInt(int min, int max, int value);

	static int FloatHash(gsl::span<const float> arr) noexcept;

	static const float M_MS2SEC; // milliseconds to seconds multiplier
	static const float M_SEC2MS; // seconds to milliseconds multiplier
	static const float INFINITY; // huge number which should be larger than any valid number used
	static const float FLT_SMALLEST_NON_DENORMAL;	// smallest non-denormal 32-bit floating point value
};

/*
========================
idMath::InvSqrt
========================
*/
inline float idMath::InvSqrt(float x) noexcept {
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
inline float idMath::Sqrt(float x) noexcept {
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
inline float idMath::Fabs(float f) noexcept {
	return fabsf(f);
}

/*
========================
idMath::ClampChar
========================
*/
inline signed char idMath::ClampChar(int i) noexcept {
	if (i < -128) {
		return -128;
	}
	if (i > 127) {
		return 127;
	}
	return static_cast<signed char>(i);
}

/*
========================
idMath::ClampInt
========================
*/
inline int idMath::ClampInt(int min, int max, int value) {
	if (value < min) {
		return min;
	}
	if (value > max) {
		return max;
	}
	return value;
}

/*
========================
idMath::FloatHash
========================
*/
inline int idMath::FloatHash(gsl::span<const float> arr) noexcept {
	int hash = 0;

	const float a[] = { 1, 2, 3 };
	gsl::span<const float> s(a);

	gsl::span<const int> ptr(reinterpret_cast<const int*>(&arr[0]), arr.size());
	for (auto iter : ptr) {
		hash ^= iter;
	}
	return hash;
}

template< typename T >
extern inline T Lerp(const T from, const T to, float f) {
	return from + ((to - from) * f);
}

template<>
extern inline int Lerp(const int from, const int to, float f) {
	return static_cast<int>(static_cast<float>(from) + ((static_cast<float>(to) - static_cast<float>(from)) * f));
}

#endif // !MATH_H

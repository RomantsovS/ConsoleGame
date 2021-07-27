#include <precompiled.h>
#pragma hdrstop

//===============================================================
//
//	idODE_Euler
//
//===============================================================

/*
=============
idODE_Euler::idODE_Euler
=============
*/
idODE_Euler::idODE_Euler(const int dim, deriveFunction_t dr, const void* ud) {
	dimension = dim;
	derivatives.resize(dim);
	derive = dr;
	userData = ud;
}

/*
=============
idODE_Euler::~idODE_Euler
=============
*/
idODE_Euler::~idODE_Euler() {
	derivatives.clear();
}

/*
=============
idODE_Euler::Evaluate
=============
*/
float idODE_Euler::Evaluate(gsl::span<const float> state, gsl::span<float> newState, float t0, float t1) noexcept {
	float delta;
	int i{0};

	derive(t0, userData, state, derivatives.data());
	delta = t1 - t0;
	for (auto& ns : newState) {
		ns = state[i] + delta * derivatives[i];
		++i;
	}
	return delta;
}
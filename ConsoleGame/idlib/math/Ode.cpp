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
float idODE_Euler::Evaluate(const float* state, float* newState, float t0, float t1) {
	float delta;
	int i;

	derive(t0, userData, state, derivatives.data());
	delta = t1 - t0;
	for (i = 0; i < dimension; i++) {
		newState[i] = state[i] + delta * derivatives[i];
	}
	return delta;
}
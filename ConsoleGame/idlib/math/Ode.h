#ifndef IDLIB_MATH_ODE_H_
#define IDLIB_MATH_ODE_H_

/*
===============================================================================

	Numerical solvers for ordinary differential equations.

===============================================================================
*/


//===============================================================
//
//	idODE
//
//===============================================================

typedef void (*deriveFunction_t)(const float t, const void* userData, const float* state, float* derivatives);

class idODE {

public:
	virtual				~idODE() {}

	virtual float		Evaluate(const float* state, float* newState, float t0, float t1) = 0;

protected:
	int					dimension;		// dimension in floats allocated for
	deriveFunction_t	derive;			// derive function
	const void* userData;		// client data
};

//===============================================================
//
//	idODE_Euler
//
//===============================================================

class idODE_Euler : public idODE {

public:
	idODE_Euler(const int dim, const deriveFunction_t dr, const void* ud);
	virtual				~idODE_Euler();

	virtual float		Evaluate(const float* state, float* newState, float t0, float t1);

protected:
	std::vector<float> derivatives;	// space to store derivatives
};

#endif // !IDLIB_MATH_ODE_H_

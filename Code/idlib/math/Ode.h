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

typedef void (*deriveFunction_t)(const float t, const void* userData,
                                 gsl::span<const float> state,
                                 float* derivatives);

class idODE {
 public:
  idODE() = default;
  virtual ~idODE() = default;
  idODE(const idODE&) = default;
  idODE& operator=(const idODE&) = default;
  idODE(idODE&&) = default;
  idODE& operator=(idODE&&) = default;

  virtual float Evaluate(gsl::span<const float> state,
                         gsl::span<float> newState, float t0, float t1) = 0;

  size_t GetDimension() noexcept { return dimension; }

 protected:
  int dimension;            // dimension in floats allocated for
  deriveFunction_t derive;  // derive function
  const void* userData;     // client data
};

//===============================================================
//
//	idODE_Euler
//
//===============================================================

class idODE_Euler : public idODE {
 public:
  idODE_Euler(const size_t dim, const deriveFunction_t dr, const void* ud);
  virtual ~idODE_Euler();
  idODE_Euler(const idODE_Euler&) = default;
  idODE_Euler& operator=(const idODE_Euler&) = default;
  idODE_Euler(idODE_Euler&&) = default;
  idODE_Euler& operator=(idODE_Euler&&) = default;

  float Evaluate(gsl::span<const float> state, gsl::span<float> newState,
                 float t0, float t1) noexcept override;

 protected:
  std::vector<float> derivatives;  // space to store derivatives
};

#endif  // !IDLIB_MATH_ODE_H_

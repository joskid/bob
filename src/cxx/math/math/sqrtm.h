/**
 * @file src/cxx/math/math/sqrt.h
 * @author <a href="mailto:Laurent.El-Shafey@idiap.ch">Laurent El Shafey</a> 
 *
 * @brief This file defines a function to compute the (unique) square root of
 * a real symmetric definite-positive matrix.
 * 
 */

#ifndef BOB5SPRO_MATH_SQRTM_H
#define BOB5SPRO_MATH_SQRTM_H

#include <blitz/array.h>
#include "eig.h"

namespace bob {
/**
 * \ingroup libmath_api
 * @{
 *
 */
  namespace math {

    /**
      * @brief Function which computes the (unique) square root of a real 
      *   symmetric definite-positive matrix.
      * @warning The input matrix should be symmetric.
      * @param A The A matrix to decompose (size NxN)
      * @param B The square root matrix B of A (size NxN)
      */
    void sqrtSymReal(const blitz::Array<double,2>& A, blitz::Array<double,2>& B); 
    void sqrtSymReal_(const blitz::Array<double,2>& A, blitz::Array<double,2>& B);

  }
/**
 * @}
 */
}

#endif /* BOB5SPRO_MATH_SQRTM_H */

/**
 * @file math/cxx/test/interiorpointLP.cc
 * @date Thu Mar 31 14:32:14 2011 +0200
 * @author Laurent El Shafey <Laurent.El-Shafey@idiap.ch>
 *
 * @brief Test the interior point methods to solve LP
 *
 * Copyright (C) 2011-2012 Idiap Research Institute, Martigny, Switzerland
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE math-interiorpoint Tests
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <blitz/array.h>
#include <stdint.h>
#include "bob/core/cast.h"
#include "bob/core/array_check.h"
#include "bob/core/array_copy.h"
#include "bob/core/array_type.h"
#include "bob/math/linear.h"
#include "bob/math/interiorpointLP.h"

struct T {
  double eps;

  T():eps(1e-4) {}

  ~T() {}
};

template<typename T, typename U, int d>  
void check_dimensions( blitz::Array<T,d>& t1, blitz::Array<U,d>& t2) 
{
  BOOST_REQUIRE_EQUAL(t1.dimensions(), t2.dimensions());
  for( int i=0; i<t1.dimensions(); ++i)
    BOOST_CHECK_EQUAL(t1.extent(i), t2.extent(i));
}

template<typename T, typename U>  
void checkBlitzEqual( blitz::Array<T,1>& t1, blitz::Array<U,1>& t2)
{
  check_dimensions( t1, t2);
  for( int i=0; i<t1.extent(0); ++i)
    BOOST_CHECK_EQUAL(t1(i), bob::core::cast<T>(t2(i)));
}

template<typename T, typename U>  
void checkBlitzEqual( blitz::Array<T,2>& t1, blitz::Array<U,2>& t2)
{
  check_dimensions( t1, t2);
  for( int i=0; i<t1.extent(0); ++i)
    for( int j=0; j<t1.extent(1); ++j)
      BOOST_CHECK_EQUAL(t1(i,j), bob::core::cast<T>(t2(i,j)));
}

template<typename T, typename U>  
void checkBlitzEqual( blitz::Array<T,3>& t1, blitz::Array<U,3>& t2) 
{
  check_dimensions( t1, t2);
  for( int i=0; i<t1.extent(0); ++i)
    for( int j=0; j<t1.extent(1); ++j)
      for( int k=0; k<t1.extent(2); ++k)
        BOOST_CHECK_EQUAL(t1(i,j,k), bob::core::cast<T>(t2(i,j,k)));
}

template<typename T>  
void checkBlitzClose( blitz::Array<T,1>& t1, blitz::Array<T,1>& t2, 
  const double eps )
{
  check_dimensions( t1, t2);
  for( int i=0; i<t1.extent(0); ++i)
    BOOST_CHECK_SMALL( fabs( t2(i)-t1(i) ), eps);
}

void generateProblem( const int n, blitz::Array<double,2>& A,
  blitz::Array<double,1>& b, blitz::Array<double,1>& c, 
  blitz::Array<double,1>& x0)
{
  bob::core::array::reindexAndResize( A, 0, 0, n, 2*n);
  bob::core::array::reindexAndResize( b, 0, n);
  bob::core::array::reindexAndResize( c, 0, 2*n);
  bob::core::array::reindexAndResize( x0, 0, 2*n);
  A = 0.;
  c = 0.;
  for( int i=0; i<n; ++i) {
    A(i,i) = 1.;
    A(i,n+i) = 1.;
    for( int j=i+1; j<n; ++j)
      A(j,i) = pow(2., 1+j);
    b(i) = pow(5.,i+1);
    c(i) = -pow(2., n-1-i);
    x0(i) = 1.;
  }
  blitz::Array<double,2> A1 = A(blitz::Range::all(), blitz::Range( 0, n-1));
  blitz::Array<double,1> ones(n);
  ones = 1.;
  blitz::Array<double,1> A1_1(n);
  bob::math::prod(A1, ones, A1_1);
  for( int i=0; i<n; ++i) {
    x0(n+i) = b(i) - A1_1(i);
  }
}

BOOST_FIXTURE_TEST_SUITE( test_setup, T )


BOOST_AUTO_TEST_CASE( test_solve_3x3 )
{
  blitz::Array<double,2> A;
  blitz::Array<double,1> b;
  blitz::Array<double,1> c;
  blitz::Array<double,1> x0;
  blitz::Array<double,1> sol;

  // Check problem 1 for dimension 1 to 10
  for( int n=1; n<=10; ++n)
  {
    generateProblem(n, A, b, c, x0);
    sol.resize(n);
    sol = 0.;
    sol(n-1) = pow(5., n); // Solution to problem 1 is [0 ... 0 5^n] 

    // Short step
    blitz::Array<double,1> x2(bob::core::array::ccopy(x0));
    bob::math::interiorpointShortstepLP(A, b, c, 0.4, x2, 1e-6);
    for( int i=0; i<n; ++i)
      BOOST_CHECK_SMALL( fabs( x2(i+x2.lbound(0))-sol(i)), eps);

    // Predictor corrector
    blitz::Array<double,1> x3(bob::core::array::ccopy(x0));
    bob::math::interiorpointPredictorCorrectorLP(
      A, b, c, 0.5, 0.25, x3, 1e-6);
    for( int i=0; i<n; ++i)
      BOOST_CHECK_SMALL( fabs( x3(i+x3.lbound(0))-sol(i)), eps);

    // Long step
    blitz::Array<double,1> x4(bob::core::array::ccopy(x0));
    bob::math::interiorpointLongstepLP(A, b, c, 1e-3, 0.1, x4, 1e-6);
    for( int i=0; i<n; ++i)
      BOOST_CHECK_SMALL( fabs( x4(i+x4.lbound(0))-sol(i)), eps);
  }
}
  
BOOST_AUTO_TEST_CASE( test_detail_neighborhood )
{
  // Test math::detail::isPositive
  blitz::Array<double,1> a1(4), a2(5);
  a1 = 2., 1., 0.1, 0.2;
  BOOST_CHECK_EQUAL( true, bob::math::detail::isPositive(a1) );
  a2 = -2., -1., 0.1, 0.2, -0.6;
  BOOST_CHECK_EQUAL( false, bob::math::detail::isPositive(a2) );

  // Test math::detail::isFeasible
  blitz::Array<double,2> A;
  blitz::Array<double,1> b;
  blitz::Array<double,1> c;
  blitz::Array<double,1> x0;
  const int n=2;
  generateProblem(n, A, b, c, x0);

  blitz::Array<double,1> x(4), lambda(2), mu1(4), mu2(4);
  x = 1.0532, 1.4341, 3.9468, 19.3532;
  lambda = -1.5172, -3.2922;
  mu1 = 12.6859, 2.2922, 1.5172, 3.2922;
  mu2 = 12.6859, 2.2922, 1.5172, 7.2922;
  BOOST_CHECK_EQUAL( true, 
    bob::math::detail::isFeasible(A,b,c,x,lambda,mu1,eps) );
  BOOST_CHECK_EQUAL( false, 
    bob::math::detail::isFeasible(A,b,c,x,lambda,mu2,eps) );

  // Test math::detail::isInVinf
  const double gamma1=1e-3, gamma2=1;
  BOOST_CHECK_EQUAL( true,
    bob::math::detail::isInVinf(x,mu1,gamma1) );
  BOOST_CHECK_EQUAL( false,
    bob::math::detail::isInVinf(x,mu1,gamma2) );

  // Test math::detail::isInV2
  const double theta1=0.5, theta2=0.03;
  blitz::Array<double,1> x3(4), mu3(4);
  x3 = 0.5562, 21.5427, 4.4438, 1.2327;
  mu3 = 2.4616, 0.0506, 0.2590, 1.0506;
  BOOST_CHECK_EQUAL( true,
    bob::math::detail::isInV2(x3,mu3,theta1) );
  BOOST_CHECK_EQUAL( false,
    bob::math::detail::isInV2(x3,mu3,theta2) );
}

BOOST_AUTO_TEST_CASE( test_detail_barrier )
{
  blitz::Array<double,2> A;
  blitz::Array<double,1> b;
  blitz::Array<double,1> c;
  blitz::Array<double,1> x0;
  const int n=2;
  generateProblem(n, A, b, c, x0);

  // Check logBarrier
  blitz::Array<double,1> lambda(n);
  lambda = -2., -6.;
  BOOST_CHECK_SMALL( fabs(bob::math::detail::logBarrierLP( 
      A.transpose(1,0), c, lambda) - -7.272398394), eps);

  // Check gradientLogBarrier
  blitz::Array<double,1> d_matlab(n);
  d_matlab = -0.54166666, -0.53333333;
  blitz::Array<double,1> work_ar = bob::core::array::ccopy(c);
  blitz::Array<double,1> d = bob::core::array::ccopy(lambda);
  bob::math::detail::gradientLogBarrierLP(A, c, lambda, work_ar, d);
  for( int i=0; i<n; ++i)
    BOOST_CHECK_SMALL( fabs(d_matlab(i)-d(i+d.lbound(0))), eps);
}

BOOST_AUTO_TEST_CASE( test_dual_variables_init )
{
  blitz::Array<double,2> A(2,4);
  A =  1., 0., 1., 0., 4., 1., 0., 1.;
  blitz::Array<double,1> c(4);
  c = -2., -1., 0., 0.;
  blitz::Array<double,1> lambda(2);
  lambda = 0.;
  blitz::Array<double,1> mu(4);
  mu = 0.;

  // Initialize lambda and mu
  bob::math::detail::initializeDualLambdaMuLP( A, c, lambda, mu);
  // Check that the point found fulfill the constraints:
  //  mu>=0 and transpose(A)*lambda+mu=c
  for( int i=mu.lbound(0); i<=mu.ubound(0); ++i )
    BOOST_CHECK_EQUAL( true, mu(i) >= 0);
  //  transpose(A)*lambda+mu=c
  blitz::Array<double,2> A_t = A.transpose(1,0);
  blitz::Array<double,1> left_vec(A_t.extent(0));
  bob::math::prod( A_t, lambda, left_vec);
  left_vec += mu;
  for( int i=c.lbound(0); i<=c.ubound(0); ++i )
    BOOST_CHECK_SMALL( fabs(left_vec(i)-c(i)), eps);

  blitz::Array<double,2> A_l;
  blitz::Array<double,1> b_l;
  blitz::Array<double,1> x_l;
  bob::math::detail::initializeLargeSystem( A, A_l, b_l, x_l);
}

BOOST_AUTO_TEST_SUITE_END()


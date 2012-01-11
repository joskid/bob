/**
 * @file cxx/sp/sp/convolution.h
 * @date Thu Jan 27 19:10:05 2011 +0100
 * @author Laurent El Shafey <Laurent.El-Shafey@idiap.ch>
 *
 * @brief Implement a blitz-based convolution product with zero padding
 *
 * Copyright (C) 2011 Idiap Reasearch Institute, Martigny, Switzerland
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

#ifndef BOB5SPRO_SP_CONVOLVE_H
#define BOB5SPRO_SP_CONVOLVE_H

#include "core/Exception.h"
#include "core/array_assert.h"
#include "core/array_copy.h"
#include "core/array_index.h"
#include <algorithm>
#include <blitz/array.h>

namespace tca = bob::core::array;

namespace bob {
  /**
   * \ingroup libsp_api
   * @{
   *
   */
  namespace sp {

    /**
     * @brief Enumerations of the possible options
     */
    namespace Convolution {
      enum SizeOption {
        Full,
        Same,
        Valid
      };

      enum BorderOption {
        Zero,
        NearestNeighbour,
        Circular,
        Mirror
      };
    }

    /**
     * @brief Gets the required size of the output of a 1D convolution product
     * @param b The size first input array
     * @param c The size second input array
     * @return The size of the output
     */
    inline const int getConvolveOutputSize(const int b, const int c,
      const enum Convolution::SizeOption size_opt = Convolution::Full)
    {
      int res=0;
      // Size of "B + C - 1"
      if( size_opt == Convolution::Full )
        res = std::max(0, b + c - 1);
      // Same size as B
      else if( size_opt == Convolution::Same )
        res = b;
      // Size when not allowing any padding
      else if( size_opt == Convolution::Valid )
        res = std::max(0, b - c + 1); 
      return res;
    }

    /**
     * @brief Gets the required size of the output of the convolution product
     * @param B The first input array B
     * @param C The second input array C
     * @param size_opt:  * Full: full size (default)
     *                   * Same: same size as the largest between B and C
     *                   * Valid: valid (part without padding)
     * @return Size of the output
     */
    template<typename T> 
    const blitz::TinyVector<int,1> getConvolveOutputSize(
      const blitz::Array<T,1>& B, const blitz::Array<T,1>& C,
      const enum Convolution::SizeOption size_opt = Convolution::Full)
    {
      blitz::TinyVector<int,1> size;
      size(0) = getConvolveOutputSize(B.extent(0), C.extent(0), size_opt);
      return size;
    }


    /**
     * @brief Gets the required size of the output of the convolution product
     * @param B The first input array B
     * @param C The second input array C
     * @param size_opt:  * Full: full size (default)
     *                   * Same: same size as the largest between B and C
     *                   * Valid: valid (part without padding)
     * @return Size of the output
     */
    template<typename T> 
    const blitz::TinyVector<int,2> getConvolveOutputSize(
      const blitz::Array<T,2>& B, const blitz::Array<T,2>& C,
      const enum Convolution::SizeOption size_opt = Convolution::Full)
    {
      blitz::TinyVector<int,2> size;
      size(0) = 0;
      size(1) = 0;

      if( size_opt == Convolution::Full )
      {
        size(0) = std::max(0, B.extent(0) + C.extent(0) - 1);
        size(1) = std::max(0, B.extent(1) + C.extent(1) - 1);
      }
      else if( size_opt == Convolution::Same )
      {
        size(0) = B.extent(0);
        size(1) = B.extent(1);
      }
      else if( size_opt == Convolution::Valid )
      {
        size(0) = std::max(0, B.extent(0) - C.extent(0) + 1);
        size(1) = std::max(0, B.extent(1) - C.extent(1) + 1);
      }

      return size;
    }

    /**
     * @brief Gets the required size of the output of the separable convolution product
     *        (Convolution of a X-D signal with a 1D kernel)
     * @param B The first input array B
     * @param C The second input array C
     * @param A The output array A=B*C convolved along the dimension dim
     * @param dim The dimension along which to convolve
     * @param size_opt:  * Full: full size (default)
     *                   * Same: same size as the largest between B and C
     *                   * Valid: valid (part without padding)
     * @return Size of the output
     */
    template<typename T, int N> 
    const blitz::TinyVector<int,N> getConvolveSepOutputSize(const blitz::Array<T,N>& B,
      const blitz::Array<T,1>& C, const int dim,
      const enum Convolution::SizeOption size_opt = Convolution::Full)
    {
      blitz::TinyVector<int,N> res;
      res = B.shape();
      if(dim<N)
      {
        int b_size_d = B.extent(dim);
        int c_size = C.extent(0);
        res(dim) = getConvolveOutputSize(b_size_d, c_size, size_opt);
      }
      else 
      {
        throw bob::core::Exception();
      }
      return res;
    }


    /**
     * @brief 1D convolution of blitz arrays: A=B*C
     * @param B The first input array B
     * @param C The second input array C
     * @param A The output array A=B*C
     * @param size_opt:  * Full: full size (default)
     *                   * Same: same size as the largest between B and C
     *                   * Valid: valid (part without padding)
     * @param border_opt:  * Zero: zero padding
     *                     * Nearest Neighbour: extrapolate with nearest 
     *                         neighbour
     *                     * Circular: extrapolate by considering tiled arrays
     *                         for B and C (<-> modulo arrays)
     *                     * Mirror: extrapolate by mirroring the arrays
     *                         for B and C
     * @warning The output A should have the correct size
     */
    template<typename T> void convolve(const blitz::Array<T,1>& B, 
      const blitz::Array<T,1>& C, blitz::Array<T,1>& A,
      const enum Convolution::SizeOption size_opt = Convolution::Full,
      const enum Convolution::BorderOption border_opt = Convolution::Zero)
    {
      const int Bsize = B.extent(0);
      const int Csize = C.extent(0);

      // Gets the expected size for the results
      const blitz::TinyVector<int,1> Asize = getConvolveOutputSize(B, C, size_opt);

      // Checks that A has the correct size and is zero base
      tca::assertSameShape(A, Asize);
      tca::assertZeroBase(A);
      // Checks that B and C are zero base
      tca::assertZeroBase(B);
      tca::assertZeroBase(C);
    
      A = 0;
      for(int i=0; i < Asize(0); ++i)
      {
        int i_shifted=0;
        if( size_opt == Convolution::Full )
          i_shifted = i;
        else if( size_opt == Convolution::Same )
          i_shifted = i + Csize / 2;

        if( size_opt == Convolution::Full || size_opt == Convolution::Same )
        {
          if( border_opt == Convolution::Zero )
          {
            int jl = ( i_shifted - (Csize-1) > 0 ? i_shifted - (Csize-1) : 0 );
            int jh = ( i_shifted < Bsize ? i_shifted : Bsize-1 ); 
            blitz::Range jb( jl, jh), jc( i_shifted-jl, i_shifted-jh, -1); 
            A(i) = blitz::sum(B(jb) * C(jc) );
          }
          else if( border_opt == Convolution::NearestNeighbour )
          {
            for(int j=i_shifted-(Csize-1); j <= i_shifted; ++j)
              A(i) += B( tca::keepInRange(j,0,Bsize-1)) * 
                C( tca::keepInRange(i_shifted-j,0,Csize-1));
          }
          else if( border_opt == Convolution::Circular )
          {
            for(int j=i_shifted-(Csize-1); j <= i_shifted; ++j)
              A(i) += B( (((j%Bsize)+Bsize) % Bsize) ) * 
                C( i_shifted-j );
          }
          else if( border_opt == Convolution::Mirror )
          {
            for(int j=i_shifted-(Csize-1); j <= i_shifted; ++j)
              A(i) += B( tca::mirrorInRange(j,0,Bsize-1) ) * 
                C( tca::mirrorInRange(i_shifted-j,0,Csize-1) );
          }
        }
        else if( size_opt == Convolution::Valid )
        {
          // Interpolation is useless in this case
          blitz::Range jb( i, i + Csize - 1), jc( Csize - 1, 0, -1); 
          A(i) = blitz::sum(B(jb) * C(jc) );
        }
        // Should be impossible to reach this point
        else
          throw bob::core::Exception();
      }
    }

    /**
     * @brief 2D convolution of 2D blitz arrays: A=B*C
     * @param B The first input array B
     * @param C The second input array C
     * @param A The output array A=B*C
     * @param size_opt:  * Full: full size (default)
     *                   * Same: same size as the largest between B and C
     *                   * Valid: valid (part without padding)
     * @param border_opt:  * Zero: zero padding
     *                     * Nearest Neighbour: extrapolate with nearest 
     *                         neighbour
     *                     * Circular: extrapolate by considering tiled arrays
     *                         fir B and C (<-> modulo arrays)
     *                     * Mirror: extrapolate by mirroring the arrays
     *                         for B and C
     * @warning The output A should have the correct size
     */
    template<typename T> void convolve(const blitz::Array<T,2>& B, 
      const blitz::Array<T,2>& C, blitz::Array<T,2>& A,
      const enum Convolution::SizeOption size_opt = Convolution::Full,
      const enum Convolution::BorderOption border_opt = Convolution::Zero)
    {
      int Bsize1 = B.extent(0);
      int Bsize2 = B.extent(1);
      int Csize1 = C.extent(0);
      int Csize2 = C.extent(1);

      // Gets the expected size for the results
      const blitz::TinyVector<int,2> Asize = getConvolveOutputSize(B, C, size_opt);

      // Checks that A has the correct size and is zero base
      tca::assertSameShape(A, Asize);
      tca::assertZeroBase(A);
      // Checks that B and C are zero base
      tca::assertZeroBase(B);
      tca::assertZeroBase(C);

      for(int i1=0; i1 < Asize(0); ++i1)
      {
        int i1_shifted=0;
        if( size_opt == Convolution::Full )
          i1_shifted = i1;
        else if( size_opt == Convolution::Same )
          i1_shifted = i1 + Csize1 / 2;

        for(int i2=0; i2 < Asize(1); ++i2)
        {
          A(i1,i2) = 0;

          int i2_shifted = i2;
          if( size_opt == Convolution::Full )
            i2_shifted = i2;
          else if( size_opt == Convolution::Same )
            i2_shifted = i2 + Csize2 / 2;

          if( size_opt == Convolution::Full || size_opt == Convolution::Same )
          {
            if( border_opt == Convolution::Zero )
            {
              int jl1 = ( i1_shifted - (Csize1-1) > 0 ? 
                i1_shifted - (Csize1-1) : 0 );
              int jh1 = ( i1_shifted < Bsize1 ? i1_shifted : Bsize1-1 );
              int jl2 = ( i2_shifted - (Csize2-1) > 0 ? 
                i2_shifted - (Csize2-1) : 0 );
              int jh2 = ( i2_shifted < Bsize2 ? i2_shifted : Bsize2-1 );
              blitz::Range jb1( jl1, jh1), jb2( jl2, jh2);
              blitz::Range jc1( i1_shifted-jl1, i1_shifted-jh1,-1), 
                           jc2( i2_shifted-jl2, i2_shifted-jh2,-1);
              A(i1,i2) = blitz::sum(B(jb1, jb2) * 
                            C(jc1, jc2) );
            }
            else if( border_opt == Convolution::NearestNeighbour )
            {
              for(int j1=i1_shifted-(Csize1-1); j1 <= i1_shifted; ++j1)
                for(int j2=i2_shifted-(Csize2-1); j2 <= i2_shifted; ++j2)
                  A(i1,i2) += B( tca::keepInRange(j1,0,Bsize1-1), 
                               tca::keepInRange(j2,0,Bsize2-1)) *
                    C( tca::keepInRange(i1_shifted-j1,0,Csize1-1),
                       tca::keepInRange(i2_shifted-j2,0,Csize2-1));
            }
            else if( border_opt == Convolution::Circular )
            {
              for(int j1=i1_shifted-(Csize1-1); j1 <= i1_shifted; ++j1)
                for(int j2=i2_shifted-(Csize2-1); j2 <= i2_shifted; ++j2)
                  A(i1,i2) += B( (((j1%Bsize1)+Bsize1) % Bsize1), 
                               (((j2%Bsize2)+Bsize2) % Bsize2)) * 
                    C( i1_shifted-j1, i2_shifted-j2);
            }
            else if( border_opt == Convolution::Mirror )
            {
              for(int j1=i1_shifted-(Csize1-1); j1 <= i1_shifted; ++j1)
                for(int j2=i2_shifted-(Csize2-1); j2 <= i2_shifted; ++j2)
                  A(i1,i2) += B( tca::mirrorInRange(j1,0,Bsize1-1), 
                               tca::mirrorInRange(j2,0,Bsize2-1)) *
                    C( tca::mirrorInRange(i1_shifted-j1,0,Csize1-1),
                       tca::mirrorInRange(i2_shifted-j2,0,Csize2-1));
            }
          }
          else if( size_opt == Convolution::Valid )
          {
            // Interpolation is useless in this case
            blitz::Range jb1( i1, i1 + Csize1 - 1), jb2( i2, i2 + Csize2 - 1);
            blitz::Range jc1( Csize1 - 1, 0,-1), jc2( Csize2 - 1, 0,-1);
            A(i1,i2) = blitz::sum(B(jb1, jb2) * 
                         C(jc1, jc2) );
          }
          // Should be impossible to reach this point
          else
            throw bob::core::Exception();
        }
      }
    }

    namespace detail {

      template<typename T> void convolveSep(const blitz::Array<T,2>& B, 
        const blitz::Array<T,1>& C, blitz::Array<T,2>& A,
        const enum Convolution::SizeOption size_opt = Convolution::Full,
        const enum Convolution::BorderOption border_opt = Convolution::Zero)
      {
        for(int i=0; i<B.extent(1); ++i)
        {
          const blitz::Array<T,1> Brow = B(blitz::Range::all(), i);
          blitz::Array<T,1> Arow = A(blitz::Range::all(), i);
          convolve(Brow, C, Arow, size_opt, border_opt);
        }
      }

     template<typename T> void convolveSep(const blitz::Array<T,3>& B, 
        const blitz::Array<T,1>& C, blitz::Array<T,3>& A,
        const enum Convolution::SizeOption size_opt = Convolution::Full,
        const enum Convolution::BorderOption border_opt = Convolution::Zero)
      {
        for(int i=0; i<B.extent(1); ++i)
          for(int j=0; j<B.extent(2); ++j)
          {
            const blitz::Array<T,1> Brow = B(blitz::Range::all(), i, j);
            blitz::Array<T,1> Arow = A(blitz::Range::all(), i, j);
            convolve(Brow, C, Arow, size_opt, border_opt);
          }
      }

      template<typename T> void convolveSep(const blitz::Array<T,4>& B, 
        const blitz::Array<T,1>& C, blitz::Array<T,4>& A,
        const enum Convolution::SizeOption size_opt = Convolution::Full,
        const enum Convolution::BorderOption border_opt = Convolution::Zero)
      {
        for(int i=0; i<B.extent(1); ++i)
          for(int j=0; j<B.extent(2); ++j)
            for(int k=0; k<B.extent(3); ++k)
            {
              const blitz::Array<T,1> Brow = B(blitz::Range::all(), i, j, k);
              blitz::Array<T,1> Arow = A(blitz::Range::all(), i, j, k);
              convolve(Brow, C, Arow, size_opt, border_opt);
            }
      }

    }

    /**
     * @brief Convolution of a X-D signal with a 1D kernel (for separable convolution)
     *        along the specified dimension (A=B*C)
     * @param B The first input array B
     * @param C The second input array C
     * @param A The output array A=B*C along the dimension d (0 or 1)
     * @param dim The dimension along which to convolve
     * @param size_opt:  * Full: full size (default)
     *                   * Same: same size as the largest between B and C
     *                   * Valid: valid (part without padding)
     * @param border_opt:  * Zero: zero padding
     *                     * Nearest Neighbour: extrapolate with nearest 
     *                         neighbour
     *                     * Circular: extrapolate by considering tiled arrays
     *                         for B and C (<-> modulo arrays)
     *                     * Mirror: extrapolate by mirroring the arrays
     *                         for B and C
     */
    template<typename T, int N> void convolveSep(const blitz::Array<T,N>& B, 
      const blitz::Array<T,1>& C, blitz::Array<T,N>& A, const int dim,
      const enum Convolution::SizeOption size_opt = Convolution::Full,
      const enum Convolution::BorderOption border_opt = Convolution::Zero)
    {
      // Gets the expected size for the results
      const blitz::TinyVector<int,N> Asize = getConvolveSepOutputSize(B, C, dim, size_opt);

      // Checks that A has the correct size and is zero base
      tca::assertSameShape(A, Asize);
      tca::assertZeroBase(A);
      // Checks that B and C are zero base
      tca::assertZeroBase(B);
      tca::assertZeroBase(C);

      if(dim==0)
        detail::convolveSep( B, C, A, size_opt, border_opt);
      else if(dim<N)
      {
        const blitz::Array<T,N> Bp = (bob::core::array::ccopy(B)).transpose(dim,0);
        blitz::Array<T,N> Ap = A.transpose(dim,0);
        detail::convolveSep( Bp, C, Ap, size_opt, border_opt);
      }
      else
        throw bob::core::Exception();
    }

  }
/**
 * @}
 */
}

#endif /* BOB5SPRO_SP_CONVOLVE_H */

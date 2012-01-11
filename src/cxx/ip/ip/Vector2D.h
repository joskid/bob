/**
 * @file cxx/ip/ip/Vector2D.h
 * @date Tue Jan 18 17:07:26 2011 +0100
 * @author André Anjos <andre.anjos@idiap.ch>
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
#ifndef VECTOR2D_INC
#define VECTOR2D_INC

#include "ip/Point2D.h"

namespace bob {

	class Matrix2D;

	/** This class is designed to handle a vector in 2D

	    @author Sebastien Marcel (marcel@idiap.ch)
	    @author Dan Sunday (http://www.softsurfer.com/)
	    @version 2.1
	    \date
	    @since 1.0
	*/
	class Vector2D : public Point2D
	{
	public:
		/** @name constructors (same as Point2D) */
		//@{
		///
		Vector2D() : Point2D() {};

		///
		Vector2D(int a) : Point2D(a) {};

		///
		Vector2D(double a) : Point2D(a) {};

		///
		Vector2D(int a, int b) : Point2D(a,b) {};

		///
		Vector2D(double a, double b) : Point2D(a,b) {};
		//@}

		/// destructor
		virtual ~Vector2D() {};

		//-----

		/** @name Vector2D Unary Operations */
		//@{
		/// Unary minus
		Vector2D operator-();

		/// Unary 2D perp operator
		Vector2D operator~();
		//@}

		//-----

		/** @name Vector2D Arithmetic Operations */
		//@{
		/// vector add
		Vector2D operator+(const Vector2D&);

		/// vector subtract
		Vector2D operator-(const Vector2D&);

		/// inner dot product
		double operator*(const Vector2D&);

		/// 2D exterior perp product
		double operator|(const Vector2D&);

		/// vector scalar mult
		Vector2D& operator*=(double);

		/// vector scalar div
		Vector2D& operator/=(double);

		/// vector increment
		Vector2D& operator+=(const Vector2D&);

		/// vector decrement
		Vector2D& operator-=(const Vector2D&);
		//@}

		//-----

		/** @name Vector Properties */
		//@{
		/// vector length
		double len();

		/// vector length squared (faster)
		double len2();
		//@}

		//-----

		/// convert vector to unit length
		void normalize();

		/// compute the angle (in radian) with the horizontal
		double angle();
	};

	//-----

	/** @name Vector2D scalar multiplication and divisions

	    @author Dan Sunday (http://www.softsurfer.com/)
	    @version 2.1
	    \date
	    @since 1.0
	*/
	//@{
	///
	Vector2D operator*(int, const Vector2D&);

	///
	Vector2D operator*(double, const Vector2D&);

	///
	Vector2D operator*(const Vector2D&, int);

	///
	Vector2D operator*(const Vector2D&, double);

	///
	Vector2D operator/(const Vector2D&, int);

	///
	Vector2D operator/(const Vector2D&, double);
	//@}

	//-----

	/** @name Vector2D rotation/scale

	    @author Sebastien Marcel (marcel@idiap.ch)
	    @version 2.1
	    \date
	    @since 1.0
	*/
	//@{
	///
	Vector2D operator*(const Vector2D&, const Matrix2D&);
	//@}

	/** @name vector 2D sums

	    @author Dan Sunday (http://www.softsurfer.com/)
	    @version 2.0
	    \date
	    @since 1.0
	*/
	//@{
	/// vector sum from int
	Vector2D sum(int, int *, const Vector2D *);

	/// vector sum from double
	Vector2D sum(int, double *, const Vector2D *);
	//@}
}

#endif

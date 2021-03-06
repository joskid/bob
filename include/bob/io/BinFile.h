/**
 * @file bob/io/BinFile.h
 * @date Wed Jun 22 17:50:08 2011 +0200
 * @author Andre Anjos <andre.anjos@idiap.ch>
 *
 * @brief This class can be used to load and store arrays from/to binary files.
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

#ifndef BOB_IO_BINFILE_H
#define BOB_IO_BINFILE_H

#include "bob/core/cast.h"
#include "bob/core/array.h"
#include "bob/io/BinFileHeader.h"
#include "bob/io/Exception.h"

namespace bob { namespace io {

  /**
   * Defines the flags that might be used when loading/storing a file
   * containing blitz arrays.
   */
  enum _BinFileFlag {
    _unset   = 0,
    _append  = 1L << 0,
    _in      = 1L << 3,
    _out     = 1L << 4
  };

  /**
   * This class can be used for loading and storing multiarrays from/to
   * binary files
   */
  class BinFile
  {
    public:
      /**
       * Defines the bitmask type for providing information about the type of
       * the stream.
       */
      typedef _BinFileFlag openmode;
      static const openmode append  = _append;
      static const openmode in      = _in;
      static const openmode out     = _out; 

      /**
       * Constructor
       */
      BinFile(const std::string& filename, openmode f);

      /**
       * Destructor
       */
      ~BinFile();

      /**
       * Tests if next operation will succeed.
       */
      inline bool operator!() const { return !m_stream; }

      /**
       * Closes the BinFile
       */
      void close();

      /** 
       * Puts an Array of a given type into the output stream/file. If the
       * type/shape have not yet been set, it is set according to the type
       * and shape given in the blitz array, otherwise the type/shape should
       * match or an exception is thrown.
       *
       * @warning: Please convert your files to HDF5, this format is
       * deprecated starting on 16.04.2011 - AA
       */
      void write(const bob::core::array::interface& a);

      /**
       * Loads a single array from the file. Checks if the array has the
       * necessary space, otherwise re-allocates it. 
       */
      void read(bob::core::array::interface& a);
      void read(size_t index, bob::core::array::interface& a);

      /**
       * Gets the Element type
       *
       * @warning An exception is thrown if nothing was written so far
       */
      inline bob::core::array::ElementType getElementType() const { 
        headerInitialized(); 
        return m_header.m_elem_type; 
      }

      /**
       * Gets the number of dimensions
       *
       * @warning An exception is thrown if nothing was written so far
       */
      inline size_t getNDimensions() const {  
        headerInitialized(); 
        return m_header.m_n_dimensions; 
      }

      /**
       * Gets the shape of each array
       *
       * @warning An exception is thrown if nothing was written so far
       */
      inline const size_t* getShape() const { 
        headerInitialized(); 
        return m_header.m_shape; 
      }

      /**
       * Gets the shape of each array in a blitz format
       *
       * @warning An exception is thrown if nothing was written so far
       */
      template<int D> inline void getShape (blitz::TinyVector<int,D>& res) 
        const {
          headerInitialized(); 
          m_header.getShape(res);
        }

      /**
       * Gets the number of samples/arrays written so far
       *
       * @warning An exception is thrown if nothing was written so far
       */
      inline size_t size() const { 
        return (m_header_init)? m_n_arrays_written : 0;
      }

      /**
       * Gets the number of elements per array
       *
       * @warning An exception is thrown if nothing was written so far
       */
      inline size_t getNElements() const { 
        headerInitialized(); 
        return m_header.getNElements(); 
      }

      /**
       * Gets the size along a particular dimension
       *
       * @warning An exception is thrown if nothing was written so far
       */
      inline size_t getSize(size_t dim_index) const { 
        headerInitialized(); 
        return m_header.getSize(dim_index); 
      }

      /**
       * Initializes the binary file with the given type and shape.
       */
      inline void initBinaryFile(bob::core::array::ElementType type,
          size_t ndim, const size_t* shape) {
        initHeader(type, ndim, shape);
      }

    private: //Some stuff I need privately

      /**
       * Checks if the end of the binary file is reached
       */
      inline void endOfFile() {
        if(m_current_array >= m_header.m_n_samples ) throw IndexError(m_current_array);
      }

      /** 
       * Checks that the header has been initialized, and raise an
       * exception if not
       */
      inline void headerInitialized() const { 
        if (!m_header_init) throw Uninitialized();
      }

      /**
       * Initializes the header of the (output) stream with the given type
       * and shape
       */
      void initHeader(bob::core::array::ElementType type, size_t ndim,
          const size_t* shape);

    private: //representation

      bool m_header_init;
      size_t m_current_array;
      size_t m_n_arrays_written;
      std::fstream m_stream;
      detail::BinFileHeader m_header;
      openmode m_openmode;
  };

  inline _BinFileFlag operator&(_BinFileFlag a, _BinFileFlag b) { 
    return _BinFileFlag(static_cast<int>(a) & static_cast<int>(b)); 
  }

  inline _BinFileFlag operator|(_BinFileFlag a, _BinFileFlag b) { 
    return _BinFileFlag(static_cast<int>(a) | static_cast<int>(b)); 
  }

  inline _BinFileFlag operator^(_BinFileFlag a, _BinFileFlag b) { 
    return _BinFileFlag(static_cast<int>(a) ^ static_cast<int>(b)); 
  }

  inline _BinFileFlag& operator|=(_BinFileFlag& a, _BinFileFlag b) { 
    return a = a | b; 
  }

  inline _BinFileFlag& operator&=(_BinFileFlag& a, _BinFileFlag b) { 
    return a = a & b; 
  }

  inline _BinFileFlag& operator^=(_BinFileFlag& a, _BinFileFlag b) { 
    return a = a ^ b; 
  }

  inline _BinFileFlag operator~(_BinFileFlag a) { 
    return _BinFileFlag(~static_cast<int>(a)); 
  }

}}

#endif /* BOB_IO_BINFILE_H */

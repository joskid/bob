/**
 * @file src/cxx/database/test/array.cc
 * @author <a href="mailto:laurent.el-shafey@idiap.ch">Laurent El Shafey</a> 
 *
 * @brief Database Array tests
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE DbArray Tests
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_array.hpp>

#include <blitz/array.h>
#include <complex>
#include "core/cast.h"
#include "database/BinFile.h"
#include "database/Array.h"

struct T {
  blitz::Array<double,1> a;
  blitz::Array<double,1> b;
  blitz::Array<uint32_t,1> c;

  blitz::Array<float,2> d;
  blitz::Array<float,2> e;
  blitz::Array<float,2> f;

  blitz::Array<double,4> g;
  blitz::Array<double,4> h;

  blitz::Array<std::complex<double>,1> cd1;
  blitz::Array<std::complex<double>,1> cd2;
  blitz::Array<std::complex<float>,1> cf1;

  T() {
    a.resize(4);
    a = 1, 2, 3, 4;
    c.resize(4);
    c = 1, 2, 3, 4;

    d.resize(2,2);
    d = 1, 2, 3, 4;
    e.resize(2,2);
    e = 5, 6, 7, 8;

    g.resize(2,3,4,5);
    g = 37.;
  
    cd1.resize(4);
    cd1 = std::complex<double>(3.,9.);
  }

  ~T() { }

};


/**
 * @brief Generates a unique temporary filename, and returns the file
 * descriptor
 */
std::string temp_file() {
  boost::filesystem::path tpl = Torch::core::tmpdir();
  tpl /= "torchtest_core_binformatXXXXXX.bin";
  boost::shared_array<char> char_tpl(new char[tpl.file_string().size()+1]);
  strcpy(char_tpl.get(), tpl.file_string().c_str());
  int fd = mkstemps(char_tpl.get(),4);
  close(fd);
  boost::filesystem::remove(char_tpl.get());
  std::string res = char_tpl.get();
  return res;
//  return char_tpl.get();
}

template<typename T, typename U> 
void check_equal(const blitz::Array<T,1>& a, const blitz::Array<U,1>& b) 
{
  BOOST_REQUIRE_EQUAL(a.extent(0), b.extent(0));
  for (int i=0; i<a.extent(0); ++i) {
    BOOST_CHECK_EQUAL(a(i), Torch::core::cast<T>(b(i)) );
  }
}

template<typename T, typename U> 
void check_equal(const blitz::Array<T,2>& a, const blitz::Array<U,2>& b) 
{
  BOOST_REQUIRE_EQUAL(a.extent(0), b.extent(0));
  BOOST_REQUIRE_EQUAL(a.extent(1), b.extent(1));
  for (int i=0; i<a.extent(0); ++i) {
    for (int j=0; j<a.extent(1); ++j) {
      BOOST_CHECK_EQUAL(a(i,j), Torch::core::cast<T>(b(i,j)));
    }
  }
}

template<typename T, typename U> 
void check_equal(const blitz::Array<T,4>& a, const blitz::Array<U,4>& b) 
{
  BOOST_REQUIRE_EQUAL(a.extent(0), b.extent(0));
  BOOST_REQUIRE_EQUAL(a.extent(1), b.extent(1));
  BOOST_REQUIRE_EQUAL(a.extent(2), b.extent(2));
  BOOST_REQUIRE_EQUAL(a.extent(3), b.extent(3));
  for (int i=0; i<a.extent(0); ++i) {
    for (int j=0; j<a.extent(1); ++j) {
      for (int k=0; k<a.extent(2); ++k) {
        for (int l=0; l<a.extent(3); ++l) {
          BOOST_CHECK_EQUAL(a(i,j,k,l), Torch::core::cast<T>(b(i,j,k,l)));
        }
      }
    }
  }
}

BOOST_FIXTURE_TEST_SUITE( test_setup, T )

BOOST_AUTO_TEST_CASE( dbArray_construction_get )
{
  // Create database Arrays from blitz::arrays and check properties

  // double,1
  Torch::database::Array db_a(a);
  BOOST_CHECK_EQUAL(db_a.getNDim(), a.dimensions());
  BOOST_CHECK_EQUAL(db_a.getElementType(), Torch::core::array::t_float64);
  BOOST_CHECK_EQUAL(db_a.isLoaded(), true);
  BOOST_CHECK_EQUAL(db_a.getFilename().size(), 0);
  BOOST_CHECK_EQUAL(db_a.getCodec().use_count(), 0);
  for(size_t i=0; i<db_a.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_a.getShape()[i], a.extent(i));
  check_equal( db_a.get<double,1>(), a );

  // float,2
  Torch::database::Array db_d(d);
  BOOST_CHECK_EQUAL(db_d.getNDim(), d.dimensions());
  BOOST_CHECK_EQUAL(db_d.getElementType(), Torch::core::array::t_float32);
  BOOST_CHECK_EQUAL(db_d.isLoaded(), true);
  BOOST_CHECK_EQUAL(db_d.getFilename().size(), 0);
  BOOST_CHECK_EQUAL(db_d.getCodec().use_count(), 0);
  for(size_t i=0; i<db_d.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_d.getShape()[i], d.extent(i));
  check_equal( db_d.get<float,2>(), d );

  // double,4
  Torch::database::Array db_g(g);
  BOOST_CHECK_EQUAL(db_g.getNDim(), g.dimensions());
  BOOST_CHECK_EQUAL(db_g.getElementType(), Torch::core::array::t_float64);
  BOOST_CHECK_EQUAL(db_g.isLoaded(), true);
  BOOST_CHECK_EQUAL(db_g.getFilename().size(), 0);
  BOOST_CHECK_EQUAL(db_g.getCodec().use_count(), 0);
  for(size_t i=0; i<db_g.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_g.getShape()[i], g.extent(i));
  check_equal( db_g.get<double,4>(), g );

  // Copy constructor
  Torch::database::Array db_g2(db_g);
  BOOST_CHECK_EQUAL(db_g2.getNDim(), g.dimensions());
  BOOST_CHECK_EQUAL(db_g2.getElementType(), Torch::core::array::t_float64);
  BOOST_CHECK_EQUAL(db_g2.isLoaded(), true);
  BOOST_CHECK_EQUAL(db_g2.getFilename().size(), 0);
  BOOST_CHECK_EQUAL(db_g2.getCodec().use_count(), 0);
  for(size_t i=0; i<db_g2.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_g2.getShape()[i], g.extent(i));
  check_equal( db_g2.get<double,4>(), g );
  
  // Assignment
  Torch::database::Array db_g3(a);
  db_g3 = db_g;
  BOOST_CHECK_EQUAL(db_g3.getNDim(), g.dimensions());
  BOOST_CHECK_EQUAL(db_g3.getElementType(), Torch::core::array::t_float64);
  BOOST_CHECK_EQUAL(db_g3.isLoaded(), true);
  BOOST_CHECK_EQUAL(db_g3.getFilename().size(), 0);
  BOOST_CHECK_EQUAL(db_g3.getCodec().use_count(), 0);
  for(size_t i=0; i<db_g3.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_g3.getShape()[i], g.extent(i));
  check_equal( db_g3.get<double,4>(), g );
  
}

BOOST_AUTO_TEST_CASE( dbArray_cast_blitz )
{
  // Create database Arrays from blitz::arrays and check properties

  // "Cast" to std::complex<double>,1
  Torch::database::Array db_cd1(cd1);
  cd2.resize(cd1.shape());
  cd2 = db_cd1.cast<std::complex<double>,1>();
  // Compare to initial array
  check_equal( cd1, cd2 );

  // Cast to std::complex<float>,1
  cf1.resize(cd1.shape());
  cf1 = db_cd1.cast<std::complex<float>,1>();
  // Compare to initial array
  check_equal( cd1, cf1 );
}

BOOST_AUTO_TEST_CASE( dbArray_creation_binaryfile )
{
  // Create a database Array from a blitz::array and save it to a binary file
  BOOST_REQUIRE_NO_THROW(Torch::database::Array db_a(a));
  Torch::database::Array db_a(a);

  std::string tmp_file = temp_file();
  BOOST_REQUIRE_NO_THROW(db_a.save( tmp_file));

  // Create a database Array from a binary file and check its properties
  BOOST_REQUIRE_NO_THROW(Torch::database::Array db_a_read(tmp_file));
  Torch::database::Array db_a_read(tmp_file);

  BOOST_CHECK_EQUAL(db_a_read.getNDim(), a.dimensions());
  BOOST_CHECK_EQUAL(db_a_read.getElementType(), Torch::core::array::t_float64);
  BOOST_CHECK_EQUAL(db_a_read.isLoaded(), false);
  BOOST_CHECK_EQUAL(db_a_read.getFilename().compare(tmp_file), 0);
  BOOST_CHECK_EQUAL(
    db_a_read.getCodec()->name().compare("torch.array.binary"), 0);
  for(size_t i=0; i<db_a_read.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_a_read.getShape()[i], a.extent(i));

  // Get a blitz array from the database Array and check that the values 
  // remain unchanged
  blitz::Array<double,1> bl_read = db_a_read.get<double,1>();
  BOOST_CHECK_EQUAL(db_a_read.isLoaded(), false);
  check_equal( a, bl_read);
}

BOOST_AUTO_TEST_CASE( dbArray_transform_getload )
{
  // Create a database Array from a blitz::array
  BOOST_REQUIRE_NO_THROW(Torch::database::Array db_a(a));
  Torch::database::Array db_a(a);

  BOOST_CHECK_EQUAL(db_a.getNDim(), a.dimensions());
  BOOST_CHECK_EQUAL(db_a.getElementType(), Torch::core::array::t_float64);
  BOOST_CHECK_EQUAL(db_a.isLoaded(), true);
  BOOST_CHECK_EQUAL(db_a.getFilename().size(), 0);
  BOOST_CHECK_EQUAL(db_a.getCodec().use_count(), 0);
  for(size_t i=0; i<db_a.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_a.getShape()[i], a.extent(i));
  
  // Save it to a binary file
  std::string tmp_file = temp_file();
  BOOST_REQUIRE_NO_THROW(db_a.save( tmp_file));
  BOOST_CHECK_EQUAL(db_a.getNDim(), a.dimensions());
  BOOST_CHECK_EQUAL(db_a.getElementType(), Torch::core::array::t_float64);
  BOOST_CHECK_EQUAL(db_a.isLoaded(), false);
  BOOST_CHECK_EQUAL(db_a.getFilename().compare(tmp_file), 0);
  BOOST_CHECK_EQUAL(db_a.getCodec()->name().compare("torch.array.binary"), 0);
  for(size_t i=0; i<db_a.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_a.getShape()[i], a.extent(i));

  // Call the get function and check that properties remain unchanged
  blitz::Array<double,1> a_get = db_a.get<double,1>();
  BOOST_CHECK_EQUAL(db_a.getNDim(), a.dimensions());
  BOOST_CHECK_EQUAL(db_a.getElementType(), Torch::core::array::t_float64);
  BOOST_CHECK_EQUAL(db_a.isLoaded(), false);
  BOOST_CHECK_EQUAL(db_a.getFilename().compare(tmp_file), 0);
  BOOST_CHECK_EQUAL(db_a.getCodec()->name().compare("torch.array.binary"), 0);
  for(size_t i=0; i<db_a.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_a.getShape()[i], a.extent(i));
  // Check that the 'get' array is unchanged
  check_equal( a, a_get);

  // Call the load function and check that properties are updated
  BOOST_REQUIRE_NO_THROW(db_a.load());
  BOOST_CHECK_EQUAL(db_a.getNDim(), a.dimensions());
  BOOST_CHECK_EQUAL(db_a.getElementType(), Torch::core::array::t_float64);
  BOOST_CHECK_EQUAL(db_a.isLoaded(), true);
  BOOST_CHECK_EQUAL(db_a.getFilename().size(), 0);
  BOOST_CHECK_EQUAL(db_a.getCodec().use_count(), 0);
  for(size_t i=0; i<db_a.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_a.getShape()[i], a.extent(i));
  // Check that the 'get' array is unchanged
  blitz::Array<double,1> a_load = db_a.get<double,1>();
  check_equal( a, a_load);
}

BOOST_AUTO_TEST_CASE( dbArray_transform_move )
{
  // Create a database Array from a blitz::array
  BOOST_REQUIRE_NO_THROW(Torch::database::Array db_a(a));
  Torch::database::Array db_a(a);

  BOOST_CHECK_EQUAL(db_a.getNDim(), a.dimensions());
  BOOST_CHECK_EQUAL(db_a.getElementType(), Torch::core::array::t_float64);
  BOOST_CHECK_EQUAL(db_a.isLoaded(), true);
  BOOST_CHECK_EQUAL(db_a.getFilename().size(), 0);
  BOOST_CHECK_EQUAL(db_a.getCodec().use_count(), 0);
  for(size_t i=0; i<db_a.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_a.getShape()[i], a.extent(i));
  
  // Save it to a binary file
  std::string tmp_file = temp_file();
  BOOST_REQUIRE_NO_THROW(db_a.save( tmp_file));
  BOOST_CHECK_EQUAL(db_a.getNDim(), a.dimensions());
  BOOST_CHECK_EQUAL(db_a.getElementType(), Torch::core::array::t_float64);
  BOOST_CHECK_EQUAL(db_a.isLoaded(), false);
  BOOST_CHECK_EQUAL(db_a.getFilename().compare(tmp_file), 0);
  BOOST_CHECK_EQUAL(db_a.getCodec()->name().compare("torch.array.binary"), 0);
  for(size_t i=0; i<db_a.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_a.getShape()[i], a.extent(i));
  // Check that the 'get' array is unchanged
  check_equal( a, db_a.get<double,1>());

  // Move it to another binary file
  std::string tmp_file2 = temp_file();
  BOOST_REQUIRE_NO_THROW(db_a.save( tmp_file2));
  BOOST_CHECK_EQUAL(db_a.getNDim(), a.dimensions());
  BOOST_CHECK_EQUAL(db_a.getElementType(), Torch::core::array::t_float64);
  BOOST_CHECK_EQUAL(db_a.isLoaded(), false);
  BOOST_CHECK_EQUAL(db_a.getFilename().compare(tmp_file2), 0);
  BOOST_CHECK_EQUAL(db_a.getCodec()->name().compare("torch.array.binary"), 0);
  for(size_t i=0; i<db_a.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_a.getShape()[i], a.extent(i));
  check_equal( a, db_a.get<double,1>());
}

BOOST_AUTO_TEST_CASE( dbArray_cast_inline )
{
  // Create a database Array from a blitz::array
  BOOST_REQUIRE_NO_THROW(Torch::database::Array db_a(a));
  Torch::database::Array db_a(a);

  // Call the cast function and check that properties remain unchanged
  blitz::Array<uint8_t,1> a_get_uint8 = db_a.cast<uint8_t,1>();
  blitz::Array<float,1> a_get_float = db_a.cast<float,1>();
  check_equal( a_get_uint8, a_get_float);

  // Create a database Array from a blitz::array
  Torch::database::Array db_g(g);

  // Call the cast function and check that properties remain unchanged
  blitz::Array<uint8_t,4> g_get_uint8 = db_g.cast<uint8_t,4>();
  blitz::Array<float,4> g_get_float = db_g.cast<float,4>();
  check_equal( g_get_uint8, g_get_float);
}

BOOST_AUTO_TEST_CASE( dbArray_cast_external )
{
  // Create a database Array from a blitz::array
  BOOST_REQUIRE_NO_THROW(Torch::database::Array db_a(a));
  Torch::database::Array db_a(a);
  // Save it to a binary file
  std::string tmp_file_a = temp_file();
  BOOST_REQUIRE_NO_THROW(db_a.save( tmp_file_a));

  // Call the cast function and check that properties remain unchanged
  blitz::Array<uint8_t,1> a_get_uint8 = db_a.cast<uint8_t,1>();
  blitz::Array<float,1> a_get_float = db_a.cast<float,1>();
  check_equal( a_get_uint8, a_get_float);

  // Create a database Array from a blitz::array
  BOOST_REQUIRE_NO_THROW(Torch::database::Array db_g(g));
  Torch::database::Array db_g(g);
  // Save it to a binary file
  std::string tmp_file_g = temp_file();
  BOOST_REQUIRE_NO_THROW(db_a.save( tmp_file_g));

  // Call the get function and check that properties remain unchanged
  blitz::Array<uint8_t,4> g_get_uint8 = db_g.cast<uint8_t,4>();
  blitz::Array<float,4> g_get_float = db_g.cast<float,4>();
  check_equal( g_get_uint8, g_get_float);
}

BOOST_AUTO_TEST_CASE( dbArray_set )
{
  // Create a database Array from a blitz::array
  Torch::database::Array db_a(a);
  check_equal( a, db_a.get<double,1>() );

  // Initialize a new blitz array
  b.resize(4);
  b = 5;
  b(0) = 37;
  // Call the set function and check that database Array and the blitz
  // Array have the same content
  db_a.set(b);
  check_equal( b, db_a.get<double,1>() );

  // Update b and check that the content of the database Array is identical,
  // as they are sharing the same storage.
  b(1) = 73;
  check_equal( b, db_a.get<double,1>() );


  // Create a database Array from a blitz::array
  Torch::database::Array db_g(g);
  check_equal( g, db_g.get<double,4>() );

  // Initialize a new blitz array
  h.resize(2,3,4,5);
  h = 5.;
  h(0,0,1,3) = 37.;
  // Call the set function and check that database Array and the blitz
  // Array have the same content
  db_g.set(h);
  check_equal( h, db_g.get<double,4>() );

  // Update b and check that the content of the database Array is identical,
  // as they are sharing the same storage.
  h(1,1,2,3) = 73.;
  check_equal( h, db_g.get<double,4>() );
}

BOOST_AUTO_TEST_CASE( dbArray_copy_constructor_inline )
{
  // Create a database Array from a blitz::array
  BOOST_CHECK_NO_THROW(Torch::database::Array db_a(a));
  Torch::database::Array db_a(a);

  BOOST_CHECK_EQUAL(db_a.getNDim(), a.dimensions());
  BOOST_CHECK_EQUAL(db_a.getElementType(), Torch::core::array::t_float64);
  BOOST_CHECK_EQUAL(db_a.isLoaded(), true);
  BOOST_CHECK_EQUAL(db_a.getFilename().size(), 0);
  BOOST_CHECK_EQUAL(db_a.getCodec().use_count(), 0);
  for(size_t i=0; i<db_a.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_a.getShape()[i], a.extent(i));

  // Test copy constructor
  BOOST_CHECK_NO_THROW(Torch::database::Array db_a_copy1(db_a));
  Torch::database::Array db_a_copy1(db_a);

  BOOST_CHECK_EQUAL(db_a.getNDim(), db_a_copy1.getNDim());
  BOOST_CHECK_EQUAL(db_a.getElementType(), db_a_copy1.getElementType());
  BOOST_CHECK_EQUAL(db_a.isLoaded(), db_a_copy1.isLoaded());
  BOOST_CHECK_EQUAL(db_a.getFilename().compare(
    db_a_copy1.getFilename()), 0);
  BOOST_CHECK_EQUAL(db_a.getCodec().use_count(), 
    db_a_copy1.getCodec().use_count());
  for(size_t i=0; i<db_a.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_a.getShape()[i], db_a_copy1.getShape()[i]);
  check_equal( db_a.get<double,1>(), db_a_copy1.get<double,1>() );

  // Test copy constructor (assignment)
  Torch::database::Array db_a_copy2 = db_a;
  BOOST_CHECK_EQUAL(db_a.getNDim(), db_a_copy2.getNDim());
  BOOST_CHECK_EQUAL(db_a.getElementType(), db_a_copy2.getElementType());
  BOOST_CHECK_EQUAL(db_a.isLoaded(), db_a_copy2.isLoaded());
  BOOST_CHECK_EQUAL(db_a.getFilename().compare(
    db_a_copy2.getFilename()), 0);
  BOOST_CHECK_EQUAL(db_a.getCodec().use_count(), 
    db_a_copy2.getCodec().use_count());
  for(size_t i=0; i<db_a.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_a.getShape()[i], db_a_copy2.getShape()[i]);
  check_equal( db_a.get<double,1>(), db_a_copy2.get<double,1>() );
}

BOOST_AUTO_TEST_CASE( dbArray_copy_constructor_external )
{
  // Create a database Array from a blitz::array
  BOOST_REQUIRE_NO_THROW(Torch::database::Array db_a(a));
  Torch::database::Array db_a(a);
  std::string tmp_file = temp_file();
  BOOST_REQUIRE_NO_THROW(db_a.save( tmp_file));
  BOOST_CHECK_EQUAL(db_a.getNDim(), a.dimensions());
  BOOST_CHECK_EQUAL(db_a.getElementType(), Torch::core::array::t_float64);
  BOOST_CHECK_EQUAL(db_a.isLoaded(), false);
  BOOST_CHECK_EQUAL(db_a.getFilename().compare(tmp_file), 0);
  BOOST_CHECK_EQUAL(db_a.getCodec()->name().compare("torch.array.binary"), 0);
  for(size_t i=0; i<db_a.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_a.getShape()[i], a.extent(i));

  // Test copy constructor
  BOOST_REQUIRE_NO_THROW(Torch::database::Array db_a_copy1(db_a));
  Torch::database::Array db_a_copy1(db_a);
  BOOST_CHECK_EQUAL(db_a.getNDim(), db_a_copy1.getNDim());
  BOOST_CHECK_EQUAL(db_a.getElementType(), db_a_copy1.getElementType());
  BOOST_CHECK_EQUAL(db_a.isLoaded(), db_a_copy1.isLoaded());
  BOOST_CHECK_EQUAL(db_a.getFilename().compare(
    db_a_copy1.getFilename()), 0);
  BOOST_CHECK_EQUAL(
    db_a.getCodec()->name().compare(db_a_copy1.getCodec()->name()), 0);
  for(size_t i=0; i<db_a.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_a.getShape()[i], db_a_copy1.getShape()[i]);
  check_equal( db_a.get<double,1>(), db_a_copy1.get<double,1>() );

  // Test copy constructor (assignment)
  BOOST_REQUIRE_NO_THROW(Torch::database::Array db_a_copy2 = db_a);
  Torch::database::Array db_a_copy2 = db_a;
  BOOST_CHECK_EQUAL(db_a.getNDim(), db_a_copy2.getNDim());
  BOOST_CHECK_EQUAL(db_a.getElementType(), db_a_copy2.getElementType());
  BOOST_CHECK_EQUAL(db_a.isLoaded(), db_a_copy2.isLoaded());
  BOOST_CHECK_EQUAL(db_a.getFilename().compare(
    db_a_copy2.getFilename()), 0);
  BOOST_CHECK_EQUAL(
    db_a.getCodec()->name().compare(db_a_copy2.getCodec()->name()), 0);
  for(size_t i=0; i<db_a.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_a.getShape()[i], db_a_copy2.getShape()[i]);
  check_equal( db_a.get<double,1>(), db_a_copy2.get<double,1>() );
}

BOOST_AUTO_TEST_SUITE_END()
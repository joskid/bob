/**
 * @file src/cxx/core/core/Dataset2.h
 * @author <a href="mailto:Laurent.El-Shafey@idiap.ch">Laurent El Shafey</a>
 * @author <a href="mailto:andre.anjos@idiap.ch">Andre Anjos</a>
 *
 * @brief A torch representation of a Dataset
 */

#ifndef TORCH5SPRO_CORE_DATASET_H
#define TORCH5SPRO_CORE_DATASET_H

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <blitz/array.h>
#include "core/logging.h"
#include "core/Exception.h"
#include "core/StaticComplexCast.h"
#include "core/dataset_common.h"

#include <string>
#include <map>
#include <cstdlib> // required when using size_t type



namespace Torch {   
  /**
   * \ingroup libcore_api
   * @{
   *
   */
  namespace core {
    class NonExistingElement: public Exception { };
    
    // Declare Arrayset for the reference to the parent Arrayset in the
    // Array class.
    class Arrayset;

    /**
     * @brief The array class for a dataset
     */
    class Array { //pure virtual
      public:
        /**
         * @brief Constructor
         */
        Array(const Arrayset& parent);
        /**
         * @brief Destructor
         */
        ~Array();
        
        /**
         * @brief Set the id of the Array
         */
        void setId(const size_t id) { m_id = id; }
        /**
         * @brief Set the flag indicating if this array is loaded.
         */
        void setIsLoaded(const bool is_loaded) { m_is_loaded = is_loaded; }
        /**
         * @brief Set the filename containing the data if any. An empty string
         * indicates that the data are stored in the XML file directly.
         */
        void setFilename(const std::string& filename)
          { m_filename.assign(filename); }
        /**
         * @brief Set the loader used to read the data from the external file 
         * if any.
         */
        void setLoader(const LoaderType loader) { m_loader = loader; }
        /**
         * @brief Set the data of the Array. Storage should have been allocated
         * with malloc, to make the deallocation easy? 
         */
        void setStorage(void* storage) { m_storage = storage; }

        /**
         * @brief Get the id of the Array
         */
        const size_t getId() const { return m_id; }
        /**
         * @brief Get the flag indicating if the array is loaded from an 
         * external file.
         */
        const bool getIsLoaded() const { return m_is_loaded; }
        /**
         * @brief Get the filename containing the data if any. An empty string
         * indicates that the data is stored in the XML file directly.
         */
        const std::string& getFilename() const { return m_filename; }
        /**
         * @brief Get the loader used to read the data from the external file 
         * if any.
         */
        const LoaderType getLoader() const { return m_loader; }
        /**
         * @brief Get a pointer to the storage area containing the data
         */
        const void* getStorage() const { 
          if(!getIsLoaded()) {
            ;//TODO:load
            Array* a=const_cast<Array*>(this);
            a->setIsLoaded(true);
          }          
          return m_storage; 
        }
        /**
         * @brief Get the parent arrayset of this array
         */
        const Arrayset& getParentArrayset() const { return m_parent_arrayset; }

        /**
         * @brief Adapt the size of each dimension of the passed blitz array
         * to the ones of the underlying array and copy the data in it.
         */
        template<typename T, int D> 
        void copy( blitz::Array<T,D>& output) const;
        /**
         * @brief Adapt the size of each dimension of the passed blitz array
         * to the ones of the underlying array and refer to the data in it.
         * @warning Updating the content of the blitz array will update the
         * content of the corresponding array in the dataset.
         */
        template<typename T, int D> 
        void refer( blitz::Array<T,D>& output);
        /************** Partial specialization declaration *************/
        template<int D> void refer( blitz::Array<bool,D>& output);
        template<int D> void refer( blitz::Array<int8_t,D>& output);
        template<int D> void refer( blitz::Array<int16_t,D>& output);
        template<int D> void refer( blitz::Array<int32_t,D>& output);
        template<int D> void refer( blitz::Array<int64_t,D>& output);
        template<int D> void refer( blitz::Array<uint8_t,D>& output);
        template<int D> void refer( blitz::Array<uint16_t,D>& output);
        template<int D> void refer( blitz::Array<uint32_t,D>& output);
        template<int D> void refer( blitz::Array<uint64_t,D>& output);
        template<int D> void refer( blitz::Array<float,D>& output);
        template<int D> void refer( blitz::Array<double,D>& output);
        template<int D> 
        void refer( blitz::Array<long double,D>& output);
        template<int D> 
        void refer( blitz::Array<std::complex<float>,D>& output);
        template<int D> 
        void refer( blitz::Array<std::complex<double>,D>& output);
        template<int D> 
        void refer( blitz::Array<std::complex<long double>,D>& output);

      private:
        template <typename T, typename U> void copyCast( U* out) const;
        template <typename T, int D> 
        void referCheck( blitz::Array<T,D>& output) const;

        const Arrayset& m_parent_arrayset;
        size_t m_id;
        bool m_is_loaded;
        std::string m_filename;
        LoaderType m_loader;
        void* m_storage;
    };


    /**
     * @brief The arrayset class for a dataset
     */
    class Arrayset {
      public:
        /**
         * @brief Constructor
         */
        Arrayset();
        /**
         * @brief Destructor
         */
        ~Arrayset();

        /**
         * @brief Add an Array to the Arrayset
         */
        void append( boost::shared_ptr<Array> array);
        /**
         * @brief Remove an Array with a given id from the Arrayset
         */
        void remove( const size_t id) {
          std::map<size_t, boost::shared_ptr<Array> >::iterator it =
            m_array.find(id);
          if(it!=m_array.end())
            m_array.erase(it);
          else
            throw NonExistingElement();
        }

        /**
         * @brief Set the id of the Arrayset
         */
        void setId(const size_t id) { m_id = id; }
        /**
         * @brief Set the number of dimensions of the arrays of this Arrayset
         */
        void setNDim(const size_t n_dim) { m_n_dim = n_dim; }
        /**
         * @brief Set the size of each dimension of the arrays of this 
         * Arrayset
         */
        void setShape(const size_t shape[]) { 
          for(size_t i=0; i<array::N_MAX_DIMENSIONS_ARRAY; ++i)
            m_shape[i] = shape[i];
        }
        /**
         * @brief Update the shape of the Array with the one given in the
         * blitz TinyVector.
         */
        template<int D> 
        void setShape( const blitz::TinyVector<int,D>& shape ) {
          m_n_dim = D;
          for( int i=0; i<array::N_MAX_DIMENSIONS_ARRAY; ++i)
            m_shape[i] = ( i<D ? shape(i) : 0);
        }
        /**
         * @brief Set the number of elements in each array of this 
         * Arrayset
         */
        void setNElem(const size_t n_elem) {  m_n_elem = n_elem; } 
        /**
         * @brief Set the type of the elements contained in the the arrays of 
         * this Arrayset
         */
        void setElementType(const array::ElementType element_type) 
          { m_element_type = element_type; }
        /**
         * @brief Set the role of the Arrayset
         */
        void setRole(const std::string& role) { m_role.assign(role); } 
        /**
         * @brief Set the flag indicating if this arrayset is loaded from an 
         * external file.
         */
        void setIsLoaded(const bool is_loaded) { m_is_loaded = is_loaded; }
        /**
         * @brief Set the filename containing the data if any. An empty string
         * indicates that the data are stored in the XML file directly.
         */
        void setFilename(const std::string& filename)
          { m_filename.assign(filename); }
        /**
         * @brief Set the loader used to read the data from the external file 
         * if any.
         */
        void setLoader(const LoaderType loader) { m_loader = loader; }
        
        /**
         * @brief Get the id of the Arrayset
         */
        size_t getId() const { return m_id; }
        /**
         * @brief Get the number of dimensions of the arrays of this Arrayset
         */
        size_t getNDim() const { return m_n_dim; }
        /**
         * @brief Get the size of each dimension of the arrays of this 
         * Arrayset
         */
        const size_t* getShape() const { return m_shape; }
        /**
         * @brief Update the given blitz array with the content of the array
         * of the provided id.
         */
        template<int D> void getShape( blitz::TinyVector<int,D>& res ) const {
          if( D!=m_n_dim )
            throw NDimensionError();
          for( int i=0; i<D; ++i)
            res[i] = m_shape[i];
        }
        /**
         * @brief Get the number of elements in each array of this 
         * Arrayset
         */
        size_t getNElem() const { return m_n_elem; } 
        /**
         * @brief Get the type of the elements contained in the the arrays of 
         * this Arrayset
         */
        array::ElementType getElementType() const { return m_element_type; }
        /**
         * @brief Get the role of this Arrayset
         */
        const std::string& getRole() const { return m_role; }
        /**
         * @brief Get the flag indicating if the arrayset is loaded from an 
         * external file.
         */
        bool getIsLoaded() const { return m_is_loaded; }
        /**
         * @brief Get the filename containing the data if any. An empty string
         * indicates that the data is stored in the XML file directly.
         */
        const std::string& getFilename() const { return m_filename; }
        /**
         * @brief Get the loader used to read the data from the external file 
         * if any.
         */
        LoaderType getLoader() const { return m_loader; }
        /**
         * @brief Get the number of arrays in this Arrayset
         */
        size_t getNArrays() const { return m_array.size(); }


        /**
         * @brief const_iterator over the Arrays of the Arrayset
         */
        typedef std::map<size_t, boost::shared_ptr<Array> >::const_iterator 
          const_iterator;
        /**
         * @brief Return a const_iterator pointing at the first Array of the 
         * Arrayset
         */
        const_iterator begin() const { 
          if(!getIsLoaded()) {
            ;//TODO:load
            Arrayset* a=const_cast<Arrayset*>(this);
            a->setIsLoaded(true);
          }          
          return m_array.begin(); 
        }
        /**
         * @brief Return a const_iterator pointing at the last Array of the 
         * Arrayset
         */
        const_iterator end() const { 
          if(!getIsLoaded()) {
            ;//TODO:load
            Arrayset* a=const_cast<Arrayset*>(this);
            a->setIsLoaded(true);
          }          
          return m_array.end(); 
        }

        /**
         * @brief iterator over the Arrays of the Arrayset
         */
        typedef std::map<size_t, boost::shared_ptr<Array> >::iterator 
          iterator;
        /**
         * @brief Return an iterator pointing at the first Array of the 
         * Arrayset
         */
        iterator begin() { 
          if(!getIsLoaded()) {
            ;//TODO:load
            setIsLoaded(true);
          }          
          return m_array.begin(); 
        }
        /**
         * @brief Return an iterator pointing at the last Array of the 
         * Arrayset
         */
        iterator end() { 
          if(!getIsLoaded()) {
            ;//TODO:load
            setIsLoaded(true);
          }          
          return m_array.end(); 
        }

        /**
         * @brief Return the array of the given id
         * @warning Please note that if you use that method, scope matters,
         * because the dataset owns the arrays.
         */
        const Array& operator[](size_t id) const;
        /**
         * @brief Return a smart pointer to the array of the given id
         */
        boost::shared_ptr<const Array> getArray(size_t id) const;
        boost::shared_ptr<Array> getArray(size_t id);

        /**
         * @brief Update the blitz array with the content of the array 
         * of the provided id.
         */
        template<typename T, int D> 
        void at(size_t id, blitz::Array<T,D>& output) const;


      private:
        size_t m_id;

        size_t m_n_dim;
        size_t m_shape[array::N_MAX_DIMENSIONS_ARRAY];
        size_t m_n_elem;
        array::ElementType m_element_type;
        
        std::string m_role;
        bool m_is_loaded;
        std::string m_filename;
        LoaderType m_loader;

        std::map<size_t, boost::shared_ptr<Array> > m_array;
    };


    /**
     * @brief The member class for a dataset
     */
    class Member {
      public:
        /**
         * @brief Constructor
         */
        Member();

        /**
         * @brief Destructor
         */
        ~Member();

        /**
         * @brief Set the array-id for this member
         */
        void setArrayId(const size_t array_id) { m_array_id = array_id; }
        /**
         * @brief Get the array-id for this member
         */
        size_t getArrayId() const { return m_array_id; }

        /**
         * @brief Set the array-id for this member
         */
        void setArraysetId(const size_t arrayset_id) { 
          m_arrayset_id = arrayset_id; }
        /**
         * @brief Get the array-id for this member
         */
        size_t getArraysetId() const { return m_arrayset_id; }

      private:
        size_t m_array_id;
        size_t m_arrayset_id;
    };


    /**
     * @brief The relation class for a dataset
     */
    class Relation {
      public:
        /**
         * @brief Constructor
         */
        Relation(boost::shared_ptr<std::map<size_t,std::string> > id_role);

        /**
         * @brief Destructor
         */
        ~Relation();

        /**
         * @brief Add a member to the Relation
         */
        void append( boost::shared_ptr<Member> member);

        /**
         * @brief Set the id for this relation
         */
        void setId(const size_t id) { m_id = id; }
        /**
         * @brief Get the id for this relation
         */
        size_t getId() const { return m_id; }

        typedef std::pair<size_t, size_t> size_t_pair;
        /**
         * @brief const_iterator over the Members of the Relation
         */
        typedef std::map<size_t_pair, boost::shared_ptr<Member> >::const_iterator
          const_iterator;
        /**
         * @brief Return a const_iterator pointing at the first Member of 
         * the Relation
         */
        const_iterator begin() const { return m_member.begin(); }
        /**
         * @brief Return a const_iterator pointing at the last Member of 
         * the Relation
         */
        const_iterator end() const { return m_member.end(); }

        /**
         * @brief iterator over the Members of the Relation
         */
        typedef std::map<size_t_pair, boost::shared_ptr<Member> >::iterator 
          iterator;
        /**
         * @brief Return an iterator pointing at the first Member of the
         * Relation
         */
        iterator begin() { return m_member.begin(); }
        /**
         * @brief Return an iterator pointing at the last Member of the 
         * Relation
         */
        iterator end() { return m_member.end(); }


        /**
         * @brief iterator over the Members of the Relation with a given 
         * arrayset-role
         */
        template <typename T, typename U, typename V> 
        class iterator_template {
          public:
            /**
             * @brief Constructor
             */
            iterator_template(): m_str(""), m_it(0), m_parent(0) { }
            iterator_template(const std::string& str, V it, U* parent):
              m_str(str), m_it(it), m_parent(parent) { }

            T& operator*() const;
            T* operator->() const;
            iterator_template<T,U,V>& operator++(); // prefix
            iterator_template<T,U,V> operator++(int); // postfix
            bool operator==(const iterator_template<T,U,V>& i) const;
            bool operator!=(const iterator_template<T,U,V>& i) const;

          private:
            std::string m_str;
            V m_it;
            const U* m_parent;
        };
       
        /**
         * @warning Looking at the STL implementation of a map, the keys are 
         * const:
         * "template <typename _Key, typename _Tp, 
         *    typename _Compare = std::less<_Key>,
         *    typename _Alloc = std::allocator<std::pair<const _Key, _Tp> > >"
         * The following iterator typedefs take this fact into consideration,
         * and use a const size_t_pair as Keys type.
         */
        typedef iterator_template<std::pair<const size_t_pair, 
          boost::shared_ptr<Member> >, Relation, Relation::iterator> 
          iterator_b;
        typedef iterator_template<const std::pair<const size_t_pair,
          boost::shared_ptr<Member> >, const Relation, 
          Relation::const_iterator> const_iterator_b;

        /**
         * @brief Return an iterator pointing at the first Member of the 
         * Relation with a given arrayset-role
         */
        iterator_b begin(const std::string& str) {
          iterator it=begin();
          while( it!=end() &&
            m_id_role->operator[]( it->second->getArraysetId()).compare(str) )
            ++it;
          return iterator_b( str, it, this);
        }

        /**
         * @brief Return an iterator pointing at the last Member of the 
         * Relation with a given arrayset-role
         */
        iterator_b end(const std::string& str) {
          return iterator_b( str, end(), this);
        }

        /**
         * @brief Return an iterator pointing at the first Member of the 
         * Relation with a given arrayset-role
         */
        const_iterator_b begin(const std::string& str) const {
          const_iterator it=begin();
          while( it!=end() &&
            m_id_role->operator[]( it->second->getArraysetId()).compare(str) )
            ++it;
          return const_iterator_b( str, it, this);
        }

        /**
         * @brief Return an iterator pointing at the last Member of the 
         * Relation with a given arrayset-role
         */
        const_iterator_b end(const std::string& str) const {
          return const_iterator_b( str, end(), this);
        }


        boost::shared_ptr<std::map<size_t,std::string> > getIdRole() const {
          return m_id_role;
        }

      private:
        std::map<size_t_pair, boost::shared_ptr<Member> > m_member;
        size_t m_id;
        /**
         * @brief Mapping from arrayset-id to role
         */
        boost::shared_ptr<std::map<size_t,std::string> > m_id_role;
    };

    template <typename T, typename U, typename V> 
    T& Relation::iterator_template<T,U,V>::operator*() const {
      return *m_it;
    }

    template <typename T, typename U, typename V> 
    T* Relation::iterator_template<T,U,V>::operator->() const {
      return m_it.operator->();
    }

    template <typename T, typename U, typename V> 
    Relation::iterator_template<T,U,V>& 
    Relation::iterator_template<T,U,V>::operator++() {
      ++m_it;
      while( m_it!=m_parent->end() && 
        m_parent->getIdRole()->operator[]( 
          m_it->second->getArraysetId()).compare(m_str) )
        ++m_it;
      return *this;
    }

    template <typename T, typename U, typename V> 
    Relation::iterator_template<T,U,V> 
    Relation::iterator_template<T,U,V>::operator++(int) {
      m_it++;
      while( m_it!=m_parent->end() && 
        m_parent->getIdRole()->operator[]( 
          m_it->second->getArraysetId()).compare(m_str) )
        ++m_it;
      return *this;
    }

    template <typename T, typename U, typename V> 
    bool Relation::iterator_template<T,U,V>::operator==(
      const iterator_template<T,U,V>& it) const 
    {
      return m_it == it.m_it;
    }

    template <typename T, typename U, typename V> 
    bool Relation::iterator_template<T,U,V>::operator!=(
    const iterator_template<T,U,V>& it) const 
    {
      return m_it != it.m_it;
    }


    /**
     * @brief The rule class for a dataset
     */
    class Rule {
      public:
        /**
         * @brief Constructor
         */
        Rule();

        /**
         * @brief Destructor
         */
        ~Rule();

        /**
         * @brief Set the arraysetrole for this rule
         */
        void setArraysetRole(const std::string& arraysetrole) { 
          m_arraysetrole.assign(arraysetrole); }
        /**
         * @brief Set the minimum number of occurences for this rule
         */
        void setMin(const size_t min) { m_min = min; }
        /**
         * @brief Set the maximum number of occurences for this rule
         */
        void setMax(const size_t max) { m_max = max; }
        /**
         * @brief Get the arrayset role for this rule
         */
        const std::string& getArraysetRole() const { return m_arraysetrole; }
        /**
         * @brief Get the minimum number of occurences for this rule
         */
        size_t getMin() const { return m_min; }
        /**
         * @brief Get the maximum number of occurences for this rule
         */
        size_t getMax() const { return m_max; }

      private:
        std::string m_arraysetrole;
        size_t m_min;
        size_t m_max;
    };


    /**
     * @brief The relationset class for a dataset
     */
    class Relationset {
      public:
        /**
         * @brief Constructor
         */
        Relationset();

        /**
         * @brief Destructor
         */
        ~Relationset();

        /**
         * @brief Add a Relation to the Relationset
         */
        void append( boost::shared_ptr<Relation> relation);

        /**
         * @brief Add a Rule to the Relationset
         */
        void append( boost::shared_ptr<Rule> rule);

        /**
         * @brief Get the name of this Relationset
         */
        const std::string& getName() const { return m_name; }
        /**
         * @brief Set the name of this Relationset
         */
        void setName(const std::string& name) { m_name.assign(name); }

        /**
         * @brief const_iterator over the Relations of the Relationset
         */
        typedef std::map<size_t, boost::shared_ptr<Relation> >::const_iterator
          const_iterator;
        /**
         * @brief Return a const_iterator pointing at the first Relation of 
         * the Relationset
         */
        const_iterator begin() const { return m_relation.begin(); }
        /**
         * @brief Return a const_iterator pointing at the last Relation of 
         * the Relationset
         */
        const_iterator end() const { return m_relation.end(); }

        /**
         * @brief iterator over the Relations of the Relationset
         */
        typedef std::map<size_t, boost::shared_ptr<Relation> >::iterator 
          iterator;
        /**
         * @brief Return an iterator pointing at the first Relation of the
         * Relationset
         */
        iterator begin() { return m_relation.begin(); }
        /**
         * @brief Return an iterator pointing at the last Relation of the 
         * Relationset
         */
        iterator end() { return m_relation.end(); }

        /**
         * @brief const_iterator over the Rules of the Relationset
         */
        typedef std::map<std::string, boost::shared_ptr<Rule> >::const_iterator
          rule_const_iterator;
        /**
         * @brief Return a rule_const_iterator pointing at the first Rule of 
         * the Relationset
         */
        rule_const_iterator rule_begin() const { return m_rule.begin(); }
        /**
         * @brief Return a rule_const_iterator pointing at the last Rule of
         * the Relationet
         */
        rule_const_iterator rule_end() const { return m_rule.end(); }

        /**
         * @brief iterator over the Rules of the Relationset
         */
        typedef std::map<std::string, boost::shared_ptr<Rule> >::iterator 
          rule_iterator;
        /**
         * @brief Return an iterator pointing at the first Rule of the
         * Relationset
         */
        rule_iterator rule_begin() { return m_rule.begin(); }
        /**
         * @brief Return an iterator pointing at the last Rule of the 
         * Relationset
         */
        rule_iterator rule_end() { return m_rule.end(); }

        /**
         * @brief Return the relation of the given id
         * @warning Please note that if you use that method, scope matters,
         * because the dataset owns the relations.
         */
        const Relation& operator[](size_t id) const;
        /**
         * @brief Return a smart pointer to the relation of the given id
         */
        boost::shared_ptr<const Relation> getRelation(size_t id) const;
        boost::shared_ptr<Relation> getRelation(size_t id);

        /**
         * @brief Return the rule object referred by the given role 
         * @warning Please note that if you use that method, scope matters,
         * because the dataset owns the rules.
         */
        const Rule& operator[](const std::string& name) const;
        /**
         * @brief Return a smart pointer to the rule given the role
         */
        boost::shared_ptr<const Rule> getRule(const std::string& name) const;
        boost::shared_ptr<Rule> getRule(const std::string& name);

      private:
        std::string m_name;

        std::map<std::string, boost::shared_ptr<Rule> > m_rule;        
        std::map<size_t, boost::shared_ptr<Relation> > m_relation;
    };


    /**
     * @brief The main dataset class
    */
    class Dataset {
      public:
        /**
         * @brief Constructor
         */
        Dataset();
        /**
         * @brief Destructor
         */
        ~Dataset();

        /**
         * @brief Add an Arrayset to the Dataset
         */
        void append( boost::shared_ptr<Arrayset> arrayset);
        /**
         * @brief Remove a Relationset with a given name from the Dataset
         */
        void remove( const size_t id) {
          std::map<size_t, boost::shared_ptr<Arrayset> >::iterator it =
            m_arrayset.find(id);
          if(it!=m_arrayset.end())
            m_arrayset.erase(it);
          else
            throw NonExistingElement();
        }

        /**
         * @brief Get the name of the Dataset
         */
        const std::string& getName() const { return m_name; }

        /**
         * @brief Set the name of the Dataset
         */
        void setName( const std::string& name) { m_name = name; }

        /**
         * @brief Get the version of the Dataset
         */
        const size_t getVersion() const { return m_version; }

        /**
         * @brief Set the version of the Dataset
         */
        void setVersion( const size_t version) { m_version = version; }

        /**
         * @brief const_iterator over the Arraysets of the Dataset
         */
        typedef std::map<size_t, boost::shared_ptr<Arrayset> >::const_iterator
          const_iterator;
        /**
         * @brief Return a const_iterator pointing at the first Arrayset of 
         * the Dataset
         */
        const_iterator begin() const { return m_arrayset.begin(); }
        /**
         * @brief Return a const_iterator pointing at the last Arrayset of 
         * the Dataset
         */
        const_iterator end() const { return m_arrayset.end(); }

        /**
         * @brief iterator over the Arraysets of the Dataset
         */
        typedef std::map<size_t, boost::shared_ptr<Arrayset> >::iterator 
          iterator;
        /**
         * @brief Return an iterator pointing at the first Arrayset of 
         * the Dataset
         */
        iterator begin() { return m_arrayset.begin(); }
        /**
         * @brief Return an iterator pointing at the first Arrayset of 
         * the Dataset
         */
        iterator end() { return m_arrayset.end(); }
   
        /**
         * @brief Return the Arrayset of the given id 
         * @warning Please note that if you use that method, scope matters,
         * because the dataset owns the arraysets.
         */
        const Arrayset& operator[]( const size_t id ) const;
        /**
         * @brief Return the arrayset of the given id
         */
        boost::shared_ptr<const Arrayset> getArrayset(const size_t id) const;
        boost::shared_ptr<Arrayset> getArrayset(const size_t id);

        /**
         * @brief Add a Relationset to the Dataset
         */
        void append( boost::shared_ptr<Relationset> relationset);
        /**
         * @brief Remove a Relationset with a given name from the Dataset
         */
        void remove( const std::string& name) {
          std::map<std::string, boost::shared_ptr<Relationset> >::iterator it=
            m_relationset.find(name);
          if(it!=m_relationset.end())
            m_relationset.erase(it);
          else
            throw NonExistingElement();
        }

        /**
         * @brief const_iterator over the Relationsets of the Dataset
         */
        typedef std::map<std::string, boost::shared_ptr<Relationset> >::
          const_iterator relationset_const_iterator;
        /**
         * @brief Return a const_iterator pointing at the first Relationset of
         * the Dataset
         */
        relationset_const_iterator relationset_begin() const { 
          return m_relationset.begin(); }
        /**
         * @brief Return a const_iterator pointing at the last Relationset of 
         * the Dataset
         */
        relationset_const_iterator relationset_end() const { 
          return m_relationset.end(); }

        /**
         * @brief iterator over the Relationsets of the Dataset
         */
        typedef std::map<std::string, boost::shared_ptr<Relationset> >::iterator
          relationset_iterator;
        /**
         * @brief Return an iterator pointing at the first Relationset of 
         * the Dataset
         */
        relationset_iterator relationset_begin() { return m_relationset.begin(); }
        /**
         * @brief Return an iterator pointing at the first Relationset of 
         * the Dataset
         */
        relationset_iterator relationset_end() { return m_relationset.end(); }
   
        /**
         * @brief Return the Relationset of the given name
         * @warning Please note that if you use that method, scope matters,
         * because the dataset owns the relationsets.
         */
        const Relationset& operator[]( const std::string& name ) const;

        /**
         * @brief Return the arrayset of the given id
         */
        boost::shared_ptr<const Relationset> 
        getRelationset( const std::string& name ) const;
        boost::shared_ptr<Relationset> 
        getRelationset( const std::string& name );

      private:
        std::string m_name;
        size_t m_version;

        std::map<size_t, boost::shared_ptr<Arrayset> > m_arrayset;
        std::map<std::string, boost::shared_ptr<Relationset> > m_relationset;
    };


    /********************** TEMPLATE FUNCTION DEFINITIONS ***************/
    template <typename T, typename U> 
    void Array::copyCast( U* out ) const {
      size_t n_elem = m_parent_arrayset.getNElem();
      for( size_t i=0; i<n_elem; ++i) {
        T* t_storage = static_cast<T*>(m_storage);
        static_complex_cast( t_storage[i], out[i] );
      }
    }

    template <typename T, int D> 
    void Array::copy( blitz::Array<T,D>& output) const 
    {
      if( D != m_parent_arrayset.getNDim() ) {
        TDEBUG3("D=" << D << " -- ParseXML: D=" << 
          m_parent_arrayset.getNDim());
        error << "Cannot copy the data in a blitz array with a different " <<
          "number of dimensions." << std::endl;
        throw NDimensionError();
      }

      // Reshape each dimensions with the correct size
      blitz::TinyVector<int,D> shape;
      m_parent_arrayset.getShape(shape);
      output.resize(shape);

      // Load the data if required
      if(!getIsLoaded()) {
        ;//TODO:load 
        Array* a=const_cast<Array*>(this);
        a->setIsLoaded(true);
      }

      T* out_data;
      if( output.isStorageContiguous() )
        out_data = output.data();
      else
        out_data = output.copy().data();
      switch(m_parent_arrayset.getElementType()) {
        case array::t_bool:
          copyCast<bool,T>(out_data); break;
        case array::t_int8:
          copyCast<int8_t,T>(out_data); break;
        case array::t_int16:
          copyCast<int16_t,T>(out_data); break;
        case array::t_int32:
          copyCast<int32_t,T>(out_data); break;
        case array::t_int64:
          copyCast<int64_t,T>(out_data); break;
        case array::t_uint8:
          copyCast<uint8_t,T>(out_data); break;
        case array::t_uint16:
          copyCast<uint16_t,T>(out_data); break;
        case array::t_uint32:
          copyCast<uint32_t,T>(out_data); break;
        case array::t_uint64:
          copyCast<uint64_t,T>(out_data); break;
        case array::t_float32:
          copyCast<float,T>(out_data); break;
        case array::t_float64:
          copyCast<double,T>(out_data); break;
        case array::t_float128:
          copyCast<long double,T>(out_data); break;
        case array::t_complex64:
          copyCast<std::complex<float>,T>(out_data); break;
        case array::t_complex128:
          copyCast<std::complex<double>,T>(out_data); break;
        case array::t_complex256:
          copyCast<std::complex<long double>,T>(out_data); break;
        default:
          break;
      }
    }

    template <typename T, int D> 
    void Array::referCheck( blitz::Array<T,D>& output) const
    {
      // Load the data if required
      if(!getIsLoaded()) {
        ;//TODO:load 
        Array* a=const_cast<Array*>(this);
        a->setIsLoaded(true);
      }

      if( D != m_parent_arrayset.getNDim() ) {
        TDEBUG3("D=" << D << " -- ParseXML: D=" <<
           m_parent_arrayset.getNDim());
        error << "Cannot refer to the data in a blitz array with a " <<
          "different number of dimensions." << std::endl;
        throw NDimensionError();
      }
    }

    template <typename T, int D> 
    void Array::refer( blitz::Array<T,D>& output)
    {
      error << "Unsupported blitz array type " << std::endl;
      throw TypeError();
    }

    template<int D> 
    void Array::refer( blitz::Array<bool,D>& output)
    {
      referCheck(output);

      // Reshape each dimensions with the correct size
      blitz::TinyVector<int,D> shape;
      m_parent_arrayset.getShape(shape);
      output.resize(shape);

      switch(m_parent_arrayset.getElementType()) {
        case array::t_bool:
          output = blitz::Array<bool,D>(static_cast<bool*>(m_storage), 
            shape, blitz::neverDeleteData); 
          break;
        default:
          error << "Cannot refer to data of type bool with a non-bool " <<
            "blitz array." << std::endl;
          throw TypeError();
          break;
      }
    }

    template<int D> 
    void Array::refer( blitz::Array<int8_t,D>& output)
    {
      referCheck(output);

      // Reshape each dimensions with the correct size
      blitz::TinyVector<int,D> shape;
      m_parent_arrayset.getShape(shape);
      output.resize(shape);

      switch(m_parent_arrayset.getElementType()) {
        case array::t_int8:
          output = blitz::Array<int8_t,D>(static_cast<int8_t*>(m_storage), 
            shape, blitz::neverDeleteData); 
          break;
        default:
          error << "Cannot refer to data of type int8_t with a non-int8_t " <<
            "blitz array." << std::endl;
          throw TypeError();
          break;
      }
    }

    template<int D> 
    void Array::refer( blitz::Array<int16_t,D>& output)
    {
      referCheck(output);

      // Reshape each dimensions with the correct size
      blitz::TinyVector<int,D> shape;
      m_parent_arrayset.getShape(shape);
      output.resize(shape);

      switch(m_parent_arrayset.getElementType()) {
        case array::t_int16:
          output = blitz::Array<int16_t,D>(static_cast<int16_t*>(m_storage), 
            shape, blitz::neverDeleteData); 
          break;
        default:
          error << "Cannot refer to data of type int16_t with a non-int16_t " <<
            "blitz array." << std::endl;
          throw TypeError();
          break;
      }
    }

    template<int D> 
    void Array::refer( blitz::Array<int32_t,D>& output)
    {
      referCheck(output);

      // Reshape each dimensions with the correct size
      blitz::TinyVector<int,D> shape;
      m_parent_arrayset.getShape(shape);
      output.resize(shape);

      switch(m_parent_arrayset.getElementType()) {
        case array::t_int32:
          output = blitz::Array<int32_t,D>(static_cast<int32_t*>(m_storage), 
            shape, blitz::neverDeleteData); 
          break;
        default:
          error << "Cannot refer to data of type int32_t with a non-int32_t " <<
            "blitz array." << std::endl;
          throw TypeError();
          break;
      }
    }

    template<int D> 
    void Array::refer( blitz::Array<int64_t,D>& output)
    {
      referCheck(output);

      // Reshape each dimensions with the correct size
      blitz::TinyVector<int,D> shape;
      m_parent_arrayset.getShape(shape);
      output.resize(shape);

      switch(m_parent_arrayset.getElementType()) {
        case array::t_int64:
          output = blitz::Array<int64_t,D>(static_cast<int64_t*>(m_storage), 
            shape, blitz::neverDeleteData); 
          break;
        default:
          error << "Cannot refer to data of type int64_t with a non-int64_t " <<
            "blitz array." << std::endl;
          throw TypeError();
          break;
      }
    }

    template<int D> 
    void Array::refer( blitz::Array<uint8_t,D>& output)
    {
      referCheck(output);

      // Reshape each dimensions with the correct size
      blitz::TinyVector<int,D> shape;
      m_parent_arrayset.getShape(shape);
      output.resize(shape);

      switch(m_parent_arrayset.getElementType()) {
        case array::t_uint8:
          output = blitz::Array<uint8_t,D>(static_cast<uint8_t*>(m_storage), 
            shape, blitz::neverDeleteData); 
          break;
        default:
          error << "Cannot refer to data of type uint8_t with a non-uint8_t " <<
            "blitz array." << std::endl;
          throw TypeError();
          break;
      }
    }

    template<int D> 
    void Array::refer( blitz::Array<uint16_t,D>& output)
    {
      referCheck(output);

      // Reshape each dimensions with the correct size
      blitz::TinyVector<int,D> shape;
      m_parent_arrayset.getShape(shape);
      output.resize(shape);

      switch(m_parent_arrayset.getElementType()) {
        case array::t_uint16:
          output = blitz::Array<uint16_t,D>(static_cast<uint16_t*>(m_storage), 
            shape, blitz::neverDeleteData); 
          break;
        default:
          error << "Cannot refer to data of type uint16_t with a non-uint16_t " <<
            "blitz array." << std::endl;
          throw TypeError();
          break;
      }
    }

    template<int D> 
    void Array::refer( blitz::Array<uint32_t,D>& output)
    {
      referCheck(output);

      // Reshape each dimensions with the correct size
      blitz::TinyVector<int,D> shape;
      m_parent_arrayset.getShape(shape);
      output.resize(shape);

      switch(m_parent_arrayset.getElementType()) {
        case array::t_uint32:
          output = blitz::Array<uint32_t,D>(static_cast<uint32_t*>(m_storage), 
            shape, blitz::neverDeleteData); 
          break;
        default:
          error << "Cannot refer to data of type uint32_t with a non-uint32_t " <<
            "blitz array." << std::endl;
          throw TypeError();
          break;
      }
    }

    template<int D> 
    void Array::refer( blitz::Array<uint64_t,D>& output)
    {
      referCheck(output);

      // Reshape each dimensions with the correct size
      blitz::TinyVector<int,D> shape;
      m_parent_arrayset.getShape(shape);
      output.resize(shape);

      switch(m_parent_arrayset.getElementType()) {
        case array::t_uint64:
          output = blitz::Array<uint64_t,D>(static_cast<uint64_t*>(m_storage), 
            shape, blitz::neverDeleteData); 
          break;
        default:
          error << "Cannot refer to data of type uint64_t with a non-uint64_t " <<
            "blitz array." << std::endl;
          throw TypeError();
          break;
      }
    }

    template<int D> 
    void Array::refer( blitz::Array<float,D>& output)
    {
      referCheck(output);

      // Reshape each dimensions with the correct size
      blitz::TinyVector<int,D> shape;
      m_parent_arrayset.getShape(shape);
      output.resize(shape);

      switch(m_parent_arrayset.getElementType()) {
        case array::t_float32:
          output = blitz::Array<float,D>(static_cast<float*>(m_storage),
            shape, blitz::neverDeleteData); 
          break;
        default:
          error << "Cannot refer to data of type float with a non-float " <<
            "blitz array." << std::endl;
          throw TypeError();
          break;
      }
    }

    template<int D> 
    void Array::refer( blitz::Array<double,D>& output)
    {
      referCheck(output);

      // Reshape each dimensions with the correct size
      blitz::TinyVector<int,D> shape;
      m_parent_arrayset.getShape(shape);
      output.resize(shape);

      switch(m_parent_arrayset.getElementType()) {
        case array::t_float64:
          output = blitz::Array<double,D>(static_cast<double*>
            (m_storage), shape, blitz::neverDeleteData); 
          break;
        default:
          error << "Cannot refer to data of type double with a non-double " <<
            "blitz array." << std::endl;
          throw TypeError();
          break;
      }
    }

    template<int D> 
    void Array::refer( blitz::Array<long double,D>& output)
    {
      referCheck(output);

      // Reshape each dimensions with the correct size
      blitz::TinyVector<int,D> shape;
      m_parent_arrayset.getShape(shape);
      output.resize(shape);

      switch(m_parent_arrayset.getElementType()) {
        case array::t_float128:
          output = blitz::Array<long double,D>(static_cast<long double*>
            (m_storage), shape, blitz::neverDeleteData); 
          break;
        default:
          error << "Cannot refer to data of type long double with a " <<
            "non-long double blitz array." << std::endl;
          throw TypeError();
          break;
      }
    }

    template<int D> 
    void Array::refer( blitz::Array<std::complex<float>,D>& output)
    {
      referCheck(output);

      // Reshape each dimensions with the correct size
      blitz::TinyVector<int,D> shape;
      m_parent_arrayset.getShape(shape);
      output.resize(shape);

      switch(m_parent_arrayset.getElementType()) {
        case array::t_complex64:
          output = blitz::Array<std::complex<float>,D>(
            static_cast<std::complex<float>*>(m_storage), 
            shape, blitz::neverDeleteData);
          break;
        default:
          error << "Cannot refer to data of type complex(float) with a " <<
            "non-complex(float) blitz array." << std::endl;
          throw TypeError();
          break;
      }
    }

    template<int D> 
    void Array::refer( blitz::Array<std::complex<double>,D>& output)
    {
      referCheck(output);

      // Reshape each dimensions with the correct size
      blitz::TinyVector<int,D> shape;
      m_parent_arrayset.getShape(shape);
      output.resize(shape);

      switch(m_parent_arrayset.getElementType()) {
        case array::t_complex128:
          output = blitz::Array<std::complex<double>,D>(
            static_cast<std::complex<double>*>(m_storage), 
            shape, blitz::neverDeleteData);
          break;
        default:
          error << "Cannot refer to data of type complex(double) with a " <<
            "non-complex(double) blitz array." << std::endl;
          throw TypeError();
          break;
      }
    }

    template<int D> 
    void 
    Array::refer( blitz::Array<std::complex<long double>,D>& output)
    {
      referCheck(output);

      // Reshape each dimensions with the correct size
      blitz::TinyVector<int,D> shape;
      m_parent_arrayset.getShape(shape);
      output.resize(shape);

      switch(m_parent_arrayset.getElementType()) {
        case array::t_complex256:
          output = blitz::Array<std::complex<long double>,D>(
            static_cast<std::complex<long double>*>(m_storage), 
            shape, blitz::neverDeleteData); 
          break;
        default:
          error << "Cannot refer to data of type complex(long double) with" <<
            "a non-complex(long double) blitz array." << std::endl;
          throw TypeError();
          break;
      }
    }


    template<typename T, int D> void 
    Arrayset::at(size_t id, blitz::Array<T,D>& output) const {
      boost::shared_ptr<Array> x = (m_array.find(id))->second;
      x->copy(output);
    }


  }
  /**
   * @}
   */
}

#endif /* TORCH5SPRO_CORE_DATASET_H */

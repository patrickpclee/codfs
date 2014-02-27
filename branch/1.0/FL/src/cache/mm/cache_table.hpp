/*
 *
 *  $Id: cache_table.hpp 18 2007-01-21 15:49:02Z matteo.merli $
 *
 *  $URL: https://cache-table.googlecode.com/svn/tags/0.2/mm/cache_table.hpp $
 *
 *  Copyright (C) 2006 Matteo Merli <matteo.merli@gmail.com>
 *
 *
 *  BSD License
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *   o Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   o Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the
 *     distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef _CACHE_TABLE_HPP_
#define _CACHE_TABLE_HPP_

#include <cassert>
#include <cstddef>
#include <cstring>
#include <memory>
#include <utility>

/// Default number of buckets, when it's not specified.
#define MM_DEFAULT_TABLE_SIZE 4096


namespace mm
{

using std::pair;
using std::_Construct;
using std::_Destroy;

template < class Value, class Key, class DiscardFunction,
           class HashFunction, class KeyEqual, class KeyExtract,
           class Allocator
         > class cache_table;

// ITERATORS

template <class V, class K, class DF, class HF, class KEq, class KEx, class A>
class cache_table_const_iterator;

/**
 * Computes the "distance" between two iterators, as the number of
 * steps needed to reach the last one starting from first.
 *
 * @param first iterator to begin
 * @param last  iterator to end
 * @return the number of steps needed to go from @a first to @a last
 */
template <typename IteratorA, typename IteratorB>
inline ptrdiff_t distance( IteratorA first, IteratorB last )
{
    ptrdiff_t n = 0;
    while ( first != last )
    {
        ++first;
        ++n;
    }

    return n;
}

////////////////////////////////////////////////////////////////////////

/**
 * Iterator class
 */
template <class V, class K, class DF, class HF, class KEq, class KEx, class A>
class cache_table_iterator
{
public:

    /// cache_table definition
    typedef cache_table<V,K,DF,HF,KEq,KEx,A>                table;

    /// Iterator definition
    typedef cache_table_iterator<V,K,DF,HF,KEq,KEx,A>       iterator;

    /// Const iterator definition
    typedef cache_table_const_iterator<V,K,DF,HF,KEq,KEx,A> const_iterator;

    /// Iterator category
    typedef std::random_access_iterator_tag iterator_category;

    /// A signed integral type.
    typedef ptrdiff_t                       difference_type;

    /// Reference to value_type.
    typedef V&                              reference;

    /// Pointer to value_type.
    typedef V*                              pointer;

    /// The key type.
    typedef K                               key_type;

    /// The value type.
    typedef V                               value_type;

    /// An unsigned integral type.
    typedef typename table::size_type       size_type;

private:

    /** Constructor.
     *  Creates an iterator that points to a specified item.
     *
     *  @param table pointer to the cache_table instance
     *  @param it pointer to an item in the table
     *  @param advance_to_next if true, the iterator will
     *         advance until it reach a valid (non-empty)
     *         item.
     */
    cache_table_iterator( const table* table, pointer it )
        : m_ht( table ),
          m_pos( it )
    {}

    /** Constructor.
     *  Creates an iterator that points to a specified item.
     *
     *  @param table pointer to the cache_table instance
     *  @param it pointer to an item in the table
     *  @param advance_to_next if true, the iterator will
     *         advance until it reach a valid (non-empty)
     *         item.
     */
    cache_table_iterator( const table* table, pointer it,
                          bool advance_to_next )
        : m_ht( table ),
          m_pos( it )
    {
        if ( advance_to_next )
            advance_to_next_item();
    }

public:

    /** Default Constructor.
     *  Construct an iterator that points to no item.
     */
    cache_table_iterator()
        : m_ht( 0 ),
          m_pos( 0 )
    {}

    /** Copy constructor.
     *
     * @param it the iterator to be copied.
     */
    cache_table_iterator( const cache_table_iterator& it )
        : m_ht(  it.m_ht  ),
          m_pos( it.m_pos )
    {}

    /** Dereferencing operator.
     *
     *  @return a reference to the item pointed by the iterator.
     */
    reference operator*() const { return *m_pos; }

    /** Dereferencing operator.
     *
     *  @return a pointer to the item pointed by the iterator.
     */
    pointer operator->()  const { return &(operator*()); }

    /** Advance the iterator to next item.
     *
     *  @warning Note that no bound check is performed, you should not pass
     *           the @p end() limit.
     */
    iterator& operator++()
    {
        ++m_pos;
        advance_to_next_item();
        return *this;
    }

    /** Advance the iterator to next item.
     *
     *  @warning Note that no bound check is performed, you should not pass
     *           the @p end() limit.
     */
    iterator operator++(int) { iterator tmp(*this); ++*this; return tmp; }

    /** Equality operator.
     *
     *  @param it another iterator
     *  @return true if the two iterators points to the same item.
     */
    bool operator==( const iterator& it ) const { return m_pos == it.m_pos; }

    /** Dis-equality operator.
     *
     *  @param it another iterator
     *  @return false if the two iterators points to the same item.
     */
    bool operator!=( const iterator& it ) const { return m_pos != it.m_pos; }

private:

    /// Advance the iterator to the next non-empty item
    void advance_to_next_item()
    {
        while ( m_pos < m_ht->m_end_marker && m_ht->is_empty_key( m_pos ) )
            ++m_pos;
    }

    /// Pointer to tha associated cache-table instance
    const table*  m_ht;
    /// Pointer to an item in the cache-table
    pointer       m_pos;

    /// Const iterator definition
    friend class cache_table_const_iterator<V,K,DF,HF,KEq,KEx,A>;

    /// Cache table definition
    friend class cache_table<V,K,DF,HF,KEq,KEx,A>;
};

////////////////////////////////////////////////////////////////////////

/**
 * Const Iterator class
 */
template <class V, class K, class DF, class HF, class KEq, class KEx, class A>
class cache_table_const_iterator
{
public:

    /// cache_table definition
    typedef cache_table<V,K,DF,HF,KEq,KEx,A>                table;

    /// Iterator definition
    typedef cache_table_iterator<V,K,DF,HF,KEq,KEx,A>       iterator;

    /// Const iterator definition
    typedef cache_table_const_iterator<V,K,DF,HF,KEq,KEx,A> const_iterator;

    /// Iterator category
    typedef std::random_access_iterator_tag iterator_category;

    /// A signed integral type.
    typedef ptrdiff_t                       difference_type;

    /// Const Reference to value_type.
    typedef const V&                        reference;

    /// Const Pointer to value_type.
    typedef const V*                        pointer;

    /// The key type.
    typedef K                               key_type;

    /// The value type.
    typedef V                               value_type;

    /// An unsigned integral type.
    typedef typename table::size_type       size_type;

private:

    /** Constructor.
     *  Creates an iterator that points to a specified item.
     *
     *  @param table pointer to the cache_table instance
     *  @param it pointer to an item in the table
     */
    cache_table_const_iterator( const table* table, pointer it )
        : m_ht( table ),
          m_pos( it )
    {}

    /** Constructor.
     *  Creates an iterator that points to a specified item.
     *
     *  @param table pointer to the cache_table instance
     *  @param it pointer to an item in the table
     *  @param advance_to_next if true, the iterator will
     *         advance until it reach a valid (non-empty)
     *         item.
     */
    cache_table_const_iterator( const table* table, pointer it,
                                bool advance_to_next )
        : m_ht( table ),
          m_pos( it )
    {
        if ( advance_to_next )
            advance_to_next_item();
    }

public:
    /** Default Constructor.
     *  Construct an iterator that points to no item.
     */
    cache_table_const_iterator()
        : m_ht( 0 ), m_pos( 0 )
    {}

    /** Copy constructor.
     *
     * @param it the iterator to be copied.
     */
    cache_table_const_iterator( const iterator& it )
        : m_ht(  it.m_ht  ),
          m_pos( it.m_pos )
    {}

    /** Dereferencing operator.
     *
     *    @return a reference to the item pointed by the iterator.
     */
    reference operator*() const { return *m_pos; }

    /** Dereferencing operator.
     *
     *  @return a pointer to the item pointed by the iterator.
     */
    pointer operator->()  const { return &(operator*()); }

    /** Advance the iterator to next item.
     *
     *  @warning Note that no bound check is performed, you should not pass
     *           the @p end() limit.
     */
    const_iterator& operator++()
    {
        ++m_pos;
        advance_to_next_item();
        return *this;
    }

    /** Advance the iterator to next item.
     *
     *  @warning Note that no bound check is performed, you should not pass
     *           the @p end() limit.
     */
    const_iterator operator++(int)
    { const_iterator tmp(*this); ++*this; return tmp; }

    /** Equality operator.
     *
     *  @param it another iterator
     *  @return true if the two iterators points to the same item.
     */
    bool operator==( const const_iterator& it ) const
    { return m_pos == it.m_pos; }

    /** Dis-equality operator.
     *
     *  @param it another iterator
     *  @return false if the two iterators points to the same item.
     */
    bool operator!=( const const_iterator& it ) const
    { return m_pos != it.m_pos; }

private:

    void advance_to_next_item()
    {
        while ( m_pos < m_ht->m_end_marker && m_ht->is_empty_key( m_pos ) )
            ++m_pos;
    }

    /// Pointer to tha associated cache-table instance
    const table*  m_ht;
    /// Pointer to an item in the cache-table
    pointer       m_pos;

    /// Cache table definition
    friend class cache_table<V,K,DF,HF,KEq,KEx,A>;
};


////////////////////////////////////////////////////////////////////////

/**
 * CacheTable: back-end data structure class.
 *
 * This class should not be used directly. Instead use one of CacheMap
 * or CacheSet.
 *
 * @author Matteo Merli
 * @date $Date: 2007-01-21 16:49:02 +0100 (Sun, 21 Jan 2007) $
 */
template < class Value,
           class Key,
           class DiscardFunction,
           class HashFunction,
           class KeyEqual,
           class KeyExtract,
           class Allocator
         >
class cache_table
{
public:
    typedef Key             key_type;
    typedef Value           value_type;
    typedef HashFunction    hasher;
    typedef KeyEqual        key_equal;
    typedef KeyExtract      key_extract;

    typedef size_t            size_type;
    typedef ptrdiff_t         difference_type;
    typedef value_type*       pointer;
    typedef const value_type* const_pointer;
    typedef value_type&       reference;
    typedef const value_type& const_reference;

    typedef cache_table_iterator< Value, Key, DiscardFunction, HashFunction,
                                  KeyEqual, KeyExtract, Allocator
                                > iterator;
    typedef cache_table_const_iterator< Value, Key, DiscardFunction, HashFunction,
                                        KeyEqual, KeyExtract, Allocator
                                      > const_iterator;

    typedef Allocator allocator_type;

private:
    HashFunction      m_hasher;
    KeyEqual          m_key_equal;
    KeyExtract        m_key_extract;
    Allocator         m_allocator;
    DiscardFunction   m_discard;

public:

    hasher hash_funct() const { return m_hasher;    }
    key_equal key_eq()  const { return m_key_equal; }

public:

    cache_table()
        : m_num_elements( 0 ),
          m_num_collisions( 0 ),
          m_empty_key_is_set( false ),
          m_table( 0 ),
          m_empty_key(),
          m_empty_value()
    {
        m_buckets = round_to_power2( MM_DEFAULT_TABLE_SIZE );
        m_mask = m_buckets - 1;

        init();
    }

    cache_table( size_type n, const hasher& hash, const key_equal& ke )
        : m_hasher( hash ),
          m_key_equal( ke ),
          m_num_elements( 0 ),
          m_num_collisions( 0 ),
          m_empty_key_is_set( false ),
          m_table( 0 ),
          m_empty_key(),
          m_empty_value()
    {
        m_buckets = round_to_power2( n );
        m_mask = m_buckets - 1;

        init();
    }

    /** Copy constructor
     *
     *  @param other the cache_table to be copied
     */
    cache_table( const cache_table& other )
        : m_hasher( other.m_hasher ),
          m_key_equal( other.m_key_equal ),
          m_num_elements( other.m_num_elements ),
          m_num_collisions( other.m_num_collisions ),
          m_empty_key_is_set( other.m_empty_key_is_set ),
          m_table( other.m_table ),
          m_empty_key( other.m_empty_key),
          m_empty_value( other.m_empty_value ),
          m_buckets( other.m_buckets ),
          m_mask( other.m_mask ),
          m_end_it( other.m_end_it )
    {
        init();
        insert( other.begin(), other.end() );
    }

    /** The assignment operator
     *
     *  @param other the cache_table to be copied
     *  @return a reference to a new cache_table object
     */
    cache_table& operator= ( const cache_table& other )
    {
        if ( &other != this )
        {
            cache_table new_table( other );
            swap( *this, other );
        }

        return *this;
    }

private:
    void init()
    {
        m_table = m_allocator.allocate( m_buckets * ItemSize );
        m_end_marker = m_table + m_buckets;
        m_end_it = iterator( this, m_end_marker );

        initialize_memory();
    }

public:
    void set_empty_value( const value_type& empty_value )
    {
        assert( m_empty_key_is_set == false );

        m_empty_key = m_key_extract( empty_value );
        m_empty_value = empty_value;

        initialize_memory();
        m_empty_key_is_set = true;
    }

    const value_type& get_empty_value() const
    {
        return m_empty_value;
    }

    ~cache_table()
    {
        clear();
        m_allocator.deallocate( m_table, m_buckets * ItemSize );
    }

    // INSERTIONS

    pair<iterator,bool> insert( const value_type& obj )
    {
        const key_type& obj_key = m_key_extract( obj );
        const size_t buck = m_hasher( obj_key ) & m_mask;

        const key_type& table_key = m_key_extract( m_table[ buck ] );

        if ( ! m_key_equal( table_key, m_empty_key ) )
        {
            // There's already an item in the bucket and its key it different
            // from the key of the inserted item.  Element is discarded.
            ++m_num_collisions;

            // Notify that the item will be discarded, to allow a policy to
            // do something useful with it.
            m_discard( m_table[ buck ], obj );
            _Destroy( m_table + buck );
            reset_value( m_table + buck );
        }
        else
            ++m_num_elements;

        // Copy the object into the hash table.
        _Construct( m_table + buck, obj );
        return pair<iterator,bool>( iterator( this, m_table + buck ), true );
    }

    template <class InputIterator>
    void insert( InputIterator first, InputIterator last )
    {
        for ( ; first != last; ++first )
            insert( *first );
    }

    // SEARCHES

    iterator find( const key_type& key )
    {
        // First of all, obtain the bucket number corresponding with the
        // supplied key
        size_t buck = m_hasher( key ) & m_mask;

        // Next, check that the key found in table bucket is equal to the
        // supplied key. If not, either the bucket is empty and so the key
        // wasn't found, or the bucket is hosting a different value with a
        // hash collision.
        if ( ! m_key_equal( m_key_extract( m_table[ buck ] ), key ) )
            return m_end_it;

        // else return the iterator to found item
        return iterator( this, m_table + buck );
    }

    const_iterator find( const key_type& key ) const
    {
        // First of all, obtain the bucket number corresponding with the
        // supplied key
        size_t buck = m_hasher( key ) & m_mask;

        // Next, check that the key found in table bucket is equal to the
        // supplied key. If not, either the bucket is empty and so the key
        // wasn't found, or the bucket is hosting a different value with a
        // hash collision.
        if ( ! m_key_equal( m_key_extract( m_table[ buck ] ), key ) )
            return m_end_it;

        // else return the iterator to found item
        return iterator( this, m_table + buck );
    }

    value_type& find_or_insert( const key_type& key )
    {
        size_t buck = m_hasher( key ) & m_mask;
        key_type& table_key = m_key_extract( m_table[ buck ] );

        if ( ! m_key_equal( key, table_key ) )
        {
            // The bucket is either empty or it contains another item.  In
            // this case we have to mark it as discarded, because we cannot
            // know how the reference will be used ( probably to overwrite
            // the item with a new one that has a different key ).
            if ( ! m_key_equal( table_key, m_empty_key ) )
            {
                // The bucket already contained an item. This item is
                // discarded and replaced with an empty one. The destructor
                // is called on it.
                // The m_num_elements does not change because
                m_discard( m_table[ buck ], m_empty_value );
                _Destroy( m_table + buck );
                reset_value( m_table + buck );
            }
            else
            {
                // The bucket was empty, so we have to increment the items
                // counter
                ++m_num_elements;
            }

            // Set the key in the empty item
            _Construct( &table_key, key );
        }

        // Returns the reference to the found or recently added item
        return m_table[ buck ];
    }

    size_type erase( const key_type& key )
    {
        iterator it = find( key );
        if ( it != m_end_it )
        {
            erase( it );
            return 1;
        }

        return 0;
    }

    void erase( const iterator& it )
    {
        if (    it != m_end_it
             && ! m_key_equal( m_key_extract( *it ), m_empty_key ) )
        {
            _Destroy( &* it );
            reset_value( it.m_pos );
            --m_num_elements;
        }
    }

    void erase( const const_iterator& it )
    {
        erase( iterator( const_cast<cache_table*>( it.m_ht ),
                         const_cast<pointer>( it.m_pos ) )
             );
    }

    void erase( iterator first, iterator last )
    {
        m_num_elements -= mm::distance( first, last );
        _Destroy( first, last );
        std::uninitialized_fill( first, last, m_empty_value );
    }

    void erase( const_iterator first, const_iterator last )
    {
        erase( iterator( const_cast<cache_table*>( first.m_ht ),
                         const_cast<pointer>( first.m_pos ) ),
               iterator( const_cast<cache_table*>( last.m_ht ),
                         const_cast<pointer>( last.m_pos ) )
             );
    }

    void resize( size_type size )
    {
        size_t new_size = round_to_power2( size );
        size_t old_size = m_buckets;

        if ( new_size == old_size )
        {
            // Do nothing
            return;
        }
        else if ( new_size < old_size )
        {
            // The new table will be smaller, so there's no need to rehash
            // all the items.
            value_type* new_table;
            new_table = m_allocator.allocate( new_size * ItemSize );

            // Copy the elements that fit into the new table and destroy
            // those that doesn't fit.  Plain old memcpy seems to have much
            // less problems with types than std::copy..
            std::memcpy( new_table, m_table, new_size * ItemSize );
            _Destroy( iterator( this, m_table + new_size, true ), m_end_it );

            m_allocator.deallocate( m_table, old_size );
            m_table = new_table;

            m_end_marker = m_table + new_size;
            m_end_it = iterator( this, m_end_marker );
            m_buckets = new_size;
            m_mask = m_buckets - 1;

            // Re-count the number of elements.
            m_num_elements = 0;
            for ( const_iterator it = begin(); it != m_end_it; ++it )
                ++m_num_elements;
        }
        else // new_size > old_size
        {
            // Creates a new table and re-insert all the items, with
            // new buckets.
            cache_table other( new_size, m_hasher, m_key_equal );
            other.set_empty_value( m_empty_value );
            swap( other );
            insert( other.begin(), other.end() );
        }
    }

    void clear()
    {
        // Call the destructor for all objects and reinitialize the memory.
        _Destroy( begin(), end() );
        initialize_memory();
        m_num_elements = 0;
    }

    // Iterator functions
    iterator begin()             { return iterator( this, m_table, true ); }
    iterator end()               { return iterator( this, m_end_marker ); }
    const_iterator begin() const { return const_iterator( this, m_table, true ); }
    const_iterator end()   const { return const_iterator( this, m_end_marker ); }

    size_type size()         const { return m_num_elements; }
    size_type max_size()     const { return m_buckets; }
    size_type bucket_count() const { return m_buckets; }
    bool empty()             const { return size() == 0; }

    size_type num_collisions() const { return m_num_collisions; }

    // Comparison
    bool operator==( const cache_table& other ) const
    {
        return (    m_table == other.m_table
                 && m_num_elements == m_num_elements );
    }

    bool operator!=( const cache_table& other ) const
    { return !( *this == other ); }

    void swap( cache_table& other)
    {
        // Swap all the fields
        std::swap( m_hasher,           other.m_hasher           );
        std::swap( m_key_equal,        other.m_key_equal        );
        std::swap( m_key_extract,      other.m_key_extract      );
        std::swap( m_allocator,        other.m_allocator        );
        std::swap( m_mask,             other.m_mask             );
        std::swap( m_buckets,          other.m_buckets          );
        std::swap( m_num_elements,     other.m_num_elements     );
        std::swap( m_num_collisions,   other.m_num_collisions   );
        std::swap( m_empty_key_is_set, other.m_empty_key_is_set );
        std::swap( m_table,            other.m_table            );
        std::swap( m_empty_key,        other.m_empty_key        );
        std::swap( m_empty_value,      other.m_empty_value      );
        std::swap( m_end_marker,       other.m_end_marker       );
        std::swap( m_end_it,           other.m_end_it           );
    }

private:

    /// Array item size
    static const size_t ItemSize = sizeof(value_type);

    /// Initialize the whole hash table with the empty value
    void initialize_memory()
    {
        std::uninitialized_fill( m_table,
                                 m_table + m_buckets,
                                 m_empty_value );
    }

    void reset_value( pointer pos )
    {
        std::memcpy( pos, &m_empty_value, ItemSize );
    }

    /// Compares the key with the empty key
    bool is_empty_key( const_pointer& pos ) const
    {
        return m_key_equal( m_key_extract( *pos ), m_empty_key );
    }

    bool is_empty_key( pointer& pos ) const
    {
        return m_key_equal( m_key_extract( *pos ), m_empty_key );
    }

    /// Round the number to the next power of 2.
    static size_t round_to_power2( size_t n )
    {
        size_t x = 1, i = 0;
        while ( (i++ < 64) &&  (x < n) )
            x <<= 1;

        return x;
    }

    friend class cache_table_iterator< Value, Key, DiscardFunction,
                                       HashFunction, KeyEqual, KeyExtract,
                                       Allocator >;
    friend class cache_table_const_iterator< Value, Key, DiscardFunction,
                                             HashFunction, KeyEqual,
                                             KeyExtract, Allocator >;

    // Internal data

    size_t m_buckets;          ///< Number of buckets in the hash table
    size_t m_mask;             ///< Mask used to calculate bucket
    size_t m_num_elements;     ///< Number of elements in the table
    size_t m_num_collisions;   ///< Number of collisions
    bool   m_empty_key_is_set; ///< Tells whether the empty key has been set

    value_type* m_table;       ///< The 'real' hash table array
    key_type    m_empty_key;   ///< The key value that identifies empty items
    value_type  m_empty_value; ///< The value that identifies empty items
    value_type* m_end_marker;  ///< Pointer to the end of the table
    iterator    m_end_it;      ///< value of end()
};

/**
 * @relates cache_table
 */
template <class V, class K, class DF, class HF, class KEq, class KEx, class A>
inline void swap( cache_table<V,K,DF,HF,KEq,KEx,A>& ht1,
                  cache_table<V,K,DF,HF,KEq,KEx,A>& ht2 )
{
    ht1.swap( ht2 );
}

} // namespace mm

#endif // _CACHE_TABLE_HPP_

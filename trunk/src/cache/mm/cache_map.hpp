/*
 *
 *  $Id: cache_map.hpp 18 2007-01-21 15:49:02Z matteo.merli $
 *
 *  $URL: https://cache-table.googlecode.com/svn/tags/0.2/mm/cache_map.hpp $
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

#ifndef _CACHE_MAP_HPP_
#define _CACHE_MAP_HPP_

#include "cache_table.hpp"
#include "hash_fun.hpp"

#include <utility>

/**
 * Namespace description.
 */
namespace mm
{

using std::pair;
using std::_Select1st;

/** Default "discarding" policy.
 *  Ignores discarded item. You can write another policy to do something
 *  useful with this item.
 */
template < class V >
struct DiscardIgnore
{
    /** This operator is called when the map need to discard an
     * item due to a key collision.
     *
     * @param old_value a reference to the element discarded
     * @param new_value a reference to the element that will enter
     *                  in the map
     *
     */
    void operator() ( const V& old_value, const V& new_value )
    {}
};

/** Cache map
 *
 * Implements a @a map container like std::hash_map, but with a
 * fixed element number.
 *
 *  <b>Template Parameters</b>
 *
 *  - @a Key : The key type of the map.
 *
 *  - @a T   : The hash_map's data type. This is also defined as
 *     hash_map::data_type.
 *
 *  - @a HashFunction : Callable hasher.
 *
 *  - @a DiscardFunction : Discarded item manager.
 *
 *  @author Matteo Merli
 *  @date $Date: 2007-01-21 16:49:02 +0100 (Sun, 21 Jan 2007) $
 */
template < class Key,
           class T,
           class HashFunction = hash<Key>,
           class KeyEqual = std::equal_to<Key>,
           class DiscardFunction = DiscardIgnore< pair<Key,T> >,
           class Allocator = std::allocator< pair<Key,T> >
>
class cache_map
{
private:
    /// The actual hash table
    typedef cache_table< pair<Key,T>, Key,
                         DiscardFunction,
                         HashFunction, KeyEqual,
                         _Select1st< pair<Key,T> >,
                         Allocator
                       > HT;
    HT m_ht;

public:

    /// The cache_map's key type, Key.
    typedef typename HT::key_type key_type;

    /// The type of object associated with the keys.
    typedef T data_type;

    /// The type of object associated with the keys.
    typedef T mapped_type;

    /// The type of object, pair<const key_type, data_type>, stored in the
    /// hash_map.
    typedef typename HT::value_type value_type;

    /// The cache_map's hash function.
    typedef typename HT::hasher hasher;

    /// Function object that compares keys for equality.
    typedef typename HT::key_equal key_equal;

    /// An unsigned integral type.
    typedef typename HT::size_type size_type;

    /// A signed integral type.
    typedef typename HT::difference_type difference_type;

    /// Pointer to value_type.
    typedef typename HT::pointer pointer;

    /// Const pointer to value_type.
    typedef typename HT::const_pointer const_pointer;

    /// Reference to value_type.
    typedef typename HT::reference reference;

    /// Const reference to value_type.
    typedef typename HT::const_reference const_reference;

    /// Iterator used to iterate through a cache_map.
    typedef typename HT::iterator iterator;

    /// Const iterator used to iterate through a cache_map.
    typedef typename HT::const_iterator const_iterator;

public:
    /** Returns the hasher object used by the hash_map.
     */
    hasher hash_funct() const { return m_ht.hash_funct(); }

    /** Returns the key_equal object used by the hash_map.
     */
    key_equal key_eq() const  { return m_ht.key_eq(); }

    /// Get an iterator to first item
    iterator begin()             { return m_ht.begin(); }
    /// Get an iterator to the end of the table
    iterator end()               { return m_ht.end();   }
    /// Get a const iterator to first item
    const_iterator begin() const { return m_ht.begin(); }
    /// Get a const iterator to the end of the table
    const_iterator end()   const { return m_ht.end();   }

public:

    /** Constructor. Construct an empty map with the default
     * number of buckets.
     */
    cache_map() : m_ht() {}

    /** Constructor.  You have to specify the size of the underlying table,
     *  in terms of the maximum number of allowed elements.
     *
     *  @attention After the cache_map is constructed, you have to call the
     *  set_empty_key() method to set the value of an unused key.
     *
     *  @param n The (fixed) size of table.
     *  @see set_empty_key
     */
    cache_map( size_type n )
        : m_ht( n, hasher(), key_equal() )
    {}

    /** Constructor.
     *
     * @param n the number of item buckets to allocate.
     * @param hash the hasher function
     */
    cache_map( size_type n, const hasher& hash )
        : m_ht( n, hash, key_equal() )
    {}

    /** Constructor.
     *
     * @param n    the number of item buckets to allocate
     * @param hash the hasher function
     * @param ke   the key comparison function
     */
    cache_map( size_type n,
               const hasher& hash,
               const key_equal& ke  )
        : m_ht( n, hash, ke )
    {}

    /** Creates a cache_map with a copy of a range.
     *
     * @param first iterator pointing to the first item
     * @param last  iterator pointing to the last item
     */
    template <class InputIterator>
    cache_map( InputIterator first, InputIterator last )
        : m_ht(  2 * std::distance( first, last ) )
    {
        m_ht.insert( first, last );
    }

    /** Creates a cache_map with a copy of a range and a bucket count of at
     *  least @a n.
     *
     * @param first iterator pointing to the first item
     * @param last  iterator pointing to the last item
     * @param n     number of buckets to allocate
     */
    template <class InputIterator>
    cache_map( InputIterator first, InputIterator last, size_type n )
            : m_ht(  2 * n )
    {
        m_ht.insert( first, last );
    }

    /** Creates a cache_map with a copy of a range and a bucket count of at
     *  least @a n, using @a h as the hash function.
     *
     * @param first iterator pointing to the first item
     * @param last  iterator pointing to the last item
     * @param n     number of buckets to allocate
     * @param h     hasher function to be used
     */
    template <class InputIterator>
    cache_map( InputIterator first, InputIterator last,
              size_type n, const hasher& h )
        : m_ht(  2 * n, h )
    {
        m_ht.insert( first, last );
    }

    /** Creates a cache_map with a copy of a range and a bucket count of at
     *  least n, using h as the hash function and k as the key equal
     *  function.
     *
     * @param first iterator pointing to the first item
     * @param last  iterator pointing to the last item
     * @param n     number of buckets to allocate
     * @param h     hasher function to be used
     * @param k     key comparison function to be used
     */
    template <class InputIterator>
    cache_map( InputIterator first, InputIterator last,
              size_type n, const hasher& h,
              const key_equal& k )
        : m_ht(  2 * n, h, k )
    {
        m_ht.insert( first, last );
    }

    /** Copy constructor
     *
     *  @param other the cache_map to be copied
     */
    cache_map( const cache_map& other )
        : m_ht( other.m_ht )
    {}

    /** The assignment operator
     *
     *  @param other the cache_map to be copied
     *  @return a reference to a new cache_map object
     */
    cache_map& operator= ( const cache_map& other )
    {
        if ( &other != this )
        {
            m_ht = other.m_ht;
        }

        return *this;
    }

    /** Sets the value of the empty key.
     *
     *  @param key the key value that will be used to identify empty items.
     */
    void set_empty_key( const key_type& key )
    {
        m_ht.set_empty_value( value_type( key, data_type() ) );
    }

    /** Get the value of the empty key.
     *
     * @return the key value used to mark empty buckets.
     */
    const key_type& get_empty_key() const
    {
        return m_ht.get_empty_value().first;
    }

    /** Insert an item in the map.
     *
     *  The item, the pair(key, data), will be inserted in the hash table.
     *  In case of a key hash collision, the inserted item will replace the
     *  existing one.
     *
     *  Following the @a DiscardFunction policy there will be a notification
     *  that old item has been replaced.
     *
     *  @return A @p pair<iterator,bool> which contain an #iterator to the
     *  inserted item and @p true if the item was correctly inserted, or @p
     *  false if the item was not inserted.
     */
    pair<iterator,bool> insert( const value_type& obj )
    { return m_ht.insert( obj ); }

    /** Non-stardard insert method.
     *  Insert an (key,data) pair in the map.
     *
     *  @warning Don't use it if you want to maintain source code
     *  compatibility with other @a Associative @a Containers.
     *  @see insert( const value_type& )
     */
    pair<iterator,bool> insert( const key_type& key, const data_type& data )
    { return m_ht.insert( value_type( key, data ) ); }

    /** Iterator insertion.
     *  Insert multiple items into the map, using the input iterators.
     *
     *  @param first first elelement
     *  @param last last element
     *  @see insert( const value_type& )
     */
    template <class InputIterator>
    void insert( InputIterator first, InputIterator last )
    { m_ht.insert( first, last ); }

    /** Not standard iterator insertion
     *
     * @param it iterator that mark the insertion position
     * @param obj the new value for the item
     * @return an iterator to the modified item
     */
    iterator insert( iterator it, const value_type& obj )
    { return m_ht.insert( obj ).first; }

    /** Finds an element whose key is @a key
     *
     *  @param key the key of the item
     *
     *  @return an #iterator pointing to the item, or @p end() if the key
     *  cannot be found in the map.
     */
    iterator find( const key_type& key )
    { return m_ht.find( key ); }

    /** Finds an element whose key is @a key
     *
     *  @param key the key of the item
     *
     *  @return a #const_iterator pointing to the item, or @p end() if the key
     *  cannot be found in the map.
     */
    const_iterator find( const key_type& key ) const
    { return m_ht.find( key ); }

    /** Reference operator.
     *
     *  Returns a reference to the object that is associated with a
     *  particular key. If the cache_map does not already contain such an
     *  object, operator[] inserts the default object data_type().
     *
     *  @return a reference to an item, found using the key or newly created.
     */
    data_type& operator[]( const key_type& key )
    { return m_ht.find_or_insert( key ).second; }

    /** Erases the element identified by the key.
     *
     *  @param key The key of the item to be deleted.
     *
     *  @return The number of deleted item. Since the item in cache_map are
     *  unique, this will either be 1 when the key is found and 0 when no
     *  item is deleted.
     */
    size_type erase( const key_type& key ) { return m_ht.erase( key ); }

    /** Erases the element pointed to by the iterator.
     *
     *  @param it a valid iterator to an element in cache_map.
     */
    void erase( iterator it ) { m_ht.erase( it ); }

    /** Erases all elements in a range.
     *
     *  The range of elements to be deleted will be @p [first,last)
     *
     *  @param first The first element in the range (included)
     *  @param last  The last element in the range (excluded)
     */
    void erase( iterator first, iterator last ) { m_ht.erase( first, last ); }

    /** Erases all of the elements. */
    void clear() { m_ht.clear(); }

    /** Increases the bucket count to at least @a size.
     *
     *  @param size the new maximum number of elements.
     *
     *  @warning This operation can be particularly time-consuming. The
     *  algorithm is O(n) and can leave to the complete re-hash of all
     *  of the elements in the cache_map.
     */
    void resize( size_type size ) { m_ht.resize( size ); }

    /** Swap the content of two cache_map.
     *
     *  @param other another cache_map
     */
    void swap( cache_map& other ) { m_ht.swap( other.m_ht ); }

    /** Equality comparison.
     *
     *  @param other The other cache_map to compare.
     *  @return @p true if the 2 cache_map are equal.
     */
    bool operator==( const cache_map& other ) const
    { return m_ht == other.m_ht; }

    /** Dis-Equality comparison.
     *
     *  @param other The other cache_map to compare.
     *  @return @p true if the 2 cache_map are @a NOT equal.
     */
    bool operator!=( const cache_map& other ) const
    { return m_ht != other.m_ht; }

    /** Get the size of the map.
     *  The size represent the number of non-empty elements that can be
     *  found in the container.
     *  @return the number of elements in the map
     */
    size_type size()         const { return m_ht.size(); }

    /** Get the maximum size of the map.
     *  This corresponds to the maximum number of item that the map can
     *  contain.
     *  @return the maximum number of elements
     */
    size_type max_size()     const { return m_ht.max_size(); }

    /** Get the number of allocated buckets.
     *  This corresponds to the max_size() value.
     *  @return the number of buckets
     *  @see max_size
     */
    size_type bucket_count() const { return m_ht.bucket_count(); }

    /** Test for empty.
     *  @return true if the cache_map does not contains items.
     */
    bool empty()             const { return m_ht.empty(); }

    /** Get the number of hash key collisions.
     *
     *  @return the number of hash key collisions
     */
    size_type num_collisions() const { return m_ht.num_collisions(); }

    /** Swap the content of two cache_map instances.
     *
     *  @param m1 a cache_map
     *  @param m2 another cache_map
     */
    friend inline void swap(
        cache_map<Key,T,HashFunction,KeyEqual,DiscardFunction,Allocator>& m1,
        cache_map<Key,T,HashFunction,KeyEqual,DiscardFunction,Allocator>& m2 )
    {
        m1.swap( m2 );
    }

};

} // namespace mm

#endif // _CACHE_MAP_HPP_

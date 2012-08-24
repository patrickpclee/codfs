/*
 *
 *  $Id: hash_fun.hpp 18 2007-01-21 15:49:02Z matteo.merli $
 *
 *  $URL: https://cache-table.googlecode.com/svn/tags/0.2/mm/hash_fun.hpp $
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

#ifndef _MM_HASH_FUN_HPP__
#define _MM_HASH_FUN_HPP__

#include <cstddef>
#include <cstring>
#include <string>

namespace mm
{

using std::size_t;

// Add declarator in order to compile on my arch linux
// Add by DQ
template <class T>
inline void hash_combine( size_t& seed, const T& v );

///////////////////////////////////////////////////////////////////////
// Scalar integers
///////////////////////////////////////////////////////////////////////

/** Hash value specialization for @a char
 *  @param n the number to be hashed
 *  @return the hash value
 *  @relates hash
 */
inline size_t hash_value( char n )
{ return static_cast<size_t>( n ); }

/** Hash value specialization for @a unsigned @a char
 *  @param n the number to be hashed
 *  @return the hash value
 *  @relates hash
 */
inline size_t hash_value( unsigned char n )
{ return static_cast<size_t>( n ); }

/** Hash value specialization for @a short
 *  @param n the number to be hashed
 *  @return the hash value
 *  @relates hash
 */
inline size_t hash_value( short n )
{ return static_cast<size_t>( n ); }

/** Hash value specialization for @a unsigned @a char
 *  @param n the number to be hashed
 *  @return the hash value
 *  @relates hash
 */
inline size_t hash_value( unsigned short n )
{ return static_cast<size_t>( n ); }

/** Hash value specialization for @a int
 *  @param n the number to be hashed
 *  @return the hash value
 *  @relates hash
 */
inline size_t hash_value( int n )
{ return static_cast<size_t>( n ); }

/** Hash value specialization for @a unsigned @a int
 *  @param n the number to be hashed
 *  @return the hash value
 *  @relates hash
 */
inline size_t hash_value( unsigned int n )
{ return static_cast<size_t>( n ); }

/** Hash value specialization for @a long
 *  @param n the number to be hashed
 *  @return the hash value
 *  @relates hash
 */
inline size_t hash_value( long n )
{ return static_cast<size_t>( n ); }

/** Hash value specialization for @a unsigned @a long
 *  @param n the number to be hashed
 *  @return the hash value
 *  @relates hash
 */
inline size_t hash_value( unsigned long n )
{ return static_cast<size_t>( n ); }

///////////////////////////////////////////////////////////////////////
// Sequences
///////////////////////////////////////////////////////////////////////

/** Hash value specialization for a range of elements.
 *
 *  Compute the combined hash value for a collection of items.
 *
 *  @param first iterator that points to the first item in the range
 *               (included)
 *  @param last iterator that points to the last item in the range
 *               (excluded)
 *  @return the hash value
 *  @relates hash
 */
template <class Iterator>
inline size_t hash_range( Iterator first, Iterator last )
{
    size_t h = 0;
    for ( ; first != last; ++first )
        hash_combine( h, *first );

    return h;
}

/** Hash value specialization for a range of elements.
 *
 *  Compute the combined hash value for a collection of items.
 *
 *  @param seed the intial seed value. it is also used as an output
 *              parameter, as it will contains the combined hash
 *  @param first iterator that points to the first item in the range
 *               (included)
 *  @param last iterator that points to the last item in the range
 *               (excluded)
 *  @relates hash
 */
template <class Iterator>
inline void hash_range( size_t& seed, Iterator first, Iterator last )
{
    for ( ; first != last; ++first )
        hash_combine( seed, *first );
}

/** Hash value specialization for @a char*
 *  @param s the c-style string to be hashed
 *  @return the hash value
 *  @relates hash
 */
inline size_t hash_value( char* s )
{
    return hash_range( s, s + strlen( s ) );
}

/** Hash value specialization for @a const @a char*
 *  @param s the c-style string to be hashed
 *  @return the hash value
 *  @relates hash
 */
inline size_t hash_value( const char* s )
{
    return hash_range( s, s + strlen( s ) );
}

/** Hash value specialization for @a std::string
 *  @param s the string to be hashed
 *  @return the hash value
 *  @relates hash
 */
inline size_t hash_value( const std::string& s )
{
    return hash_range( s.begin(), s.end() );
}

/** Hash value specialization for @a std::pair<T1,T2>
 *  @param p the std::pair object to be hashed
 *  @return the hash value
 *  @relates hash
 */
template <class T1, class T2>
inline size_t hash_value( const std::pair<T1,T2>& p )
{
    size_t seed = hash_value( p.first );
    hash_combine( seed, p.second );
    return seed;
}

///////////////////////////////////////////////////////////////////////


/** Combine the hash of various object.
 *
 *  @param seed contain the initial seed value and, after the function
 *  call, will contains the new value.
 *
 *  @param v the object to be hashed.
 *  @relates hash
 */
template <class T>
inline void hash_combine( size_t& seed, const T& v )
{
    // Taken from boost::hash::hash_combine
    seed ^= hash_value( v ) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

///////////////////////////////////////////////////////////////////////

/** Callable hash function.
 *
 *  A Hash Function is a Unary Function that is used by Hashed Associative
 *  Containers: it maps its argument to a result of type @a size_t. A Hash
 *  Function must be deterministic and stateless. That is, the return value
 *  must depend only on the argument, and equal arguments must yield equal
 *  results.
 *
 *  This particular hash implementation, borrow many ideas from boost::hash.
 *
 * @author Matteo Merli
 * @date $Date: 2007-01-21 16:49:02 +0100 (Sun, 21 Jan 2007) $
 */
template <class Key>
struct hash
    : std::unary_function<Key, size_t>
{
    /** Callable operator
     *
     *  @param key the object to be hashed.
     *
     *  @return the @a hash value for the given key
     */
    size_t operator() ( const Key& key ) const
    {
        size_t h = hash_value( key );
        return h;
    }

};

} // namespace mm

#endif // _MM_HASH_FUN_HPP__
